"""Actual ground ownership admission and client completion, never loot/trade coverage."""
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
ap.add_argument("--reload", action="store_true")
ap.add_argument("--verify-sources", action="store_true")
ap.add_argument("--previous", type=Path)
ap.add_argument("--output", type=Path, required=True)
a = ap.parse_args()
report = verify(a.log, a.export, True, logical_ownership=True)
items, digest = read(a.export)
baseline, _ = read(a.baseline)
assert len(items) == 71 and report["grams"] == 4928
assert all(items[key] == value for key, value in baseline.items()), "existing item state changed"
incoming = {key: value for key, value in items.items() if key not in baseline}
assert len(incoming) == 65
assert all(n["location"] == 0 and n["attributes"]["native_section"] == "bandage"
           and n["grams"] == 38 and n["condition"] == 10000 for n in incoming.values()), "ordinary arrival used recovery or changed state"
log = a.log.read_text(errors="replace")
if not a.reload:
    accepted = re.findall(r"SQA3 ground pickup server accepted live=(\d+)", log)
    ack = re.findall(r"SQA3 (?:ground )?pickup client acknowledged live=(\d+)", log)
    refused = re.findall(r"SQA3 ground pickup refused live=(\d+) reason=no_fit; source retained", log)
    assert len(accepted) == len(set(accepted)) == 65 and sorted(accepted) == sorted(ack)
    assert len(refused) == 4 and len(set(refused)) == 2, "full-grid retries did not preserve two sources"
    assert "ground admission accepted=65 source_retained=2" in log
    assert "repeated full-grid refusal retained two exact ground sources" in log
    assert log.count("save refused: pickup acknowledgement pending") == 65
    assert "pending=0 save_ready=1" in log
    report.update(accepted=65, refused_sources=2, repeated_refusals=4, pending_save_gate=True)
else:
    assert a.previous, "reload needs previous export"
    previous, _ = read(a.previous)
    assert items == previous, "native state/placement changed after restart"
    report["exact_item_reload"] = True
    if a.verify_sources:
        previous_log=a.previous.with_suffix(".log").read_text(errors="replace")
        refused=set(re.findall(r"SQA3 ground pickup refused live=(\d+)",previous_log))
        retained=re.findall(r"reloaded refused source live=(\d+) grams=38 condition=1",log)
        assert len(refused)==len(retained)==2 and set(retained)==refused
        report["refused_sources_preserved_after_reload"]=sorted(refused)
report["limits"] = "Opt-in ground pickup only. Corpse/container/trade pre-detach admission and loaded world subtrees remain unfinished."
a.output.write_text(json.dumps(report, indent=2)+"\n")
print(json.dumps(report))
