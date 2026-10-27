"""Generate real-content C++ fixtures and run hidden, idle, bounded contracts."""
import ctypes
import json
import os
from pathlib import Path
import subprocess
import time
import compile_rules
import extract_catalog as c

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
OUT = HERE / 'generated/native-contract'
COMPILER = Path('C:/GAMMA/sqa-previews/inventory-next-20261027/toolchain/w64devkit/bin/g++.exe')


def main():
    if os.name != 'nt':
        raise RuntimeError('Windows hidden/idle execution required')
    kernel = ctypes.windll.kernel32
    kernel.GetCurrentProcess.restype = ctypes.c_void_p
    kernel.SetPriorityClass.argtypes = [ctypes.c_void_p, ctypes.c_ulong]
    if not kernel.SetPriorityClass(kernel.GetCurrentProcess(), 0x40):
        raise RuntimeError('cannot set idle priority')
    OUT.mkdir(parents=True, exist_ok=True)
    snapshots = {name: (ROOT / 'src/inventory_next' / name).read_bytes() for name in ('model.cpp', 'model.h')}
    for name, data in snapshots.items():
        (OUT / name).write_bytes(data)
    catalog = c.build()
    rules = compile_rules.compile_catalog(catalog)
    (OUT / 'rules-v1.json').write_bytes(c.encode(rules))
    fixtures, skips = [], []
    for item in rules['items']:
        if item['geometry'] is None:
            skips.append({'id': item['id'], 'scope': 'geometry', 'reason': item['reason']})
            continue
        fixtures.append({'id': item['id'], 'geometry': item['geometry'], 'source': item['source'],
                         'capacity': sum(bool(v) for v in item['geometry']['cells']), 'scope': 'authored_geometry_only'})
        skips.append({'id': item['id'], 'scope': 'full_item_runtime', 'reason': 'host exterior/class/acceptance unresolved; synthetic test roots only',
                      'host_dependencies': item['host_dependencies']})
    for pack in rules['backpacks']:
        if pack['geometry'] is None:
            skips.append({'id': pack['key'], 'scope': 'geometry', 'reason': 'legacy scalar lacks resolved width'})
        else:
            fixtures.append({'id': 'backpack/' + pack['selector'] + '/' + pack['key'], 'geometry': pack['geometry'],
                             'capacity': pack['cells'], 'source': pack['source'], 'scope': 'authored_capacity_only'})
    for pouch in rules['pouches']:
        for mount in pouch['grant_templates']:
            for grant in mount['compartments']:
                fixtures.append({'id': pouch['id'] + '/' + grant['name'], 'geometry': grant,
                                 'capacity': len(grant['cells']), 'source': pouch.get('source', pouch['runtime_source']),
                                 'scope': 'isolated_grant_grid_not_mount_transaction'})
        skips.append({'id': pouch['id'], 'scope': 'mount_transaction', 'reason': 'mount footprint/category and atomic grant binding not resolved'})
    fixtures.sort(key=lambda f: f['id'])
    lines = ['const std::vector<Fixture> fixtures = {']
    for f in fixtures:
        g = f['geometry']
        names = ','.join('{'+key+','+json.dumps(name)+'}' for key, name in g['compartment_names'].items())
        lines.append('{'+json.dumps(f['id'])+', {'+str(g['grid']['w'])+','+str(g['grid']['h'])+'}, {'+
                     ','.join(map(str,g['cells']))+'}, {'+names+'}, '+str(f['capacity'])+'},')
    lines.append('};')
    (OUT / 'fixtures.inc').write_text('\n'.join(lines)+'\n', encoding='utf-8')
    (OUT / 'coverage.json').write_bytes(c.encode({'fixtures': fixtures, 'skips': skips, 'unresolved_source_rules': rules['unresolved'],
        'scope': 'real authored interior cells bound to shared Model via synthetic ActorRoot; probe items synthetic',
        'cases_per_fixture': ['every usable cell accepts 1x1', 'every blocked cell rejects', 'every adjacent different positive region rejects crossing',
                              'every region accepts bounding rectangle and rotated equivalent', 'all usable cells filled simultaneously',
                              'extra item rejected atomically', 'save/reload retains regions and capacity']}))
    env = dict(os.environ)
    env['PATH'] = str(COMPILER.parent) + os.pathsep + env.get('PATH', '')
    commands = [[str(COMPILER), '-std=c++17', '-O0', '-g0', '-Wall', '-Wextra', '-static', '-DNOMINMAX',
                 '-I'+str(OUT), str(HERE/'native_contract.cpp'), str(OUT/'model.cpp'), '-o', str(OUT/'contract.exe')],
                [str(OUT/'contract.exe')]]
    evidence = {'core_sources': {n:c.digest(b) for n,b in snapshots.items()}, 'rules_sha256': c.digest(c.encode(rules)),
                'compiler': str(COMPILER), 'compiler_sha256': c.digest(COMPILER.read_bytes()),
                'fixture_count': len(fixtures), 'skip_count': len(skips), 'game_validation': 'not_run', 'runs': [],
                'test_sources': {name:c.digest((HERE/name).read_bytes()) for name in ('native_contract.cpp', 'run_native_contract.py', 'compile_rules.py', 'extract_catalog.py')}}
    for index, command in enumerate(commands):
        started = time.monotonic()
        timeout = 40 if index == 0 else 10
        try:
            result = subprocess.run(command, cwd=OUT, env=env, capture_output=True, timeout=timeout,
                                    creationflags=subprocess.CREATE_NO_WINDOW | subprocess.IDLE_PRIORITY_CLASS)
            stdout, stderr = result.stdout, result.stderr
            code = result.returncode
        except subprocess.TimeoutExpired as exc:
            stdout, stderr, code = exc.stdout or b'', exc.stderr or b'', 'timeout'
        (OUT/f'step-{index}.stdout.txt').write_bytes(stdout)
        (OUT/f'step-{index}.stderr.txt').write_bytes(stderr)
        evidence['runs'].append({'command': command, 'returncode': code, 'seconds': time.monotonic()-started,
                                'timeout_seconds': timeout, 'creationflags': ['CREATE_NO_WINDOW', 'IDLE_PRIORITY_CLASS']})
        if code != 0:
            break
    current = {n:c.digest((ROOT/'src/inventory_next'/n).read_bytes()) for n in snapshots}
    evidence['core_hashes_after'] = current
    evidence['source_stable_during_run'] = current == evidence['core_sources']
    evidence['passed'] = len(evidence['runs']) == 2 and all(r['returncode'] == 0 for r in evidence['runs']) and evidence['source_stable_during_run']
    if len(evidence['runs']) == 2:
        output = (OUT/'step-1.stdout.txt').read_text()
        evidence['passed_contract_cases'] = sum(line.startswith('PASS\t') for line in output.splitlines())
        evidence['environment_verified'] = 'ENV\thidden_no_console\tidle_priority' in output
    evidence['generated_files'] = {p.name:c.digest(p.read_bytes()) for p in sorted(OUT.iterdir()) if p.is_file() and p.name != 'evidence.json'}
    (OUT/'evidence.json').write_bytes(c.encode(evidence))
    print(json.dumps(evidence, indent=2))
    return 0 if evidence['passed'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
