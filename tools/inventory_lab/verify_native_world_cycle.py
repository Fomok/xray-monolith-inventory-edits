"""Verify real ALife lifetime transitions separately from area travel."""
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
    ap.add_argument('--destroy-retained', action='store_true')
    ap.add_argument('--resume-offline', action='store_true')
    a = ap.parse_args()
    log = a.stem.with_suffix('.log').read_text(errors='replace')
    report = verify(a.stem.with_suffix('.log'), a.stem.with_suffix('.inv'), logical_ownership=True)
    actor, _ = read(a.stem.with_suffix('.inv'))
    baseline, _ = read(a.baseline)
    world, digest = world_records(a.stem.with_suffix('.world'))
    assert not world and len(actor) == 7 and report['grams'] == 3278
    stages = {}
    pattern = r'cycle member phase=(initial|offline|online) before=(\d+) live=(\d+) parent=(\d+) section=(\w+) online=(true|false)'
    for phase, old, live, parent, section, online in re.findall(pattern, log):
        stage = stages.setdefault(phase, {})
        assert int(old) not in stage
        stage[int(old)] = (int(live), int(parent), section, online)
    assert set(stages) == {'initial', 'offline', 'online'}
    initial = stages['initial']
    assert len(initial) == 3 and all(set(rows) == set(initial) for rows in stages.values())
    for phase, rows in stages.items():
        custodians = re.findall(rf'cycle custodian phase={phase} before=(\d+) live=(\d+) online=(true|false) level=(\w+)', log)
        assert len(custodians) == 1
        _, owner, online, _ = custodians[0]
        expected_online = 'false' if phase == 'offline' or (a.resume_offline and phase == 'initial') else 'true'
        assert online == expected_online
        assert all(row[1] == int(owner) and row[3] == expected_online and row[2] == initial[old][2] for old, row in rows.items())
    drift = {}
    expected = copy.deepcopy(baseline)
    for identity, node in expected.items():
        attrs = node['attributes']; old = int(attrs['native_live_id'])
        if old in initial:
            attrs['native_live_id'] = str(stages['online'][old][0])
            actual_slot = actor[identity]['attributes']['native_slot']
            # CurrSlot is transient for a rucksack item after net_Destroy/net_Spawn.
            # Permit only its native reset to zero; equipment and all core state stay exact.
            if actual_slot != attrs['native_slot']:
                assert attrs['native_place'] == '3' and actual_slot == '0'
                drift[str(identity)] = {'before': attrs['native_slot'], 'after': actual_slot}
                attrs['native_slot'] = actual_slot
    assert actor == expected
    assert not (a.ground and a.corpse)
    kind = 'corpse' if a.corpse else ('ground' if a.ground else 'crate')
    if a.corpse and not a.resume_offline:
        assert len(re.findall(r'explicit native corpse offline transition live=\d+', log)) == 1
        report['corpse_offline_transition_explicit'] = True
    if a.destroy_retained:
        requested = re.findall(r'requesting real offline-custodian destruction live=(\d+)', log)
        assert len(requested) == 1
        assert f'SQA3 retained stored inventory during ALife release live={requested[0]}' in log
        attempted = re.findall(r'requesting real offline-member destruction live=(\d+)', log)
        assert len(attempted) == len(set(attempted)) == 3
        assert all(f'SQA3 retained stored inventory during ALife release live={live}' in log for live in attempted)
        report['offline_cleanup_retained_loaded_custodian'] = True
    assert f'Game sqa3_cycle_{kind}_offline.scop is successfully saved' in log
    assert f'Game sqa3_cycle_{kind}_return.scop is successfully saved' in log
    assert log.count('loaded transfer server committed count=3') == 1
    assert log.count('loaded transfer client completed count=3 save_ready=1') == 1
    assert len(re.findall(r'cycle returned before=', log)) == 3
    if a.ground:
        assert 'empty ground proxy cleanup live=' in log
    if a.resume_offline:
        assert len(re.findall(r'fresh process resumed offline custody live=\d+', log)) == 1
        report['fresh_process_offline_save_resumed'] = True
    run = json.loads(a.stem.with_name(a.stem.name+'-run.json').read_text(encoding='utf-8-sig'))
    assert run['exit_code'] == 0 and not run['timed_out']
    report.update(real_offline_online_cycle=True, area_transition_tested=False,
                  persistent_item_state_preserved=True, transient_ruck_slot_resets=drift,
                  world_sha256=digest, limits='Real ALife offline/online lifetime cycle and retrieval; not area travel. Physical UI observed separately.')
    a.stem.with_name(a.stem.name+'-verification.json').write_text(json.dumps(report, indent=2)+'\n')
    print(json.dumps(report))


if __name__ == '__main__':
    main()
