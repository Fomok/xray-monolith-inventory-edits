"""Validate captured real-game replacement fixture exports across a fresh process."""
import argparse,json
from pathlib import Path
from verify_native_placement import read

def main():
    p=argparse.ArgumentParser()
    p.add_argument('edited',type=Path);p.add_argument('reloaded',type=Path)
    p.add_argument('--output',type=Path,required=True)
    a=p.parse_args();first,h1=read(a.edited);second,h2=read(a.reloaded)
    assert first==second,'restart changed identity, native state, mass, or placement'
    sections={n['attributes']['native_section']:n for n in first.values()}
    assert len(first)==len(sections)==5,'lost or duplicate fixture item'
    assert set(sections)=={'device_pda_1','bandage','medkit','wpn_pm','bread'}
    assert first[100]['position']==[4,3,0],'PDA placement lost'
    assert sections['wpn_pm']['location']==1 and sections['wpn_pm']['equipped_slots']==4,'pistol not in core equipment slot2'
    assert sections['wpn_pm']['attributes']['native_place']=='1' and sections['wpn_pm']['attributes']['native_slot']=='2','native pistol slot mismatch'
    assert all(n['parent']==1 and n['condition']==10000 for n in first.values())
    assert all(n['location']==0 for s,n in sections.items() if s!='wpn_pm'),'unexpected overflow or equipment'
    assert len({n['attributes']['native_live_id'] for n in first.values()})==5,'duplicate native identity'
    report={'status':'passed','count':len(first),'grams':sum(n['grams'] for n in first.values()),'edited_sha256':h1,'reloaded_sha256':h2,'items':second,'limits':'Requires matching runtime logs for real equip/loot transitions. Does not prove physical mouse dragging, right-click actions, authored capacity, or full gameplay parity.'}
    a.output.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
if __name__=='__main__':main()
