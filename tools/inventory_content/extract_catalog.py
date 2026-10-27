"""SQA content interchange v1. Standard library only; never loads game code."""
import argparse
import hashlib
import json
import re
from pathlib import Path
import xml.etree.ElementTree as ET

VERSION = "1.0.0"
DEFAULT_SOURCE = Path("C:/GAMMA/sqa-previews/release-2.1.0-hotfix1-full/source")
DEFAULT_STASH = Path("C:/GAMMA/sqa-previews/stash-grid-marker-test/package")


def digest(data):
    return hashlib.sha256(data).hexdigest()


def encode(value):
    return (json.dumps(value, ensure_ascii=True, sort_keys=True, indent=2) + "\n").encode("utf-8")


def decode(data):
    try:
        return data.decode("utf-8-sig"), "utf-8-sig"
    except UnicodeDecodeError:
        # Lossless byte mapping; do not guess a legacy locale from prose.
        return data.decode("latin-1"), "latin-1-byte-preserving-unknown-locale"


def uncomment(line):
    quoted = False
    for i, char in enumerate(line):
        if char == '"':
            quoted = not quoted
        if char == ";" and not quoted:
            return line[:i].strip()
    return line.strip()


def parse_ltx(text, file_id):
    sections, directives, unknown = [], [], []
    current = None
    for line_no, raw in enumerate(text.splitlines(), 1):
        line = uncomment(raw)
        if not line:
            continue
        ref = {"file": file_id, "line": line_no}
        if line.startswith("#"):
            directives.append({"raw": line, "source": ref})
            continue
        match = re.fullmatch(r"([!<>]*)\[([^\]]*)\]\s*(?::(.*))?", line)
        if match:
            op, name, parents = match.groups()
            current = {"id": f"{file_id}:{line_no}", "name": name.strip(),
                       "operation": op or "declare", "parents": [p.strip() for p in (parents or "").split(",") if p.strip()],
                       "source": ref, "raw_header": raw, "entries": []}
            sections.append(current)
        elif current is None:
            unknown.append({"raw": raw, "source": ref})
        else:
            key, separator, value = line.partition("=")
            current["entries"].append({"key": key.strip(), "value": value.strip() if separator else None,
                                       "raw": raw, "source": ref})
    return sections, directives, unknown


def category(path):
    name = Path(path).name
    if name.startswith("mod_system_"):
        return "system"
    if "craft" in name:
        return "recipes"
    if "trade" in path:
        return "traders"
    if "outfitloadouts" in path:
        return "rig_and_pouch_grants"
    if any(x in name for x in ("death_items", "treasure", "loadouts", "items_tier")):
        return "loot_and_loadouts"
    if "parts" in name:
        return "repair"
    if "outfitloadouts" in path:
        return "rig_and_pouch_grants"
    if name == "zzz_grid_packs.ltx":
        return "backpacks"
    if name == "zzz_grid_stacks.ltx":
        return "stack_policy"
    if name == "amp_boxes.ltx":
        return "box_acceptance"
    return "other"


def resolve_system(sections, diagnostics):
    """Partial projection only. Unknown DLTX operators invalidate resolution."""
    nodes = {}
    for section in sections:
        name = section["name"]
        if not name or not name.isascii() or len(name.encode("utf-8")) > 63:
            diagnostics.append({"severity": "error", "code": "invalid_system_identifier", "source": section["source"], "id": name})
        node = nodes.setdefault(name, {"id": name, "parents": [], "own_fields": {}, "history": [], "unresolved": [], "declared": False})
        node["history"].append(section["id"])
        op = section["operation"]
        if op == "declare":
            if node["declared"]:
                diagnostics.append({"severity": "error", "code": "duplicate_declaration", "id": name, "source": section["source"]})
                node["unresolved"].append("duplicate_declaration")
            node["declared"] = True
            node["parents"] = section["parents"]
        elif op != "!":
            node["unresolved"].append("unsupported_section_operator:" + op)
        elif section["parents"]:
            # DLTX inheritance edits require host semantics, never replace blindly.
            node["unresolved"].append("override_inheritance_requires_host")
        for entry in section["entries"]:
            key = entry["key"]
            if not key or key[0] in "!<>" or entry["value"] is None:
                node["unresolved"].append("unsupported_field_operation:" + key)
            else:
                node["own_fields"][key] = {"value": entry["value"], "source": entry["source"]}
    cache = {}

    def visit(name, chain):
        if name in chain:
            return {}, ["inheritance_cycle:" + "->".join(chain + [name])]
        if name not in nodes:
            return {}, ["external_parent:" + name]
        if name in cache:
            return cache[name]
        node = nodes[name]
        fields, unresolved = {}, list(node["unresolved"])
        if not node["declared"]:
            unresolved.append("external_override_target:" + name)
        for parent in node["parents"]:
            inherited, missing = visit(parent, chain + [name])
            fields.update(inherited)
            unresolved.extend(missing)
        fields.update(node["own_fields"])
        cache[name] = fields, sorted(set(unresolved))
        return cache[name]

    for name, node in nodes.items():
        node["fields"], node["unresolved"] = visit(name, [])
        node["resolution"] = "partial" if node["unresolved"] else "local_complete"
        values = node["fields"]
        node["semantics"] = {label: {k: v for k, v in values.items() if predicate(k)} for label, predicate in {
            "exterior": lambda k: k.startswith("inv_grid_"),
            "text": lambda k: k in ("inv_name", "inv_name_short", "description"),
            "icons": lambda k: k == "icons_texture",
            "rig_pouch_box": lambda k: k.startswith("amp_"),
            "repair": lambda k: any(t in k for t in ("repair", "condition", "degradation")),
            "equipment": lambda k: k in ("slot", "restore_slot_from_config", "kind", "class", "inv_weight"),
        }.items()}
    return [nodes[k] for k in sorted(nodes)]


