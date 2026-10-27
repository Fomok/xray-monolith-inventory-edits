"""Verify a physical Buy refusal against the saved full-bag purchase result."""
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
ap.add_argument("--output", type=Path, required=True)
a = ap.parse_args()
report = verify(a.log, a.export, True, logical_ownership=True)
items, _ = read(a.export)
previous, _ = read(a.previous)
assert items == previous and len(items) == 71 and report["grams"] == 4928
log = a.log.read_text(errors="replace")
old_log = a.previous.with_suffix(".log").read_text(errors="replace")
assert log.count("SQA3 purchase refused before callbacks reason=no_fit") >= 1
assert "SQA3 purchase committed" not in log and "SQA3 pickup client acknowledged" not in log
retained = re.findall(r"purchase refusal mouse retained live=(\d+) seller=(\d+)", log)
assert len(retained) == 2 and set(retained) == set(re.findall(r"purchase retained live=(\d+) seller=(\d+)", old_log))
money = re.search(r"purchase refusal mouse verified actor_money=(\d+) seller_money=(\d+)", log)
old_money = re.search(r"purchase verified count65 spent=\d+ actor_money=(\d+) seller_money=(\d+)", old_log)
assert money and old_money and money.groups() == old_money.groups()
report.update(exact_item_records=True, exact_retained_sources=True, money_unchanged=True,
              limits="Requires separately observed drag/Buy interaction. Full-bag refusal only; not full trade compatibility.")
a.output.write_text(json.dumps(report, indent=2)+"\n")
print(json.dumps(report))
