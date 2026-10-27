"""Verify native purchase ownership, capacity refusal, money and persistence."""
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
assert len(items) == 71 and report["grams"] == 4928
assert all(items[key] == value for key, value in baseline.items())
incoming = {key: value for key, value in items.items() if key not in baseline}
assert len(incoming) == 65
assert all(n["location"] == 0 and n["attributes"]["native_section"] == "bandage"
           and n["grams"] == 38 and n["condition"] == 10000 for n in incoming.values())
log = a.log.read_text(errors="replace")
if not a.previous:
    before = re.search(r"purchase whole basket refused count67 actor_money=(\d+) seller_money=(\d+)", log)
    after = re.search(r"purchase verified count65 spent=(\d+) actor_money=(\d+) seller_money=(\d+) owner=(\d+)", log)
    assert before and after
    spent, actor, seller, owner = map(int, after.groups())
    assert int(before[1])-actor == seller-int(before[2]) == spent > 0
    assert log.count("SQA3 purchase refused before callbacks reason=no_fit") == 2
    assert log.count("SQA3 purchase committed count=65") == 1
    assert log.count("SQA3 purchase client completed count=65 save_ready=1") == 1
    assert "purchase full retry retained sources and money" in log
    acknowledged = re.findall(r"SQA3 pickup client acknowledged live=(\d+)", log)
    assert len(acknowledged) == len(set(acknowledged)) == 65
    assert set(acknowledged) == {n["attributes"]["native_live_id"] for n in incoming.values()}
    assert "save refused: pickup acknowledgement pending" in log
    retained = re.findall(r"purchase retained live=(\d+) seller=(\d+)", log)
    assert len(retained) == 2 and all(int(source) == owner for _, source in retained)
    report.update(spent=spent, actor_money=actor, seller_money=seller, seller=owner,
                  retained_sources=[live for live, _ in retained], whole_batch_refusal=True,
                  pending_save_gate=True, client_acknowledged=65)
else:
    previous, _ = read(a.previous)
    assert items == previous, "purchase item records changed after reload"
    previous_log = a.previous.with_suffix(".log").read_text(errors="replace")
    original = set(re.findall(r"purchase retained live=(\d+) seller=(\d+)", previous_log))
    retained = re.findall(r"purchase reloaded retained live=(\d+) seller=(\d+)", log)
    assert len(original) == len(retained) == 2 and set(retained) == original
    original_money = re.search(r"purchase verified count65 spent=\d+ actor_money=(\d+) seller_money=(\d+)", previous_log)
    reloaded_money = re.search(r"purchase money reloaded actor=(\d+) seller=(\d+)", log)
    assert original_money and reloaded_money and original_money.groups() == reloaded_money.groups()
    report.update(exact_item_reload=True, money_preserved=True, source_items_preserved=True)
report["limits"] = "Native fixture purchase authority; not physical trade-button validation or full sale/equip/use compatibility."
a.output.write_text(json.dumps(report, indent=2)+"\n")
print(json.dumps(report))