def build(source=DEFAULT_SOURCE, stash=DEFAULT_STASH):
    source, stash = Path(source), Path(stash)
    for root in (source, stash):
        if not (root / "gamedata/configs").is_dir():
            raise ValueError(f"Missing required config source: {root}")
    catalog = {"schema": "sqa.inventory-content", "schema_version": VERSION,
               "sources": {"release": source.resolve().as_posix(), "stash": stash.resolve().as_posix()},
               "files": [], "documents": [], "localizations": [], "texture_regions": [],
               "profiles": [], "dependencies": [], "diagnostics": []}
    profiles = [("release", source), ("stash", stash)]
    profiles += [("optional/" + p.name, p) for p in sorted((source / "optional").glob("*")) if p.is_dir()]
    for profile, root in profiles:
        sections = []
        assets = {}
        for path in sorted((root / "gamedata").rglob("*")):
            if not path.is_file():
                continue
            rel = path.relative_to(root).as_posix()
            file_id = profile + "/" + rel
            data = path.read_bytes()
            info = {"id": file_id, "profile": profile, "path": rel, "sha256": digest(data), "bytes": len(data)}
            catalog["files"].append(info)
            assets[rel.lower()] = file_id
            if path.suffix.lower() not in (".ltx", ".xml"):
                continue
            text, encoding = decode(data)
            info["text_encoding"] = encoding
            # Exact source bytes remain recoverable through original hash/source;
            # source_text retains comments and unknown syntax without dropping it.
            doc = {"file": file_id, "category": category(rel), "source_text": text}
            catalog["documents"].append(doc)
            if path.suffix.lower() == ".ltx":
                blocks, directives, unknown = parse_ltx(text, file_id)
                doc.update(sections=blocks, directives=directives, unknown=unknown)
                for block in blocks:
                    if not block["name"]:
                        catalog["diagnostics"].append({"severity": "error", "code": "empty_section", "source": block["source"]})
                if doc["category"] == "system":
                    sections.extend(blocks)
                for directive in directives:
                    catalog["dependencies"].append({"kind": "directive", "status": "requires_host_include_order", **directive})
                for record in unknown:
                    catalog["diagnostics"].append({"severity": "warning", "code": "unparsed_ltx", **record})
            else:
                try:
                    xml = ET.fromstring(data)
                except ET.ParseError as exc:
                    catalog["diagnostics"].append({"severity": "error", "code": "xml_parse", "file": file_id, "detail": str(exc)})
                    continue
                for index, string in enumerate(xml.iter("string")):
                    catalog["localizations"].append({"profile": profile, "language": path.parent.name,
                        "id": string.get("id"), "text": "".join(string.itertext()).strip(),
                        "source": {"file": file_id, "element": f"string[{index}]"}})
                for atlas in xml.iter("file"):
                    for index, texture in enumerate(atlas.findall("texture")):
                        catalog["texture_regions"].append({"profile": profile, "atlas": atlas.get("name"),
                            "attributes": dict(texture.attrib), "source": {"file": file_id, "element": f"file[{atlas.get('name')}]/texture[{index}]"}})
        definitions = resolve_system(sections, catalog["diagnostics"])
        catalog["profiles"].append({"id": profile, "composition": "independent_uncomposed_layer", "definitions": definitions})
        for item in definitions:
            for reason in item["unresolved"]:
                catalog["dependencies"].append({"profile": profile, "item": item["id"], "kind": "system_resolution", "status": "unresolved", "reason": reason})
            for key, field in item["fields"].items():
                if key in ("icons_texture", "visual"):
                    prefix, extension = ("textures/", ".dds") if key == "icons_texture" else ("meshes/", ".ogf")
                    value = field["value"].strip('"').replace("\\", "/")
                    target = "gamedata/" + prefix + value
                    if not target.lower().endswith(extension):
                        target += extension
                    catalog["dependencies"].append({"profile": profile, "item": item["id"], "kind": key,
                        "target": target, "status": "local" if target.lower() in assets else "external_or_other_layer",
                        "source": field["source"]})
                if key in ("description", "inv_name", "inv_name_short"):
                    matches = [v["source"] for v in catalog["localizations"] if v["profile"] == profile and v["id"] == field["value"]]
                    catalog["dependencies"].append({"profile": profile, "item": item["id"], "kind": "localization", "target": field["value"],
                        "status": "local" if matches else "external_or_literal", "matches": matches, "source": field["source"]})
    # Specialized tables remain ordered records, never global section dictionaries.
    catalog["tables"] = [{"file": d["file"], "category": d["category"], "section_ids": [s["id"] for s in d["sections"]],
                           "resolution": "authored_records_host_merge_required"}
                         for d in catalog["documents"] if "sections" in d and d["category"] != "system"]
    for doc in catalog["documents"]:
        if doc["category"] not in ("recipes", "traders", "loot_and_loadouts", "repair", "rig_and_pouch_grants", "backpacks"):
            continue
        for section in doc.get("sections", []):
            for entry in section["entries"]:
                catalog["dependencies"].append({"kind": "authored_table_row", "category": doc["category"],
                    "key": entry["key"], "tokens": [v.strip() for v in (entry["value"] or "").split(",")],
                    "status": "host_schema_and_external_references_unresolved", "source": entry["source"]})
    seen = set()
    for loc in catalog["localizations"]:
        key = (loc["profile"], loc["language"], loc["id"])
        if not loc["id"] or key in seen:
            catalog["diagnostics"].append({"severity": "error", "code": "missing_or_duplicate_localization_id", "source": loc["source"], "id": loc["id"]})
        seen.add(key)
    catalog["recipes"] = []
    catalog["backpack_sizes"] = []
    local_ids = {n["id"] for p in catalog["profiles"] if p["id"] == "release" for n in p["definitions"] if n["declared"]}
    for doc in catalog["documents"]:
        for section in doc.get("sections", []):
            for entry in section["entries"]:
                value = entry["value"] or ""
                if doc["category"] == "recipes" and entry["key"].startswith("x_"):
                    tokens = [t.strip() for t in value.split(",")]
                    if len(tokens) < 4 or len(tokens) % 2 or not tokens[0].isdigit() or any(not t.isdigit() for t in tokens[3::2]):
                        catalog["diagnostics"].append({"severity": "error", "code": "invalid_recipe", "source": entry["source"]})
                        continue
                    recipe = {"output": entry["key"][2:], "toolkit": int(tokens[0]), "book": tokens[1],
                        "ingredients": [{"id": tokens[i], "count": int(tokens[i+1])} for i in range(2, len(tokens), 2)],
                        "source": entry["source"], "section": section["id"]}
                    catalog["recipes"].append(recipe)
                    for target in [recipe["output"], recipe["book"]] + [i["id"] for i in recipe["ingredients"]]:
                        catalog["dependencies"].append({"kind": "recipe_reference", "target": target,
                            "status": "local_declared" if target in local_ids else "external_definition_required", "source": entry["source"]})
                if doc["category"] == "backpacks":
                    match = re.fullmatch(r"(\d+)x(\d+)", value)
                    if match:
                        rows, columns = map(int, match.groups())
                        catalog["backpack_sizes"].append({"selector": section["name"], "key": entry["key"],
                            "rows": rows, "columns": columns, "cells": rows * columns, "raw": value, "source": entry["source"]})
                    elif value.isdigit():
                        catalog["backpack_sizes"].append({"selector": section["name"], "key": entry["key"],
                            "cells": int(value), "columns": None, "raw": value, "source": entry["source"]})
    catalog["input_manifest_sha256"] = digest(encode(catalog["files"]))
    return catalog


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, default=DEFAULT_SOURCE)
    parser.add_argument("--stash", type=Path, default=DEFAULT_STASH)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    result = build(args.source, args.stash)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(encode(result))
    errors = [d for d in result["diagnostics"] if d["severity"] == "error"]
    print(json.dumps({"output": str(args.output), "sha256": digest(encode(result)), "errors": len(errors),
                      "files": len(result["files"]), "profiles": len(result["profiles"])}))
    return 1 if errors else 0


if __name__ == "__main__":
    raise SystemExit(main())
