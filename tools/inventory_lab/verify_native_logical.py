"""Verify actual native backpack storage, not ordinary pickup enforcement."""
import argparse
import json
from pathlib import Path
from verify_native_placement import read
from verify_native_snapshot import verify


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("log", type=Path)
    ap.add_argument("export", type=Path)
    ap.add_argument("--phase", choices=["unequipped", "equipped"], required=True)
    ap.add_argument("--previous", type=Path)
    ap.add_argument("--med-position", nargs=2, type=int, default=[1, 1])
    ap.add_argument("--output", type=Path, required=True)
    a = ap.parse_args()
    report = verify(a.log, a.export, True, logical_ownership=True)
    items, digest = read(a.export)
    assert set(items) == set(range(100, 106)), "lost or unexpected native identity"
    pack, med = items[105], items[102]
    assert pack["attributes"]["native_section"] == "itm_backpack"
    assert med["attributes"]["native_section"] == "medkit" and med["parent"] == 105
    assert med["position"] == a.med_position+[0], "nested item moved or flattened"
    assert items[101]["parent"] == 1 and items[101]["position"] == [2, 0, 0], "pocket assignment changed"
    assert pack["location"] == (3 if a.phase == "unequipped" else 1)
    assert pack["equipped_slots"] == (0 if a.phase == "unequipped" else 8192)
    rows = {int(f[0]): f for f in (r.split() for r in a.export.read_text().splitlines()[1:-1])}
    assert rows[1][16:18] == ["4", "2"] and "native_logical_storage" in rows[1]
    assert rows[105][14] == "1" and rows[105][16:18] == ["8", "8"]
    log = a.log.read_text(errors="replace")
    assert "SQA3 loaded storage release refused" in log, "missing server drop refusal"
    assert "loaded backpack drop refused; native parent retained" in log
    if a.previous:
        previous, old_digest = read(a.previous)
        assert set(previous) == set(items)
        for identity, item in items.items():
            for field in ("grams", "condition"):
                assert item[field] == previous[identity][field], "native state changed across restart"
            assert item["attributes"]["native_section"] == previous[identity]["attributes"]["native_section"]
            if identity != 105:
                assert item["parent"] == previous[identity]["parent"], "reload reassigned contents"
        report["previous_sha256"] = old_digest
    report.update(phase=a.phase, items=items, export_sha256=digest,
                  limits="Native carried backpack ownership and protected drop only. World subtree transfer, strict pickup admission, rig gates and full parity remain unverified.")
    a.output.write_text(json.dumps(report, indent=2)+"\n")
    print(json.dumps(report))


if __name__ == "__main__":
    main()
