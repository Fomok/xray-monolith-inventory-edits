"""Verify bounded action/definition/catalog evidence without claiming UI gestures."""
import argparse
import json
from pathlib import Path
import re
from read_native_model import read
from verify_native_snapshot import verify


def check(stem, mode):
    log=Path(str(stem)+'.log');export=Path(str(stem)+'.inv')
    text=log.read_text(errors='replace')
    run=json.loads(Path(str(stem)+'-run.json').read_text(encoding='utf-8-sig'))
    assert run['exit_code']==0 and not run['timed_out'],'unclean or timed-out run'
    result=verify(log,export,True,True,reader=read)
    nodes,_=read(export)
    if mode=='catalog':
        members={}
        for section,live,identity,grams,condition in re.findall(r'\[SQA3 CATALOG TEST\] section=(\S+) live=(\d+) persistent=(\d+) grams=(\S+) condition=(\S+)',text):
            assert section not in members and int(identity)>0,'duplicate or unregistered catalog member'
            members[section]=dict(identity=int(identity),grams=float(grams),condition=float(condition))
        assert len(members)==49 and '[SQA3 CATALOG TEST] native definitions validated=49' in text,'incomplete native catalog'
        assert len({v['identity'] for v in members.values()})==49,'catalog identity collision'
        result['catalog_members']=members
    elif mode=='definitions':
        assert '[SQA3 DEFINITION TEST] native charges/condition/upgrades/compartments PASS reload=' in text
        meds=[n for n in nodes.values() if n['attributes']['native_section']=='medkit']
        assert any(n['attributes'].get('native_uses')=='1' for n in meds)
        assert any(n['attributes'].get('native_uses')=='2' for n in meds)
        backpacks=[n for n in nodes.values() if n['attributes']['native_section']=='itm_backpack']
        assert any(n['condition']==6500 for n in backpacks)
        assert any(n['condition']==9000 for n in backpacks)
        assert any('up_firsta_pm' in n['attributes'].get('native_upgrades','').split('|') for n in nodes.values())
    else:
        for marker in ['full-pocket direct rig/pouch custody and occupied refusal PASS',
                       'source switch/disappearance availability PASS',
                       'native medical consume/refusal and ammo 7+13 accounting PASS',
                       'loaded refusal, empty native dismantle, pouch recovery and exact parts PASS']:
            assert '[SQA3 ACTION TEST] '+marker in text,'missing native action: '+marker
    result['manual_gaps']=re.findall(r'\[SQA3 MANUAL GAP\] (.+)',text)
    return nodes,result


def main():
    ap=argparse.ArgumentParser();ap.add_argument('mode',choices=['catalog','definitions','actions']);ap.add_argument('initial',type=Path)
    ap.add_argument('--reload',type=Path);ap.add_argument('--output',type=Path,required=True)
    a=ap.parse_args();first,r1=check(a.initial,a.mode);result=dict(status='passed',initial=r1)
    if a.reload:
        second,r2=check(a.reload,a.mode)
        if a.mode=='catalog':assert r1['catalog_members']==r2['catalog_members'],'catalog state changed on restart'
        for collection in [first,second]:
            for node in collection.values():node['attributes'].pop('native_live_id',None)
        assert first==second,'native instance/compartment state changed on restart'
        result['reload']=r2
    result['limits']='Bounded native routes and snapshot agreement; not physical drag/picker/New Game or long-session acceptance.'
    a.output.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8');print(json.dumps(result))


if __name__=='__main__':main()
