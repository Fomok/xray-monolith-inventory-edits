"""Bounded hidden-process diagnostic renderer checks; no game or desktop capture."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess

parser = argparse.ArgumentParser()
parser.add_argument('--exe', type=Path, required=True)
parser.add_argument('--snapshot', type=Path, required=True)
parser.add_argument('--output', type=Path, required=True)
args = parser.parse_args()
args.output.mkdir(parents=True, exist_ok=False)
before = hashlib.sha256(args.snapshot.read_bytes()).hexdigest()
checks = []

def run(extra):
    return subprocess.run([str(args.exe), *map(str, extra)], timeout=15,
                          creationflags=subprocess.CREATE_NO_WINDOW,
                          capture_output=True).returncode

def check(label, condition):
    assert condition, label
    checks.append(label)

image = args.output / 'model.bmp'
base = ['--snapshot', args.snapshot, '--output', image]
check('valid snapshot renders successfully', run(base) == 0)
data = image.read_bytes()
check('native BMP dimensions and pixel payload',
      data[:2] == b'BM' and struct.unpack_from('<ii', data, 18) == (1600, -1100)
      and len(data) == 54 + 1600 * 1100 * 4)
check('image contains more than background', len(set(data[54:])) > 8)
check('existing output refused and preserved', run(base) != 0 and image.read_bytes() == data)
invalid = args.output / 'bad.inv'
invalid.write_text('not a model', encoding='utf8')
bad_output = args.output / 'bad.bmp'
check('invalid snapshot produces no image', run(['--snapshot', invalid, '--output', bad_output]) != 0
      and not bad_output.exists())
check('invalid snapshot records reason', 'invalid_snapshot' in Path(str(bad_output) + '.error.txt').read_text())
missing_output = args.output / 'missing.bmp'
check('missing selected container rejected', run(['--snapshot', args.snapshot, '--output', missing_output,
      '--container', '999999999']) != 0 and not missing_output.exists())
wrong_output = args.output / 'wrong-root.bmp'
check('wrong root kind rejected', run(['--snapshot', args.snapshot, '--output', wrong_output,
      '--actor', '2']) != 0 and not wrong_output.exists())
check('malformed arguments rejected', run(['--unknown', 'value']) == 2)
check('source snapshot bytes unchanged', hashlib.sha256(args.snapshot.read_bytes()).hexdigest() == before)
(args.output / 'validation.json').write_text(json.dumps({
    'checks': checks, 'passed': len(checks), 'snapshotSha256': before,
    'executableSha256': hashlib.sha256(args.exe.read_bytes()).hexdigest(),
    'imageSha256': hashlib.sha256(data).hexdigest(), 'gameLaunched': False,
    'limit': 'Diagnostic GDI rendering, not Anomaly rendering or interactive UI.'
}, indent=2), encoding='utf8')
print(f'PASS {len(checks)} snapshot renderer checks')
