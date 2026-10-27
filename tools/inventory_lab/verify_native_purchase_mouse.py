"""Verify the separate mouse purchase fixture and its fresh-process reload."""
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
assert len(items) == 7 and report["grams"] == 2496
assert all(items[key] == value for key, value in baseline.items())
incoming = [value for key, value in items.items() if key not in baseline]
assert len(incoming) == 1
item = incoming[0]
assert item["location"] == 0 and item["grams"] == 38 and item["condition"] == 10000
assert item["attributes"]["native_section"] == "bandage"
log = a.log.read_text(errors="replace")
pattern = r"mouse purchase verified live=(\d+) seller=(\d+) spent=(\d+) actor_money=(\d+) seller_money=(\d+)"
if not a.previous:
    before = re.search(r"mouse purchase input window live=(\d+) seller=(\d+) actor_money=(\d+) seller_money=(\d+)", log)
    after = re.search(pattern, log)
    assert before and after
    live, seller, spent, actor_money, seller_money = map(int, after.groups())
    assert before[1] == after[1] == item["attributes"]["native_live_id"] and before[2] == after[2]
    assert int(before[3])-actor_money == seller_money-int(before[4]) == spent > 0
    assert log.count("SQA3 purchase committed count=1 ") == 1
    assert log.count("SQA3 purchase client completed count=1 save_ready=1") == 1
    assert re.findall(r"SQA3 pickup client acknowledged live=(\d+)", log) == [str(live)]
    assert "save refused: pickup acknowledgement pending" in log
    report.update(source=live, seller=seller, spent=spent, actor_money=actor_money, seller_money=seller_money)
else:
    previous, _ = read(a.previous)
    assert items == previous
    original = re.search(pattern, a.previous.with_suffix(".log").read_text(errors="replace"))
    reloaded = re.search(r"mouse purchase reload live=(\d+) seller=(\d+) actor_money=(\d+) seller_money=(\d+)", log)
    assert original and reloaded
    assert reloaded.groups() == (original[1], original[2], original[4], original[5])
    report.update(exact_item_reload=True, money_preserved=True)
report["limits"] = "Mouse interaction must also be observed separately; does not establish sales or full trade parity."
a.output.write_text(json.dumps(report, indent=2)+"\n")
print(json.dumps(report))
