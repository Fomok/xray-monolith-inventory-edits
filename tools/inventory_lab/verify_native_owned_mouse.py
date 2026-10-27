"""Verify native mouse-driven refusal, drop, acquisition and optional reload."""
import argparse
import json
import re
from pathlib import Path
from verify_native_snapshot import verify
from verify_native_placement import read

ap = argparse.ArgumentParser()
ap.add_argument("log", type=Path)
ap.add_argument("export", type=Path)
ap.add_argument("--previous", type=Path, required=True)
ap.add_argument("--reload", action="store_true")
ap.add_argument("--output", type=Path, required=True)
a = ap.parse_args()
report = verify(a.log, a.export, logical_ownership=True)
items, _ = read(a.export)
previous, _ = read(a.previous)
log = a.log.read_text(errors="replace")
old_log = a.previous.with_suffix(".log").read_text(errors="replace")
assert len(items) == len(previous) == 71 and report["grams"] == 4928
assert "owned mouse transfer verified acquired=1 retained=1 dropped=1" in log
dropped = re.findall(r"mouse dropped actor bandage live=(\d+) grams=38 condition=1", log)
assert len(dropped) == 1
assert dropped[0] not in {n["attributes"]["native_live_id"] for n in items.values()}
if a.reload:
    assert items == previous, "mouse result changed after native restart"
    assert dropped == re.findall(r"mouse dropped actor bandage live=(\d+) grams=38 condition=1", old_log)
    assert re.findall(r"mouse source live=(\d+) parent=(\d+)", log) == re.findall(r"mouse source live=(\d+) parent=(\d+)", old_log)
    report["exact_item_reload"] = True
else:
    removed, added = set(previous)-set(items), set(items)-set(previous)
    assert len(removed) == len(added) == 1
    old = previous[next(iter(removed))]
    new = items[next(iter(added))]
    assert old["attributes"]["native_live_id"] == dropped[0]
    assert all(n["attributes"]["native_section"] == "bandage" and n["grams"] == 38 for n in (old, new))
    assert new["location"] == 0
    assert all(items[key] == previous[key] for key in set(items)&set(previous)), "unrelated item changed"
    accepted = re.findall(r"SQA3 owned pickup server accepted live=(\d+)", log)
    acknowledged = re.findall(r"SQA3 pickup client acknowledged live=(\d+)", log)
    assert accepted == acknowledged == [new["attributes"]["native_live_id"]]
    source_ids = {i for i, _ in re.findall(r"refused owned source live=(\d+) owner=(\d+)", old_log)}
    assert new["attributes"]["native_live_id"] in source_ids
    refused = re.findall(r"SQA3 owned pickup refused live=(\d+) reason=no_fit; source retained", log)
    assert refused and set(refused) <= source_ids
    report.update(accepted_native_transfer=accepted[0], dropped_native_item=dropped[0], full_grid_refusals=len(refused))
report["limits"] = "Owned-source mouse drag/context drop path only; trade, external equip/use and loaded-world-subtree parity remain open."
a.output.write_text(json.dumps(report, indent=2)+"\n")
print(json.dumps(report))
