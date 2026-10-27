"""Compare the final real inventory snapshot with a canonical model export.

This checks observed ownership, identity, section, mass and condition, not UI input.
"""
import argparse
import json
import math
import re
from pathlib import Path
from verify_native_placement import read


def verify(log_path, export_path, preserve_initial=False, logical_ownership=False,
           completion_pattern=r"\[SQA3 AUTORUN\] step=\d+ execute quit"):
    log = log_path.read_text(errors="replace")
    assert "[SQA3 AUTORUN] FAIL" not in log, "fixture failed"
    assert "FATAL ERROR" not in log, "engine crashed"
    assert re.search(completion_pattern, log), "fixture did not complete"
    pending, snapshots, raw_mass = {}, [], 0.0
    for line in log.splitlines():
        item = re.search(r"\[SQA3 AUTORUN\] item id=(\d+) section=(\S+) grams=(\S+) condition=(\S+)", line)
        if item:
            live, section, grams, condition = item.groups()
            assert int(live) not in pending, "duplicate native identity"
            mass, state = float(grams), float(condition)
            assert math.isfinite(mass) and mass >= 0 and math.isfinite(state) and 0 <= state <= 1
            raw_mass += mass
            # Match C++ llround for nonnegative native values, including half units.
            pending[int(live)] = (section, math.floor(mass+0.5), math.floor(state*10000+0.5))
        total = re.search(r"\[SQA3 AUTORUN\] native count=(\d+) grams=(\S+)", line)
        if total:
            assert len(pending) == int(total[1]), "snapshot count mismatch"
            assert abs(raw_mass-float(total[2])) < 0.01, "snapshot mass mismatch"
            snapshots.append(pending)
            pending, raw_mass = {}, 0.0
    assert snapshots, "missing real native snapshot"
    nodes, digest = read(export_path)
    projected = {}
    for node in nodes.values():
        attrs = node["attributes"]
        live = int(attrs["native_live_id"])
        assert live not in projected, "duplicate native identity"
        owner = node["parent"]
        if logical_ownership:
            visited = set()
            while owner != 1:
                assert owner in nodes and owner not in visited, "foreign or cyclic logical ancestry"
                visited.add(owner)
                owner = nodes[owner]["parent"]
        assert owner == 1, "incorrect actor ownership"
        projected[live] = (attrs["native_section"], node["grams"], node["condition"])
    assert projected == snapshots[-1], "native/model item records disagree"
    if preserve_initial:
        assert all(live in projected and projected[live][0] == item[0]
                   for live, item in snapshots[0].items()), "pre-transfer item lost or replaced"
    return {"status": "passed", "count": len(projected), "grams": sum(x[1] for x in projected.values()),
            "initial_identities_preserved": True if preserve_initial else None,
            "export_sha256": digest, "limits": "Final native snapshot/model agreement; no physical UI or complete gameplay claim."}


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("log", type=Path)
    ap.add_argument("export", type=Path)
    ap.add_argument("--output", type=Path, required=True)
    ap.add_argument("--preserve-initial", action="store_true", help="For acquisition-only fixtures: retain every initial live identity and section")
    args = ap.parse_args()
    report = verify(args.log, args.export, args.preserve_initial)
    args.output.write_text(json.dumps(report, indent=2)+"\n")
    print(json.dumps(report))
