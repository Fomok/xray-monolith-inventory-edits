"""Verify the isolated finite-capacity preview; not native pickup enforcement."""
import argparse
import json
import re
from pathlib import Path
from verify_native_placement import read
from verify_native_snapshot import verify


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("log", type=Path)
    ap.add_argument("native", type=Path)
    ap.add_argument("preview", type=Path)
    ap.add_argument("--output", type=Path, required=True)
    args = ap.parse_args()
    report = verify(args.log, args.native, True)
    log = args.log.read_text(errors="replace")
    rows = re.findall(r"SQA3 capacity preview pack=(\S+) grid=(\d+)x(\d+) legal=(\d+) recovery=(\d+) grams=(\d+) export=saved; preflight only", log)
    assert len(rows) == 3, "missing capacity probe stage"
    assert [r[:3] for r in rows] == [("none", "4", "2"), ("itm_backpack", "8", "8"), ("none", "4", "2")]
    native, _ = read(args.native)
    preview, digest = read(args.preview)
    assert set(native) == set(preview), "preview lost or duplicated items"
    for identity, item in native.items():
        for field in ("parent", "grams", "condition", "attributes"):
            assert item[field] == preview[identity][field], "preview changed native item facts"
    packs = [v for v in native.values() if v["attributes"]["native_section"] == "itm_backpack"]
    assert len(packs) == 1
    assert int(rows[1][5]) == int(rows[2][5]) == report["grams"]
    assert int(rows[1][5])-int(rows[0][5]) == packs[0]["grams"], "mass changed across equipment transition"
    assert int(rows[0][4]) > 0 and int(rows[1][4]) == 0 and int(rows[2][4]) > 0, "missing recovery/growth/shrink transition"
    assert sum(v["grams"] for v in preview.values()) == report["grams"]
    report.update(stages=rows, preview_sha256=digest,
                  limits="Separate finite preview only. Active inventory ownership and native pickup admission are unchanged.")
    args.output.write_text(json.dumps(report, indent=2)+"\n")
    print(json.dumps(report))


if __name__ == "__main__":
    main()
