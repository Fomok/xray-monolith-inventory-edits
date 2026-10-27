"""Verify direct equipment admission from full storage, source refusal and reload."""
import argparse
import json
import math
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
assert len(items) == len(baseline)+1 == 72
assert all(items[key] == value for key, value in baseline.items())
log = a.log.read_text(errors="replace")
result = re.search(r"direct equip result acquired=(\d+) retained=(\d+) source=(\d+) slot=3", log)
assert result
acquired, retained, source = result.groups()
def weapons(text, prefix=""):
    rows = re.findall(r"direct equip "+prefix+r"weapon live=(\d+) parent=(\d+) grams=(\S+) condition=(\S+) ammo=(\d+)", text)
    assert len(rows) == 2 and len({row[0] for row in rows}) == 2
    return {live: dict(parent=parent, grams=math.floor(float(grams)+0.5), condition=math.floor(float(condition)*10000+0.5), ammo=int(ammo))
            for live, parent, grams, condition, ammo in rows}
observed = weapons(log)
assert sorted((row["condition"], row["ammo"]) for row in observed.values()) == [(8314, 5), (9098, 9)]
active = re.search(r"direct equip active after close live=(\d+|none)", log)
assert active and active.group(1) == acquired, "equipped rifle was not selected after closing the menu"
assert set(observed) == {acquired, retained} and observed[retained]["parent"] == source
incoming = [n for key,n in items.items() if key not in baseline]
assert len(incoming) == 1
node = incoming[0]
assert node["location"] == 1 and node["equipped_slots"] == 8 and node["parent"] == 1
assert node["attributes"]["native_live_id"] == acquired and node["attributes"]["native_slot"] == "3"
assert node["attributes"]["native_section"] == "wpn_ak74"
assert node["grams"] == observed[acquired]["grams"] and node["condition"] == observed[acquired]["condition"]
if not a.previous:
    original = weapons(log, "before ")
    for live in original:
        assert all(original[live][key] == observed[live][key] for key in ["grams","condition","ammo"])
    assert f"SQA3 direct equip refused live={retained} slot=3; source retained" in log
    assert log.count(f"SQA3 direct equip reserved live={acquired} slot=3") == 1
    assert re.findall(r"SQA3 pickup client acknowledged live=(\d+)", log) == [acquired]
    assert "save refused: pickup acknowledgement pending" in log
else:
    previous, _ = read(a.previous)
    assert items == previous
    assert observed == weapons(a.previous.with_suffix(".log").read_text(errors="replace"))
    report["exact_item_reload"] = True
report.update(acquired=int(acquired), retained=int(retained), source=int(source), weapon_state_preserved=True,
              limits="One empty primary-slot loot path and occupied-slot refusal. All-slot swaps, belt/use, and arbitrary callback reconciliation remain separate checks.")
a.output.write_text(json.dumps(report, indent=2)+"\n")
print(json.dumps(report))
