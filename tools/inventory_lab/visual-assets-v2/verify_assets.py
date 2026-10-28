from pathlib import Path
from PIL import Image
import json,hashlib,xml.etree.ElementTree as ET
R=Path(__file__).resolve().parent
m=json.loads((R/'asset-manifest.json').read_text())
dds=Image.open(R/'package/gamedata/textures/ui/sqa3_visual_info_v2.dds').convert('RGBA')
png=Image.open(R/'atlas.png').convert('RGBA')
assert dds.size==(1024,1024) and dds.tobytes()==png.tobytes()
ids=set();rects=[];parts={}
for t in ET.parse(R/'package/gamedata/configs/ui/textures_descr/sqa3_visual_assets_v2.xml').getroot().iter('texture'):
    a=t.attrib;k=a['id'];assert k not in ids;ids.add(k)
    x,y,w,h=[int(a[v]) for v in ('x','y','width','height')]
    assert x>=2 and y>=2 and x+w<=1022 and y+h<=1022
    for xx,yy,ww,hh in rects:assert x+w<=xx or xx+ww<=x or y+h<=yy or yy+hh<=y
    rects.append((x,y,w,h));parts[k]=dds.crop((x,y,x+w,y+h))
assert len(ids)==9
assert len({parts['sqa3v2_close_'+s].tobytes() for s in 'ehtd'})==4
frame=parts['sqa3v2_portrait_frame']
assert frame.getpixel((92,88))[3]==0 and frame.getpixel((2,2))[3]==255
assert parts['sqa3v2_info_panel'].size==(724,188)
for o in m['outputs']:assert hashlib.sha256((R/o['path']).read_bytes()).hexdigest()==o['sha256']
v1=R.parent/'visual-assets-v1';old=json.loads((v1/'asset-manifest.json').read_text())
for o in old['outputs']:assert hashlib.sha256((v1/o['path']).read_bytes()).hexdigest()==o['sha256']
old_ids={t.attrib['id'] for t in ET.parse(v1/'package/gamedata/configs/ui/textures_descr/sqa3_visual_assets.xml').getroot().iter('texture')}
assert not ids.intersection(old_ids)
result={'status':'pass','texture_ids':9,'checks':['DDS exact pixel roundtrip','unique IDs, bounds and nonoverlap','all four close states distinct','transparent portrait overlay aperture','composite dimensions','payload hashes','v1 payload hashes unchanged','no v1 ID collisions'],'native_runtime':'not_run'}
(R/'validation.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result))
