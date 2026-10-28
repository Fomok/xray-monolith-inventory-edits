"""Additive SQA3 header/close artwork; never writes v1 or Engine files."""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont, ImageEnhance
import json, hashlib, xml.etree.ElementTree as ET

R=Path(__file__).resolve().parent
S=Path('C:/GAMMA/sqa-previews/release-2.1.0-hotfix1-full/source/gamedata')
O=R/'package/gamedata'
(O/'textures/ui').mkdir(parents=True,exist_ok=True)
(O/'configs/ui/textures_descr').mkdir(parents=True,exist_ok=True)
p=Image.open(S/'textures/ui/sqa_fk_panels.dds').convert('RGBA')
c=Image.open(S/'textures/ui/sqa_fk_controls.dds').convert('RGBA')
crops=[]
def crop(im,name,rect,size):
    x,y,w,h=rect
    crops.append({'name':name,'source':'sqa_fk_panels.dds' if im is p else 'sqa_fk_controls.dds','xywh':rect,'output_pixels':size,'resampling':'LANCZOS'})
    return im.crop((x,y,x+w,y+h)).resize(size,Image.Resampling.LANCZOS)
identity=crop(p,'public olive identity',(740,30,704,124),(528,124))
money=crop(p,'public dark info bar',(740,160,704,56),(528,56))
portrait=crop(p,'public portrait inset',(1470,50,224,150),(184,176))
# A thin warm inner rim matches the reference portrait overlay. This is authored
# line art; it does not include a faction emblem, rank or portrait photograph.
pd=ImageDraw.Draw(portrait)
pd.rectangle((2,2,181,173),outline=(122,117,65,255),width=2)
overlay=portrait.copy();ImageDraw.Draw(overlay).rectangle((4,4,179,171),fill=(0,0,0,0))
info=Image.new('RGBA',(724,188))
info.paste(identity,(0,0));info.paste(money,(0,132));info.paste(portrait,(536,12))
assets={
 'sqa3v2_info_panel':info,
 'sqa3v2_identity_region':identity,
 'sqa3v2_money_bar':money,
 'sqa3v2_portrait_recess':portrait,
 'sqa3v2_portrait_frame':overlay,
}
base=crop(c,'public close button base',(600,160,48,48),(40,40))
for state,mult in [('e',1.0),('h',1.35),('t',.72),('d',.48)]:
    bg=ImageEnhance.Brightness(base).enhance(mult)
    mask=Image.new('L',(160,160));d=ImageDraw.Draw(mask)
    offset=1 if state=='t' else 0
    # Draw diagonal strokes as actual antialiased texture pixels, never a glyph.
    for a,b in [((12+offset,12+offset),(27+offset,27+offset)),((27+offset,12+offset),(12+offset,27+offset))]:
        d.line(tuple(v*4 for v in a+b),fill=255,width=12)
    mask=mask.resize((40,40),Image.Resampling.LANCZOS)
    shadow=Image.new('L',(40,40));shadow.paste(mask,(1,2))
    bg.paste((0,0,0,255),(0,0,40,40),shadow)
    color=(108,109,100,255) if state=='d' else (226,225,204,255)
    bg.paste(color,(0,0,40,40),mask)
    assets['sqa3v2_close_'+state]=bg

atlas=Image.new('RGBA',(1024,1024));root=ET.Element('w');f=ET.SubElement(root,'file',name='ui\\sqa3_visual_info_v2')
manifest={'version':2,'additive':True,'raster_pixels_per_ui_unit':2,'source_crops':crops,'sources':[],'textures':{},'outputs':[]}
for src in [S/'textures/ui/sqa_fk_panels.dds',S/'textures/ui/sqa_fk_controls.dds',S/'configs/ui/ui_inventory_16.xml']:
    manifest['sources'].append({'path':str(src),'sha256':hashlib.sha256(src.read_bytes()).hexdigest()})
x=y=4;rh=0
for key,im in assets.items():
    if x+im.width+4>1024:x=4;y+=rh+8;rh=0
    assert y+im.height+4<=1024
    atlas.paste(im,(x,y))
    atlas.paste(im.crop((0,0,im.width,1)).resize((im.width,2)),(x,y-2))
    atlas.paste(im.crop((0,im.height-1,im.width,im.height)).resize((im.width,2)),(x,y+im.height))
    atlas.paste(im.crop((0,0,1,im.height)).resize((2,im.height)),(x-2,y))
    atlas.paste(im.crop((im.width-1,0,im.width,im.height)).resize((2,im.height)),(x+im.width,y))
    ET.SubElement(f,'texture',id=key,x=str(x),y=str(y),width=str(im.width),height=str(im.height))
    manifest['textures'][key]={'xywh':[x,y,im.width,im.height],'logical_size':[im.width/2,im.height/2]}
    x+=im.width+8;rh=max(rh,im.height)
