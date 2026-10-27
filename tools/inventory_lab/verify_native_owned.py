"""Check actual owned-source acquisition and retained source objects."""
import argparse
import json
import re
from pathlib import Path
from verify_native_snapshot import verify
from verify_native_placement import read

ap = argparse.ArgumentParser()
ap.add_argument("log", type=Path)
ap.add_argument("export", type=Path)
ap.add_argument("--baseline", type=Path, required=True)
ap.add_argument("--kind", choices=["box", "corpse"], required=True)
ap.add_argument("--previous", type=Path)
ap.add_argument("--output", type=Path, required=True)
a = ap.parse_args()
report = verify(a.log, a.export, True, logical_ownership=True)
items, digest = read(a.export)
baseline, _ = read(a.baseline)
assert len(items) == 71 and report["grams"] == 4928
assert all(items[key] == value for key, value in baseline.items())
incoming = {key: value for key, value in items.items() if key not in baseline}
assert len(incoming) == 65
assert all(n["location"] == 0 and n["attributes"]["native_section"] == "bandage"
           and n["grams"] == 38 and n["condition"] == 10000 for n in incoming.values())
log = a.log.read_text(errors="replace")
if not a.previous:
    accepted = re.findall(r"SQA3 owned pickup server accepted live=(\d+)", log)
    acknowledged = re.findall(r"SQA3 pickup client acknowledged live=(\d+)", log)
    refused = re.findall(r"SQA3 owned pickup refused live=(\d+) reason=no_fit; source retained", log)
    assert len(accepted) == len(set(accepted)) == 65 and sorted(accepted) == sorted(acknowledged)
    assert len(refused) == 4 and len(set(refused)) == 2
    summary = re.search(r"owned admission kind="+a.kind+r" accepted=65 source_retained=2 owner=(\d+)", log)
    assert summary and "repeated full-grid refusal retained two exact owned sources" in log
    source_records = re.findall(r"refused owned source live=(\d+) owner=(\d+)", log)
    assert len(source_records) == 2 and {i for i, _ in source_records} == set(refused)
    assert all(owner == summary[1] for _, owner in source_records)
    assert log.count("save refused: pickup acknowledgement pending") == 65
    assert "pending=0 save_ready=1" in log
    report.update(accepted=65, refused_sources=sorted(set(refused)), owner=summary[1], pending_save_gate=True)
else:
    previous, _ = read(a.previous)
    assert items == previous, "native carried records changed after reload"
    previous_log = a.previous.with_suffix(".log").read_text(errors="replace")
    original = set(re.findall(r"refused owned source live=(\d+) owner=(\d+)", previous_log))
    retained = re.findall(r"reloaded refused owned source live=(\d+) owner=(\d+) grams=38 condition=1", log)
    assert len(original) == len(retained) == 2 and set(retained) == original
    report.update(exact_item_reload=True, refused_sources_preserved_after_reload=retained)
report.update(kind=a.kind, limits="Native owned-source Move/Take All path. Trade money, external equip/use actions and loaded world subtrees are not covered.")
a.output.write_text(json.dumps(report, indent=2)+"\n")
print(json.dumps(report))
