import argparse
import json
import pathlib
import subprocess

p = argparse.ArgumentParser()
p.add_argument('--compiler', required=True)
p.add_argument('--output', required=True)
a = p.parse_args()
root = pathlib.Path(__file__).resolve().parents[3]
out = pathlib.Path(a.output).resolve()
out.mkdir(parents=True, exist_ok=True)
flags = subprocess.CREATE_NO_WINDOW | subprocess.IDLE_PRIORITY_CLASS
results = []
for mode, options in [('debug', ['-O0', '-g', '-D_GLIBCXX_ASSERTIONS']), ('release', ['-O2', '-DNDEBUG'])]:
    exe = out / ('drag-' + mode + '.exe')
    cmd = [a.compiler, '-std=c++17', '-Wall', '-Wextra', '-Werror', '-static', *options,
           '-I' + str(root / 'src/inventory_next'), str(root / 'src/inventory_next/model.cpp'), str(root / 'src/inventory_next/drag_session.cpp'),
           str(pathlib.Path(__file__).with_name('tests.cpp')), '-o', str(exe)]
    build = subprocess.run(cmd, capture_output=True, text=True, timeout=60, creationflags=flags)
    (out / (mode + '-build.log')).write_text(build.stdout + build.stderr)
    if build.returncode:
        raise RuntimeError(build.stderr)
    run = subprocess.run([str(exe)], capture_output=True, text=True, timeout=15, creationflags=flags)
    (out / (mode + '-tests.log')).write_text(run.stdout + run.stderr)
    if run.returncode:
        raise RuntimeError(run.stdout + run.stderr)
    results.append({'mode': mode, 'scenario_groups': 5, 'passed': True})
(out / 'validation.json').write_text(json.dumps(results, indent=2))
print(json.dumps(results, indent=2))