atlas.save(R/'atlas.png');atlas.save(O/'textures/ui/sqa3_visual_info_v2.dds')
ET.indent(root);ET.ElementTree(root).write(O/'configs/ui/textures_descr/sqa3_visual_assets_v2.xml',encoding='utf-8',xml_declaration=True)

layout={'canvas':[1024,768],'group':{'relative_to':'column origin','x':10,'y':10,'width':362,'height':94,'texture':'sqa3v2_info_panel'},
 'regions_relative_to_group':{
 'identity':{'x':0,'y':0,'width':264,'height':62},
 'money':{'x':0,'y':66,'width':264,'height':28},
 'portrait_frame':{'x':268,'y':6,'width':92,'height':88},
 'portrait_image':{'x':270,'y':8,'width':88,'height':84},
 'name':{'x':8,'y':14,'width':158,'height':18},
 'faction':{'x':8,'y':34,'width':158,'height':18},
 'rank_icon':{'x':176,'y':6,'width':38,'height':50},
 'emblem':{'x':216,'y':6,'width':42,'height':50},
 'money_text':{'x':8,'y':70,'width':246,'height':20,'align':'right'}},
 'close':{'texture_base':'sqa3v2_close','width':20,'height':20,'right_inset':4,'top_inset':4,'states':['e','h','t','d']},
 'semantics':'Engine supplies actual identities; non-character sources hide rank/faction/portrait/money or use genuine source data. No collapse button in separate windows.'}
(R/'layout-contract.json').write_text(json.dumps(layout,indent=2)+'\n')

font=lambda n:ImageFont.truetype('C:/Windows/Fonts/segoeui.ttf',n)
sheet=Image.new('RGB',(1100,690),(25,27,27));d=ImageDraw.Draw(sheet)
d.text((24,18),'SQA3 / v2 top information artwork + textured close',font=font(24),fill='#dfddce')
d.text((24,54),'Additive assets; no baked identity, faction, rank, portrait or money.',font=font(16),fill='#a4ac9d')
sheet.paste(info,(24,100),info)
d.text((780,108),'362 x 94 UI units',font=font(19),fill='#dfddce')
d.text((780,142),'Shown at 2x raster size\nOlive region: 264 x 62\nInfo bar: 264 x 28\nPortrait: 92 x 88',font=font(16),fill='#a4ac9d',spacing=8)
d.text((24,314),'Layout zones only — Engine draws real values/icons',font=font(17),fill='#dfddce')
demo=info.copy();dd=ImageDraw.Draw(demo)
dd.text((16,26),'Name',font=font(22),fill='#deded1');dd.text((16,66),'Faction / source label',font=font(20),fill='#d3cbb0')
for box,label in [((352,12,428,112),'Rank'),((432,12,516,112),'Emblem'),((540,16,716,184),'Portrait')]:
    dd.text(((box[0]+box[2])/2,(box[1]+box[3])/2),label,font=font(16),fill='#c2c3ad',anchor='mm')
dd.text((500,150),'Money / source info',font=font(19),fill='#deded1',anchor='ra')
sheet.paste(demo,(24,350),demo)
d.text((780,350),'Window close states',font=font(19),fill='#dfddce')
for i,st in enumerate(['e','h','t','d']):
    im=assets['sqa3v2_close_'+st];sheet.paste(im,(780+i*72,394),im)
    d.text((780+i*72,444),st,font=font(18),fill='#c7c8b4')
d.text((780,480),'e normal / h hover\nt pressed / d disabled\n20 x 20 UI units',font=font(16),fill='#a4ac9d',spacing=6)
d.text((24,590),'Source: released Field Kit atlas; reference screenshot supplies appearance only.',font=font(17),fill='#a4ac9d')
d.text((24,624),'Offline asset review. Native integration / live values / input acceptance remain pending.',font=font(17),fill='#a4ac9d')
sheet.save(R/'contact-sheet.png')
for path in sorted(O.rglob('*')):
    if path.is_file():manifest['outputs'].append({'path':str(path.relative_to(R)),'bytes':path.stat().st_size,'sha256':hashlib.sha256(path.read_bytes()).hexdigest()})
(R/'asset-manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
print('Built additive v2:',len(assets),'texture IDs')
