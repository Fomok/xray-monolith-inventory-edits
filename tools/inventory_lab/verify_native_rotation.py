"""Validate native menu rotation evidence, without claiming OS input coverage."""
import argparse
import json
from pathlib import Path
from verify_native_placement import read
from verify_native_snapshot import verify


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("log", type=Path)
    ap.add_argument("export", type=Path)
    ap.add_argument("--edge", action="store_true")
    ap.add_argument("--reload", type=Path)
    ap.add_argument("--output", type=Path, required=True)
    args = ap.parse_args()
    report = verify(args.log, args.export, True)
    nodes, _ = read(args.export)
    pistol = nodes[103]
    assert pistol["attributes"]["native_section"] == "wpn_pm"
    assert pistol["position"] == ([7, 6, 1] if args.edge else [5, 6, 1]), "rotation/edge placement mismatch"
    assert pistol["location"] == 0 and pistol["equipped_slots"] == 0, "pistol left the grid"
    assert report["count"] == 5 and report["grams"] == 1638
    log = args.log.read_text(errors="replace")
    assert "native reload binding dispatched" in log, "missing native binding dispatch"
    expected = "bounds" if args.edge else "accepted"
    assert "SQA3 native rotate id=103 result="+expected in log, "missing expected rotation outcome"
    if args.reload:
        restored, digest = read(args.reload)
        assert restored == nodes, "fresh restart changed item records"
        report["reloaded_sha256"] = digest
    report["edge_refusal"] = args.edge
    report["limits"] = "Native input routing and model state; OS keyboard injection and drag-time rotation are unverified."
    args.output.write_text(json.dumps(report, indent=2)+"\n")
    print(json.dumps(report))


if __name__ == "__main__":
    main()
