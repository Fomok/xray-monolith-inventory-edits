"""Verify Move all retains a loaded subtree while transferring loose items."""
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
report = verify(a.log, a.export, logical_ownership=True)
items, _ = read(a.export)
baseline, _ = read(a.baseline)
assert items == {key: baseline[key] for key in [102, 103, 105]}
assert items[102]["parent"] == 105 and report["grams"] == 2130
log = a.log.read_text(errors="replace")
result = re.search(r"loaded bulk retained backpack subtree; crate=(\d+) loose_items=3", log)
assert result
rows = re.findall(r"loaded bulk source live=(\d+) owner=(\d+) section=(\S+) grams=(\S+) condition=(\S+)", log)
assert len(rows) == 3 and len({r[0] for r in rows}) == 3
observed = {live: (section, math.floor(float(grams)+0.5), math.floor(float(condition)*10000+0.5))
            for live, owner, section, grams, condition in rows}
expected = {baseline[key]["attributes"]["native_live_id"]:
            (baseline[key]["attributes"]["native_section"], baseline[key]["grams"], baseline[key]["condition"])
            for key in [100, 101, 104]}
assert observed == expected and all(row[1] == result[1] for row in rows)
if a.previous:
    previous, _ = read(a.previous)
    assert items == previous
    old_log = a.previous.with_suffix(".log").read_text(errors="replace")
    assert f"crate={result[1]} loose_items=3" in old_log
    report["exact_item_reload"] = True
else:
    assert "SQA3 loaded storage release refused" in log and "SQA3 nested bulk release refused" in log
report.update(crate=int(result[1]), exact_loaded_subtree_preserved=True, loose_items_preserved_at_source=True,
              limits="One native Move all to crate. Loaded subtree transfer is refused, not implemented; corpse and broader producer coverage remain separate.")
a.output.write_text(json.dumps(report, indent=2)+"\n")
print(json.dumps(report))
