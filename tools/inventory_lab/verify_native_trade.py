"""Verify exact-item sale/buyback and native money accounting evidence."""
import argparse
import json
import re
from pathlib import Path
from verify_native_placement import read
from verify_native_snapshot import verify


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("sale_log", type=Path)
    ap.add_argument("sold", type=Path)
    ap.add_argument("buy_log", type=Path)
    ap.add_argument("bought", type=Path)
    ap.add_argument("--output", type=Path, required=True)
    args = ap.parse_args()
    sale = verify(args.sale_log, args.sold, False)
    buy = verify(args.buy_log, args.bought, True)
    sold, _ = read(args.sold)
    bought, _ = read(args.bought)
    assert sale["count"] == 4 and sale["grams"] == 1448
    assert buy["count"] == 5 and buy["grams"] == 1638
    assert set(bought)-set(sold) == {104}, "bread core identity did not return"
    assert all(bought[k] == v for k, v in sold.items()), "untraded item records changed"
    s = re.search(r"native sale verified live=(\d+) actor_gain=(\d+) partner_loss=(\d+)", args.sale_log.read_text(errors="replace"))
    b = re.search(r"native buyback verified live=(\d+) actor_loss=(\d+) partner_gain=(\d+)", args.buy_log.read_text(errors="replace"))
    assert s and b and s[1] == b[1] == bought[104]["attributes"]["native_live_id"]
    assert int(s[2]) > 0 and s[2] == s[3] and int(b[2]) > 0 and b[2] == b[3]
    report = dict(status="passed", sale=sale, buyback=buy, live_id=int(s[1]),
                  sale_ru=int(s[2]), buy_ru=int(b[2]),
                  limits="Stationary disposable NPC; fixture grants test funds. No strict capacity admission or complete trading parity claim.")
    args.output.write_text(json.dumps(report, indent=2)+"\n")
    print(json.dumps(report))


if __name__ == "__main__":
    main()
