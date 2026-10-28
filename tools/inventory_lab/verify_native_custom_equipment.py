"""Compare actual custom bindings and complete instance state after a restart."""
import argparse
import json
import re
from pathlib import Path
from read_native_model import read
from verify_native_snapshot import verify


def check(log,export):
    result=verify(log,export,True,True,reader=read)
    text=log.read_text(errors='replace');items={}
    for key,live,identity in re.findall(r'\[SQA3 CUSTOM TEST\] item=(\w+) live=(\d+) persistent=(\d+)',text):
        assert key not in items,'duplicate fixture declaration';items[key]=int(identity)
    assert set(items)=={'rig','spare','pouch','right','bandage'},'incomplete custom fixture'
    assert '[SQA3 CUSTOM TEST] native bindings verified reload=' in text,'native bindings unchecked'
    nodes,_=read(export);rig=nodes[items['rig']]
    assert rig['parent']==1 and rig['location']==1 and rig['attributes']['native_custom_slot']=='65536','rig custody'
    assert rig['attributes']['native_place']=='4' and rig['condition']==10000,'rig native place/repair'
    assert len(rig['mounts'])==2,'rig registration lost mounts'
    for key,slot in [('pouch',65537),('right',65538)]:
        pouch=nodes[items[key]]
        assert pouch['parent']==items['rig'] and pouch['location']==2 and pouch['mounted_slot']==slot,'mounted custody'
        assert pouch['attributes']['native_place']=='4','pouch native placement'
    assert nodes[items['bandage']]['parent']==items['pouch'],'restored pouch contents'
    result['custom_items']=items
    return nodes,result


def main():
    ap=argparse.ArgumentParser();ap.add_argument('initial_log',type=Path);ap.add_argument('initial_export',type=Path)
    ap.add_argument('reload_log',type=Path);ap.add_argument('reload_export',type=Path);ap.add_argument('--output',type=Path,required=True)
    a=ap.parse_args();first,r1=check(a.initial_log,a.initial_export);second,r2=check(a.reload_log,a.reload_export)
    assert r1['custom_items']==r2['custom_items'],'persistent custom identities changed'
    # Native live IDs may be remapped on a restart. Every other saved member
    # field, including conditions, rounds, charges, upgrades and bindings, agrees.
    for collection in [first,second]:
        for node in collection.values():node['attributes'].pop('native_live_id',None)
    assert first==second,'restart changed logical/native instance state'
    result=dict(status='passed',initial=r1,reload=r2,limits='Native adapter and persistence fixture; physical drag/context/picker gestures require separate UI observation.')
    a.output.write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result))


if __name__=='__main__':main()
