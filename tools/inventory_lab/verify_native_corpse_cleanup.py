"""Require native empty-corpse deletion and unchanged carried storage."""
import argparse
import json
import re
from pathlib import Path
from verify_native_snapshot import verify
from verify_native_placement import read
from verify_native_world_transfer import world_records


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('stem', type=Path)
    ap.add_argument('--baseline', type=Path, required=True)
    ap.add_argument('--reload', action='store_true')
    a = ap.parse_args()
    log = a.stem.with_suffix('.log').read_text(errors='replace')
    run = json.loads(a.stem.with_name(a.stem.name+'-run.json').read_text(encoding='utf-8-sig'))
    assert run['exit_code'] == 0 and not run['timed_out']
    report = verify(a.stem.with_suffix('.log'), a.stem.with_suffix('.inv'), logical_ownership=True)
    assert read(a.stem.with_suffix('.inv'))[0] == read(a.baseline)[0]
    assert not world_records(a.stem.with_suffix('.world'))[0]
    assert len(re.findall(r'empty corpse cleanup completed live=\d+', log)) == 1
    assert 'Game sqa3_corpse_cleaned.scop is successfully saved' in log
    assert ('-sqa3-cleanup-reload' in run['arguments']) == a.reload
    report.update(empty_corpse_cleanup=True, exact_actor_state=True, fresh_reload=a.reload,
                  limits='Empty corpse absent after cleanup; exact carried-state comparison. No broader corpse lifetime claim.')
    a.stem.with_name(a.stem.name+'-verification.json').write_text(json.dumps(report, indent=2)+'\n')
    print(json.dumps(report))


if __name__ == '__main__':
    main()
