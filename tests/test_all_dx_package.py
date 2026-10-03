"""Exercise the actual PowerShell packager without compiling the engine."""
from pathlib import Path
import shutil, subprocess, tempfile
root=Path(__file__).resolve().parents[1]
shell=shutil.which('pwsh') or shutil.which('powershell')
assert shell, 'PowerShell required'
configs=[dx+suffix for dx in ['DX8','DX9','DX10','DX11'] for suffix in ['', '-AVX']]
solution=(root/'src/engine-vs2022.sln').read_text(encoding='utf-8-sig')
workflow=(root/'.github/workflows/amp-build.yml').read_text()
for config in configs:
 assert config+'|x64 = '+config+'|x64' in solution
assert 'configuration: ['+', '.join(configs)+']' in workflow
assert 'needs: [build, gamedata]' in workflow
assert 'fail-fast: false' in workflow
files=['bin/Anomaly'+c.replace('-','')+'.'+ext for c in configs for ext in ['exe','pdb']]+['db/mods/00_modded_exes_gamedata.db0']
with tempfile.TemporaryDirectory(prefix='sqa-package-') as temp:
 temp=Path(temp);source=temp/'incoming'
 for name in files:
  file=source/name;file.parent.mkdir(parents=True,exist_ok=True);file.write_bytes(name.encode())
 def run(dest):
  return subprocess.run([shell,'-NoProfile','-File',str(root/'.github/scripts/package-all-dx.ps1'),'-SourceRoot',str(source),'-DestinationRoot',str(dest)],capture_output=True,text=True)
 out=temp/'complete';r=run(out);assert r.returncode==0,r.stderr
 assert sorted(str(p.relative_to(out)).replace(chr(92),'/') for p in out.rglob('*') if p.is_file())==sorted(files)
 for name in files: assert (out/name).read_bytes()==(source/name).read_bytes()
 missing=source/files[0];saved=missing.read_bytes();missing.unlink()
 out=temp/'missing';r=run(out);assert r.returncode!=0 and 'Missing required file' in r.stderr and not out.exists()
 missing.write_bytes(saved);empty=source/files[-1];empty.write_bytes(b'')
 out=temp/'empty';r=run(out);assert r.returncode!=0 and 'Empty required file' in r.stderr and not out.exists()
print('PASS: eight supported configurations; complete 17-file package; missing/empty inputs rejected before staging')
