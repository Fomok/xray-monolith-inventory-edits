"""Verify named loaded-stash evidence; UI observation is recorded separately."""
import argparse
import io
import json
import re
from pathlib import Path
from verify_native_placement import read
from verify_native_snapshot import verify
from verify_native_world_transfer import world_records


def names(path):
    stream = io.BytesIO(path.read_bytes().rsplit(b"CRC32 ", 1)[0])
    count = int(stream.readline().split()[2])
    result = {}
    for _ in range(count):
        root, _, length = map(int, stream.readline().split())
        payload = stream.read(length).decode().splitlines()
        rows = [row.split() for row in payload[1:] if row.startswith("2 ")]
        assert len(rows) == 1, "world root metadata missing"
        row = rows[0]
        if "stash_name" in row:
            value = row[row.index("stash_name")+1]
            assert value.startswith("x")
            result[root] = bytes.fromhex(value[1:]).decode()
    return result


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("stem", type=Path)
    ap.add_argument("--baseline", type=Path, required=True)
    ap.add_argument("--previous", type=Path)
    a = ap.parse_args()
    log_path = a.stem.with_suffix(".log")
    actor_path = a.stem.with_suffix(".inv")
    world_path = a.stem.with_suffix(".world")
    log = log_path.read_text(errors="replace")
    report = verify(log_path, actor_path, logical_ownership=True)
    actor, _ = read(actor_path)
    baseline, _ = read(a.baseline)
    records, world_hash = world_records(world_path)
    run = json.loads(Path(str(a.stem)+"-run.json").read_text(encoding="utf-8-sig"))
    assert run["exit_code"] == 0 and not run["timed_out"], "supervised run failed"
    matches = re.findall(r"\[SQA3 STASH TEST\] completed phase=(\w+) empty=false carried=(true|false) created=(\d+) removed=(\d+)", log)
    assert len(matches) == 1, "loaded stash fixture did not complete"
    phase, carried, created, removed = matches[0]
    carried = carried == "true"
    assert f"Game sqa3_stash_{phase}.scop is successfully saved" in log
    assert len(baseline) == 7 and set(baseline) == set(range(100, 107))
    subtree = {key: baseline[key] for key in (102, 105, 106)}
    retained = {key: value for key, value in baseline.items() if key not in subtree}
    if carried:
        assert not records and not names(world_path)
        assert actor == baseline, "carried state or identity changed"
    else:
        assert actor == retained and set(records) == {105}
        custodian, outside = records[105]
        expected = dict(subtree)
        expected[105] = dict(subtree[105], parent=2, position=[0, 0, 0])
        assert outside == expected and custodian >= 100
        assert names(world_path) == {105: "SQA3 field stash"}
        assert "stored members=3 marker=true" in log
    if a.previous:
        previous_actor, _ = read(a.previous.with_suffix(".inv"))
        previous_world, _ = world_records(a.previous.with_suffix(".world"))
        assert actor == previous_actor and records == previous_world
        assert names(world_path) == names(a.previous.with_suffix(".world"))
        assert created == removed == "0"
        report["exact_reload"] = True
    elif phase in ("release", "return"):
        assert log.count("loaded transfer server committed count=3") == 1
        assert log.count("loaded transfer client completed count=3 save_ready=1") == 1
        assert (created, removed) == (("1", "0") if phase == "release" else ("0", "1"))
    elif phase.startswith("rollback_"):
        assert log.count("loaded transfer fixture interrupted after moved=1 before client notification") == 1
        assert log.count("loaded transfer rejection restored count=3 save_ready=1") == 1
        assert "loaded transfer server committed" not in log
    elif phase == "timeout":
        assert "ground bag provisioning shell=" in log and "empty ground proxy cleanup live=" in log
        assert "loaded transfer server committed" not in log
    elif phase == "cancel":
        assert "ground bag provisioning shell=" not in log
        assert created == removed == "0"
    else:
        raise AssertionError("unsupported phase")
    report.update(phase=phase, world_sha256=world_hash, original_shell_and_nested_state_preserved=True,
                  limits="Loaded stash fixture only; physical dialog/drag observation is separate. Empty/equipped bags, travel and full parity are not inferred.")
    Path(str(a.stem)+"-verification.json").write_text(json.dumps(report, indent=2)+"\n")
    print(json.dumps(report))


if __name__ == "__main__":
    main()
