"""Verify nested loaded-bag custody using native snapshots and both saved models."""
import argparse
import hashlib
import io
import json
import math
import re
import tempfile
import zlib
from pathlib import Path
from verify_native_placement import read
from verify_native_snapshot import verify


def world_records(path):
    raw = path.read_bytes()
    body, footer = raw.rsplit(b"CRC32 ", 1)
    assert zlib.crc32(body) == int(footer.strip(), 16), "world ledger checksum"
    stream = io.BytesIO(body)
    header = stream.readline().split()
    assert header[:2] == [b"SQA3_WORLD_STORAGE", b"1"]
    records, all_ids = {}, set()
    for _ in range(int(header[2])):
        root, custodian, length = map(int, stream.readline().split())
        payload = stream.read(length)
        assert len(payload) == length and root not in records
        with tempfile.TemporaryDirectory(dir=path.parent, prefix="world-verify-") as directory:
            model_path = Path(directory) / "world.inv"
            model_path.write_bytes(payload)
            nodes, _ = read(model_path)
        assert not all_ids.intersection(nodes)
        all_ids.update(nodes)
        records[root] = (custodian, nodes)
    assert not stream.read(), "trailing world records"
    return records, hashlib.sha256(raw).hexdigest()


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("log", type=Path)
    ap.add_argument("export", type=Path)
    ap.add_argument("--world", type=Path, help="Required for transfers; optional for Dev18 preparation only")
    ap.add_argument("--baseline", type=Path, required=True)
    ap.add_argument("--phase", choices=["prepared", "release", "return", "nofit", "rollback-release", "rollback-return", "timeout"], required=True)
    ap.add_argument("--ground", action="store_true", help="Verify dedicated ground-proxy lifecycle as well as contents")
    ap.add_argument("--travel-blocked", action="store_true", help="Require a real pending-transfer level-change refusal")
    ap.add_argument("--corpse", action="store_true")
    ap.add_argument("--destroy-retained", action="store_true")
    ap.add_argument("--previous", type=Path)
    ap.add_argument("--output", type=Path, required=True)
    a = ap.parse_args()
    report = verify(a.log, a.export, logical_ownership=True)
    actor, _ = read(a.export)
    baseline, _ = read(a.baseline)
    assert a.world or a.phase == "prepared", "transfer requires world ledger evidence"
    records, world_hash = world_records(a.world) if a.world else ({}, None)
    log = a.log.read_text(errors="replace")
    if a.travel_blocked:
        assert a.ground and a.phase == "timeout" and not a.previous
        assert "attempting real level change while ground provisioning is pending" in log
        assert "SQA3 level change refused: inventory state is not saveable" in log
        assert "pending inventory travel blocked; original level retained" in log
        assert "Game sqa3_ground_timeout.scop is successfully saved" in log
        run = json.loads(a.log.with_name(a.log.stem+"-run.json").read_text(encoding="utf-8-sig"))
        assert run['exit_code'] == 0 and not run['timed_out']
        report['pending_transfer_travel_refused'] = True
    if a.phase == "prepared":
        assert len(actor) == 7 and report["grams"] == 3278 and not records
        added = set(actor) - set(baseline)
        assert len(added) == 1
        nested = added.pop()
        assert actor[nested]["attributes"]["native_section"] == "itm_backpack"
        assert actor[nested]["parent"] == 105 and actor[nested]["position"] == [4, 4, 0]
        expected_medkit = dict(baseline[102], parent=nested, position=[1, 1, 0])
        assert actor[102] == expected_medkit
        assert all(actor[key] == value for key, value in baseline.items() if key != 102)
        assert "prepared world nested bag" in log
        report["nested_bag_identity"] = nested
    else:
        assert len(baseline) == 7
        subtree = {}
        for key, node in baseline.items():
            owner = key
            while owner in baseline and owner != 105:
                owner = baseline[owner]["parent"]
            if owner == 105:
                subtree[key] = node
        assert len(subtree) == 3 and sum(n["grams"] for n in subtree.values()) == 2140
        result = re.search(r"world transfer phase=(release|return|nofit|rollback-release|rollback-return|timeout) crate=(\d+) actor=(\d+) members=3", log)
        assert result and result[1] == a.phase
        crate, actor_live = int(result[2]), int(result[3])
        if a.corpse:
            assert not a.ground
            counts = re.findall(rf'corpse custodian live={crate} dead=true native_items=(\d+)', log)
            assert counts == ['0' if a.phase in ('return', 'rollback-release') else '3']
            assert f'Game sqa3_corpse_{a.phase.replace("-", "_")}.scop is successfully saved' in log
            report['native_corpse_custody'] = True
        if a.destroy_retained:
            assert a.phase == 'release' and not a.previous
            assert f'requesting real loaded-custodian destruction live={crate}' in log
            assert f'SQA3 retained stored inventory during destroy event live={crate}' in log
            attempted = re.findall(r'requesting real loaded-member destruction live=(\d+)', log)
            assert len(attempted) == len(set(attempted)) == 3
            assert all(f'SQA3 retained stored inventory during destroy event live={live}' in log for live in attempted)
            report['online_cleanup_retained_loaded_custodian'] = True
        rows = re.findall(r"world member live=(\d+) owner=(\d+) section=(\S+) grams=(\S+) condition=(\S+)", log)
        assert len(rows) == 3 and len({row[0] for row in rows}) == 3
        observed = {live: (section, math.floor(float(grams)+0.5), math.floor(float(condition)*10000+0.5))
                    for live, owner, section, grams, condition in rows}
        expected = {n["attributes"]["native_live_id"]: (n["attributes"]["native_section"], n["grams"], n["condition"])
                    for n in subtree.values()}
        assert observed == expected
        actor_has_bag = a.phase in ("return", "rollback-release", "timeout")
        assert all(int(row[1]) == (actor_live if actor_has_bag else crate) for row in rows)
        if not actor_has_bag:
            retained = {key: n for key, n in baseline.items() if key not in subtree}
            assert all(actor.get(key) == n for key, n in retained.items())
            if a.phase == "nofit":
                assert len(actor) == 10 and report["grams"] == 1366
                fillers = set(actor)-set(retained)
                assert len(fillers) == 6 and not fillers.intersection(subtree)
                assert all(actor[key]["attributes"]["native_section"] == "bandage" and
                           actor[key]["grams"] == 38 and actor[key]["condition"] == 10000 and
                           actor[key]["parent"] == 1 for key in fillers)
            else:
                assert actor == retained and len(actor) == 4 and report["grams"] == 1138
            assert set(records) == {105}
            custody, outside = records[105]
            expected_world = dict(subtree)
            expected_world[105] = dict(subtree[105], parent=2, position=[0, 0, 0])
            assert outside == expected_world and custody >= 100
            assert f"world bag id=105 custodian={custody} items=3 grams=2140" in log
            report["persistent_custodian"] = custody
        else:
            assert actor == baseline and not records and report["grams"] == 3278
        if a.previous:
            previous, _ = read(a.previous)
            assert actor == previous
            previous_world, _ = world_records(a.previous.with_suffix(".world"))
            assert records == previous_world
            report["exact_actor_and_world_reload"] = True
        elif a.phase == "timeout":
            assert a.ground and "ground bag provisioning shell=" in log
            assert "empty ground proxy cleanup live=" in log
            assert "loaded transfer server committed" not in log
            assert "loaded transfer fixture interrupted" not in log
            report["ground_provision_timeout_retained_source"] = True
        elif a.phase.startswith("rollback-"):
            assert log.count("loaded transfer fixture interrupted after moved=1 before client notification") == 1
            assert log.count("loaded transfer rejection restored count=3 save_ready=1") == 1
            assert "loaded transfer server committed" not in log
            assert "loaded transfer client completed" not in log
            report["native_partial_transfer_rolled_back"] = True
        elif a.phase == "nofit":
            assert "loaded transfer refused reason=" in log
            assert "loaded transfer server committed" not in log
            assert "loaded transfer client completed" not in log
            report["full_storage_refusal_preserved_subtree"] = True
        else:
            assert log.count("loaded transfer server committed count=3") == 1
            assert log.count("loaded transfer client completed count=3 save_ready=1") == 1
        report.update(crate=crate, complete_nested_subtree_preserved=True)
        if a.ground:
            assert f"ground proxy present={0 if actor_has_bag else 1} live=" in log
            if not a.previous and a.phase in ("return", "rollback-release"):
                assert "empty ground proxy cleanup live=" in log
            report["ground_proxy_lifecycle_observed"] = True
    report.update(world_sha256=world_hash, world_persistence_observed=a.world is not None, phase=a.phase,
                  limits=("Bounded native ground-proxy lifecycle; vanilla Create stash, corpse custody and broad travel remain unverified." if a.ground else "Bounded nested actor/crate transfer and persistence. Physical UI requires separate inspection; ground custody and broader gameplay remain unverified."))
    if a.corpse:
        report['limits'] = 'Opt-in loaded corpse custody and recorded cleanup checks only; no vanilla stash or complete corpse lifecycle claim.'
        run = json.loads(a.log.with_name(a.log.stem+'-run.json').read_text(encoding='utf-8-sig'))
        assert run['exit_code'] == 0 and not run['timed_out']
    a.output.write_text(json.dumps(report, indent=2)+"\n")
    print(json.dumps(report))


if __name__ == "__main__":
    main()
