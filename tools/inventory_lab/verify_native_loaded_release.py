"""Verify refusal preserves a loaded carried backpack and its logical subtree."""
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
ap.add_argument("--output", type=Path, required=True)
a = ap.parse_args()
report = verify(a.log, a.export, True, logical_ownership=True)
items, _ = read(a.export)
baseline, _ = read(a.baseline)
assert items == baseline and len(items) == 6 and report["grams"] == 2458
assert items[102]["parent"] == 105 and items[105]["attributes"]["native_section"] == "itm_backpack"
log = a.log.read_text(errors="replace")
retained = re.search(r"loaded release retained bag=(\d+) empty_crate=(\d+)", log)
assert retained and items[105]["attributes"]["native_live_id"] == retained[1]
if a.previous:
    previous, _ = read(a.previous)
    assert items == previous
    assert f"loaded release retained bag={retained[1]} empty_crate={retained[2]}" in a.previous.with_suffix(".log").read_text(errors="replace")
    report["exact_item_reload"] = True
else:
    assert f"SQA3 loaded storage release refused live={retained[1]}; subtree transfer not ready" in log
report.update(bag=int(retained[1]), empty_crate=int(retained[2]), exact_loaded_subtree_preserved=True,
              limits="Native refusal/state preservation only; visible refusal UI needs separate screenshot inspection. Loaded world-container transfer is not implemented.")
a.output.write_text(json.dumps(report, indent=2)+"\n")
print(json.dumps(report))
