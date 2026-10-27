"""Check exact bread removal and unchanged surviving nested inventory records."""
import argparse
import json
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
assert baseline[104]["attributes"]["native_section"] == "bread"
assert baseline[104]["parent"] == 105, "baseline bread was not nested"
assert len(items) == 5 and report["grams"] == 2268
assert items == {key: value for key, value in baseline.items() if key != 104}
assert "nested manual consumption verified count5 grams2268" in a.log.read_text(errors="replace")
if a.previous:
    previous, _ = read(a.previous)
    assert items == previous
    report["exact_item_reload"] = True
report.update(exact_bread_removed=True, surviving_records_unchanged=True,
              limits="One nested bread consumption. Other use/custom actions and external-source admission remain separate checks.")
a.output.write_text(json.dumps(report, indent=2)+"\n")
print(json.dumps(report))
