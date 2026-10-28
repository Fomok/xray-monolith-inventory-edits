from pathlib import Path
from PIL import Image, ImageChops
import json, hashlib, xml.etree.ElementTree as ET
R=Path(__file__).resolve().parent
m=json.loads((R/'asset-manifest.json').read_text())
ids=set(); count=0
for f in ET.parse(R/'package/gamedata/configs/ui/textures_descr/sqa3_visual_assets.xml').getroot():
    name=f.attrib['name'].split('\\')[-1]
    dds=Image.open(R/f'package/gamedata/textures/ui/{name}.dds').convert('RGBA')
    png=Image.open(R/f'{name}.png').convert('RGBA')
    assert dds.size==png.size and dds.tobytes()==png.tobytes()
    for t in f:
        a=t.attrib; assert a['id'] not in ids;ids.add(a['id'])
        x,y,w,h=[int(a[k]) for k in ('x','y','width','height')]
        assert x>=2 and y>=2 and w>0 and h>0 and x+w<=dds.width-2 and y+h<=dds.height-2
        assert dds.crop((x,y,x+w,y+h)).getextrema()[3]==(255,255)
        count+=1
for o in m['outputs']:assert hashlib.sha256((R/o['path']).read_bytes()).hexdigest()==o['sha256']
for d in ['left','right','up','down']:
    regions=[]
    for s in ['e','h','t','d']:
        a=m['textures']['sqa3v1_arrow_'+d+'_'+s];x,y,w,h=a['xywh']
        im=Image.open(R/(a['atlas']+'.png'));regions.append(im.crop((x,y,x+w,y+h)).tobytes())
    assert len(set(regions))==4
a=m['textures']['sqa3v1_grid_cell'];x,y,w,h=a['xywh'];im=Image.open(R/(a['atlas']+'.png')).crop((x,y,x+w,y+h))
assert len(im.getcolors())==2 and im.getpixel((32,32))==im.getpixel((63,63))
# Independent base assets do not overlap. Nine-slice aliases intentionally share pixels.
vals=list(m['textures'].values())
for i,a in enumerate(vals):
    x,y,w,h=a['xywh']
    for b in vals[i+1:]:
        if a['atlas']!=b['atlas']:continue
        xx,yy,ww,hh=b['xywh'];assert x+w<=xx or xx+ww<=x or y+h<=yy or yy+hh<=y
result={'status':'pass','descriptor_ids':count,'base_assets':len(vals),'checks':['DDS/PNG exact pixel equality','descriptor bounds and unique IDs','opaque crop interiors','base asset non-overlap','four distinct button states','two-color unframed grid','payload SHA256'],'runtime':'not_run'}
(R/'validation.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result))
