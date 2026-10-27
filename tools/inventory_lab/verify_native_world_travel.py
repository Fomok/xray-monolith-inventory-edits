"""Require actual outbound/return map transitions and complete loaded-bag retrieval."""
import argparse
import copy
import json
import re
from pathlib import Path
from verify_native_placement import read
from verify_native_snapshot import verify
from verify_native_world_transfer import world_records


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('stem', type=Path)
    ap.add_argument('--baseline', type=Path, required=True)
    ap.add_argument('--ground', action='store_true')
    ap.add_argument('--corpse', action='store_true')
    ap.add_argument('--stash', action='store_true')
    a = ap.parse_args()
    assert sum((a.ground,a.corpse,a.stash)) <= 1
    log = a.stem.with_suffix('.log').read_text(errors='replace')
    run = json.loads(a.stem.with_name(a.stem.name+'-run.json').read_text(encoding='utf-8-sig'))
    assert run['exit_code'] == 0 and not run['timed_out'], 'unclean process termination'
    report = verify(a.stem.with_suffix('.log'), a.stem.with_suffix('.inv'), logical_ownership=True)
    actor, _ = read(a.stem.with_suffix('.inv'))
    baseline, _ = read(a.baseline)
    world, digest = world_records(a.stem.with_suffix('.world'))
    assert not world and len(actor) == 7 and report['grams'] == 3278
    assert re.findall(r'travel resume phase=(\d+) level=(\w+)', log) == [
        ('0', 'fake_start'), ('1', 'l01_escape'), ('2', 'fake_start')]
    members = {}
    for phase, level, owner, online in re.findall(
            r'travel custody phase=(source|away|returned) level=(\w+) live=(\d+) online=(true|false) members=3', log):
        assert phase not in members
        assert level == ('l01_escape' if phase == 'away' else 'fake_start')
        assert online == ('false' if phase == 'away' else 'true')
        rows = re.findall(rf'travel member phase={phase} live=(\d+) parent=(\d+) section=(\w+) online=(true|false)', log)
        assert len(rows) == 3 and len({r[0] for r in rows}) == 3
        assert all(r[1] == owner and r[3] == online for r in rows)
        members[phase] = {r[0]: r[2] for r in rows}
    assert set(members) == {'source', 'away', 'returned'}
    # This fixture has observed stable native IDs. Reject remapping rather than
    # guessing the correspondence between its two same-section backpacks.
    assert members['source'] == members['away'] == members['returned']
    expected = copy.deepcopy(baseline)
    resets = {}
    for identity, node in expected.items():
        attrs = node['attributes']
        actual = actor[identity]['attributes']
        if attrs['native_slot'] != actual['native_slot']:
            assert attrs['native_live_id'] in members['source']
            assert attrs['native_place'] == '3' and actual['native_slot'] == '0'
            resets[str(identity)] = {'before': attrs['native_slot'], 'after': '0'}
            attrs['native_slot'] = '0'
    assert actor == expected, 'persistent item state changed'
    kind = 'stash' if a.stash else ('corpse' if a.corpse else ('ground' if a.ground else 'crate'))
    if a.corpse:
        assert len(re.findall(r'travel corpse emptied live=\d+ dead=true', log)) == 1
    for stage in ('away', 'return'):
        assert f'Game sqa3_travel_{kind}_{stage}.scop is successfully saved' in log
    assert 'travel retrieval completed level=fake_start' in log
    assert log.count('loaded transfer server committed count=3') == 1
    assert log.count('loaded transfer client completed count=3 save_ready=1') == 1
    if a.ground or a.stash:
        assert 'empty ground proxy cleanup live=' in log
    if a.stash:
        assert re.findall(r'stash travel marker phase=(\w+) present=true',log)==['source','away','returned']
        assert 'stash travel marker removed after retrieval' in log
    report.update(area_transition_tested=True, real_offline_online_cycle=True,
                  persistent_item_state_preserved=True, transient_ruck_slot_resets=resets,
                  world_sha256=digest, limits='Actual two-map travel and retrieval; no forced native-ID remapping, corpse storage or vanilla stash claim.')
    if a.corpse:
        report['limits'] = 'Actual two-map corpse custody and retrieval; no forced native-ID remapping or vanilla stash claim.'
    if a.stash:
        report['limits'] = 'Actual two-map named loaded-stash custody, marker lifecycle and retrieval; no forced native-ID remapping or complete stash parity claim.'
    a.stem.with_name(a.stem.name+'-verification.json').write_text(json.dumps(report, indent=2)+'\n')
    print(json.dumps(report))


if __name__ == '__main__':
    main()
