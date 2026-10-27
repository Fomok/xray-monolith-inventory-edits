"""Verify physical slot-to-crate transfer without using full carried storage."""
import argparse
import json
import math
import re
from pathlib import Path
from verify_native_placement import read
from verify_native_snapshot import verify

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
assert len(items) == 71 and len(baseline) == 72
assert all(baseline[key] == value for key, value in items.items())
removed = [value for key, value in baseline.items() if key not in items]
assert len(removed) == 1 and removed[0]["equipped_slots"] == 8
log = a.log.read_text(errors="replace")
result = re.search(r"direct unequip result moved=(\d+) retained=(\d+) source=(\d+)", log)
assert result
moved, retained, source = result.groups()
assert removed[0]["attributes"]["native_live_id"] == moved

def weapons(text, prefix=""):
    rows = re.findall(r"direct unequip "+prefix+r"weapon live=(\d+) parent=(\d+) grams=(\S+) condition=(\S+) ammo=(\d+)", text)
    assert len(rows) == 2 and len({row[0] for row in rows}) == 2
    return {live: dict(parent=parent, grams=math.floor(float(grams)+0.5),
                      condition=math.floor(float(condition)*10000+0.5), ammo=int(ammo))
            for live, parent, grams, condition, ammo in rows}

observed = weapons(log)
assert set(observed) == {moved, retained}
assert all(row["parent"] == source for row in observed.values())
if a.previous:
    previous, _ = read(a.previous)
    assert items == previous
    assert observed == weapons(a.previous.with_suffix(".log").read_text(errors="replace"))
    report["exact_item_reload"] = True
else:
    before = weapons(log, "before ")
    for live in before:
        assert all(before[live][key] == observed[live][key] for key in ["grams", "condition", "ammo"])
    assert before[retained]["parent"] == source and before[moved]["parent"] != source
report.update(moved=int(moved), retained=int(retained), source=int(source), weapon_state_preserved=True,
              limits="One physical primary-slot to native crate transfer with full carried storage. Finite target capacity, rigs and loaded container subtrees remain separate checks.")
a.output.write_text(json.dumps(report, indent=2)+"\n")
print(json.dumps(report))
