"""Verify keyboard rotation inside carried storage and edge refusal persistence."""
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
ap.add_argument("--previous", type=Path)
ap.add_argument("--edge", action="store_true")
ap.add_argument("--output", type=Path, required=True)
a = ap.parse_args()
report = verify(a.log, a.export, True, logical_ownership=True)
items, _ = read(a.export)
baseline, _ = read(a.baseline)
assert set(items) == set(baseline) and len(items) == 6 and report["grams"] == 2458
assert all(items[key] == value for key, value in baseline.items() if key != 103)
weapon = items[103]
assert weapon["parent"] == 105 and weapon["location"] == 0 and weapon["equipped_slots"] == 0
assert weapon["position"] == ([7, 6, 1] if a.edge else [5, 6, 1])
assert weapon["grams"] == baseline[103]["grams"] and weapon["condition"] == baseline[103]["condition"]
assert weapon["attributes"]["native_live_id"] == baseline[103]["attributes"]["native_live_id"]
assert weapon["attributes"]["native_section"] == "wpn_pm" and weapon["attributes"]["native_place"] == "3"
log = a.log.read_text(errors="replace")
assert f"logical rotation finished edge={str(a.edge).lower()}" in log
if a.previous:
    previous, _ = read(a.previous)
    assert items == previous
    report["exact_item_reload"] = True
else:
    results = re.findall(r"SQA3 native rotate id=103 result=(\S+)", log)
    assert len(results) == 1 and ((results[0] != "accepted") if a.edge else results[0] == "accepted")
report.update(edge_refusal=a.edge, unchanged_other_five_records=True,
              limits="One physical select/R input on a carried-backpack pistol, or its boundary refusal. Dragging while rotating and other storage/weapon shapes remain separate checks.")
a.output.write_text(json.dumps(report, indent=2)+"\n")
print(json.dumps(report))
