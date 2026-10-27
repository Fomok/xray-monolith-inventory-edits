"""Bounded Windows headless runner integration checks; no game/window/input control."""
import argparse, hashlib, json, pathlib, shutil, struct, subprocess, tempfile
p=argparse.ArgumentParser();p.add_argument('--build',type=pathlib.Path,required=True);p.add_argument('--output',type=pathlib.Path,required=True);a=p.parse_args()
a.output.mkdir(parents=True,exist_ok=True)
root=pathlib.Path(tempfile.mkdtemp(prefix='runner-checks-',dir=a.output))
scenarios=pathlib.Path(__file__).parent/'scenarios'
checks=[]
def launch(exe,scenario,dest,resume=None,expected=True,error=None):
    args=[str(exe),'--scenario',str(scenario),'--output',str(dest)]
    if resume:args+=['--resume',str(resume)]
    r=subprocess.run(args,capture_output=True,timeout=40,creationflags=subprocess.CREATE_NO_WINDOW|subprocess.IDLE_PRIORITY_CLASS)
    assert (r.returncode==0)==expected,(args,r.returncode)
    assert not r.stdout and not r.stderr,'GUI runner emitted console text'
    result=json.loads((dest/'result.json').read_text())
    assert result['success']==expected,result
    if expected:
        assert result['priority_class']==64 and not result['console_attached'],result
        events=[json.loads(x) for x in (dest/'events.jsonl').read_text().splitlines()]
        assert events and all(x['pass'] for x in events)
    if error:assert result['error']==error,result
    checks.append({'case':dest.name,'result':result,'exit':r.returncode})
    return result
for variant in ['debug','release']:
    exe=a.build/f'inventory-lab-{variant}.exe'
    data=exe.read_bytes();pe=struct.unpack_from('<I',data,0x3c)[0]
    assert struct.unpack_from('<H',data,pe+24+68)[0]==2,'runner is not Windows GUI subsystem'
    dest=root/f'{variant}-lifecycle';launch(exe,scenarios/'lifecycle.sqa',dest)
    launch(exe,scenarios/'resume.sqa',root/f'{variant}-resume',dest/'final.inv')
    launch(exe,scenarios/'rigid.sqa',root/f'{variant}-rigid')
    launch(exe,scenarios/'stacks-equipment.sqa',root/f'{variant}-stacks-equipment')
    launch(exe,scenarios/'crafting.sqa',root/f'{variant}-crafting')
    launch(exe,scenarios/'swap.sqa',root/f'{variant}-swap')
    launch(exe,scenarios/'compartments.sqa',root/f'{variant}-compartments')
    launch(exe,scenarios/'pouches.sqa',root/f'{variant}-pouches')
    launch(exe,scenarios/'overflow.sqa',root/f'{variant}-overflow')
    launch(exe,scenarios/'condition-recovery.sqa',root/f'{variant}-condition-recovery')
    view=json.loads((root/f'{variant}-stacks-equipment'/'snapshot-restored.json').read_text())
    assert len(view['items'])==3 and view['carried_grams']==210
    assert sum(n['quantity'] for n in view['items'] if n['type']==100)==20
exe=a.build/'inventory-lab-release.exe'
base='SQA3_SCENARIO 1\nroot 1 actor 4 4 0 1\nbegin\n'
cases={
 'bad-header':('WRONG\n','scenario_header'),
 'bad-number':('SQA3_SCENARIO 1\nroot -1 actor 4 4 0 1\nbegin\n','invalid_number'),
 'unknown-command':(base+'launch_game\n','unknown_command'),
 'path-traversal':(base+'save ../escape\n','snapshot_name'),
 'device-path':(base+'save CON\n','snapshot_name'),
 'assert-failure':(base+'assert mass 1\n','assertion'),
 'too-many-lines':(base+('assert mass 0\n'*513),'scenario_limit'),
 'too-large':('x'*131073,'scenario_size'),
 'missing-begin':('SQA3_SCENARIO 1\nassert mass 0\n','begin_required'),
 'empty-scenario':(base,'empty_scenario'),
}
for name,(content,error) in cases.items():
    scenario=root/(name+'.sqa');scenario.write_text(content)
    launch(exe,scenario,root/name,expected=False,error=error)
# Output reuse must fail before touching previous results or snapshots.
existing=root/'release-lifecycle';before={p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in existing.iterdir()}
r=subprocess.run([str(exe),'--scenario',str(scenarios/'rigid.sqa'),'--output',str(existing)],timeout=40,creationflags=subprocess.CREATE_NO_WINDOW|subprocess.IDLE_PRIORITY_CLASS)
assert r.returncode!=0
assert before=={p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in existing.iterdir()}
checks.append({'case':'existing-output-preserved','pass':True})
# Exercise the supported asynchronous launcher inside a hidden PowerShell process.
launchroot=root/'launcher';ps=pathlib.Path(__file__).parent/'start-background.ps1'
r=subprocess.run(['powershell.exe','-NoProfile','-NonInteractive','-ExecutionPolicy','Bypass','-File',str(ps),'-Runner',str(exe),'-ResultsRoot',str(launchroot)],capture_output=True,timeout=50,creationflags=subprocess.CREATE_NO_WINDOW|subprocess.IDLE_PRIORITY_CLASS)
assert r.returncode==0,(r.returncode,r.stderr)
record=json.loads((launchroot/'last-run.json').read_text(encoding='utf-8-sig'))
assert json.loads(pathlib.Path(record['result']).read_text())['success']
checks.append({'case':'hidden-launcher','pass':True,'record':record})
# Native no-argument entry needs no script host or terminal launcher.
portable=root/'portable app with spaces';portable.mkdir()
shutil.copy2(exe,portable/exe.name);shutil.copytree(scenarios,portable/'scenarios')
r=subprocess.run([str(portable/exe.name)],capture_output=True,timeout=40,creationflags=subprocess.CREATE_NO_WINDOW|subprocess.IDLE_PRIORITY_CLASS)
assert r.returncode==0 and not r.stdout and not r.stderr
outputs=list((portable/'runs').iterdir());assert len(outputs)==1
outcome=json.loads((outputs[0]/'result.json').read_text());assert outcome['success'] and not outcome['console_attached'] and outcome['priority_class']==64
checks.append({'case':'native-no-arguments','pass':True,'result':outcome})
report={'checks':checks,'passed':len(checks),'evidence':str(root),'gameLaunched':False,'PEsubsystem':'Windows GUI','timeoutSecondsPerRun':40}
(root/'runner-validation.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps({'passed':len(checks),'report':str(root/'runner-validation.json')}))
