"""Deterministic released-material composition. No screenshots supply pixels."""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont, ImageEnhance
import hashlib, json, xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parent
SOURCE = Path('C:/GAMMA/sqa-previews/release-2.1.0-hotfix1-full/source/gamedata')
OUT = ROOT / 'package/gamedata'
(OUT / 'textures/ui').mkdir(parents=True, exist_ok=True)
(OUT / 'configs/ui/textures_descr').mkdir(parents=True, exist_ok=True)
panels = Image.open(SOURCE / 'textures/ui/sqa_fk_panels.dds').convert('RGBA')
controls = Image.open(SOURCE / 'textures/ui/sqa_fk_controls.dds').convert('RGBA')
sources = []
def crop(im, name, rect):
    sources.append({'material': name, 'source': 'sqa_fk_panels.dds' if im is panels else 'sqa_fk_controls.dds', 'xywh': rect})
    x,y,w,h = rect
    return im.crop((x,y,x+w,y+h))
lining = crop(panels,'dark lining',(8,1700,256,128))
olive = crop(panels,'olive header',(280,1700,256,128))
rim = crop(controls,'outer metal window',(8,8,128,128))
recess = crop(panels,'equipment recess',(40,26,200,192))
def tile(im, size):
    out=Image.new('RGBA',size)
    for y in range(0,size[1],im.height):
        for x in range(0,size[0],im.width): out.paste(im,(x,y))
    return out
def frame(src,size,b,fill=None):
    w,h=size; sw,sh=src.size
    out=tile(fill,size) if fill else Image.new('RGBA',size)
    xs=[0,b,sw-b,sw]; ys=[0,b,sh-b,sh]
    xd=[0,b,w-b,w]; yd=[0,b,h-b,h]
    for j in range(3):
        for i in range(3):
            if i==1 and j==1 and fill is not None: continue
            part=src.crop((xs[i],ys[j],xs[i+1],ys[j+1]))
            out.paste(part.resize((xd[i+1]-xd[i],yd[j+1]-yd[j]),Image.Resampling.LANCZOS),(xd[i],yd[j]))
    return out

assets={}; specs={}
def add(name,im,usage,logical=None):
    key='sqa3v1_'+name; assets[key]=im
    specs[key]={'usage':usage,'logical_size':logical or [im.width/2,im.height/2]}
    return im
eq=add('equipment_panel',frame(rim,(521,1536),12,lining),'Full column; fixed outer rim and tiled lining, no baked slots.',[260.5,768])
inv=add('storage_panel',frame(rim,(764,1536),12,lining),'Storage or conditional nearby column; one complete panel, never a cell.',[382,768])
add('surface',lining,'Tile or use a precomposed panel; never stretch over a full column.',[128,64])
add('panel_frame',rim,'Nine-slice outer frame; center replaced with tiled dark lining.',[64,64])
add('slot_frame',recess,'Nine-slice equipment/quickslot frame only; never use for grid cells.',[100,96])
for name,size in [('helmet',(150,192)),('chest',(150,300)),('small',(150,84)),('weapon',(474,192)),('quick',(69,92))]:
    add('slot_'+name,frame(recess,size,6),'Fixed equipment recess; native icons inset and aspect-preserved.')
cell=Image.new('RGBA',(64,64),(19,20,20,255)); d=ImageDraw.Draw(cell)
d.line((0,0,63,0),fill=(48,49,47,255),width=2);d.line((0,0,0,63),fill=(48,49,47,255),width=2)
add('grid_cell',cell,'Repeat at actual cell pitch; top/left thin lines only. Add bottom/right outer edges once.',[32,32])
add('grid_fill',Image.new('RGBA',(8,8),(19,20,20,255)),'Uniform grid interior; safe to stretch.')
add('grid_line',Image.new('RGBA',(2,2),(48,49,47,255)),'Single logical-pixel grid lines. No metal, bevel or rivets.',[1,1])
header=add('section_bar',frame(recess,(708,44),6,olive),'Olive tiled center, metal recess rim; names/weight/collapse are separate live controls.',[354,22])
add('identity_bar',frame(recess,(708,112),6,olive),'Optional player/source identity header.',[354,56])

# Polygon chevrons are raster artwork, not font glyphs. Same material at every state.
for direction in ['left','right','up','down']:
    for state,mult in [('e',1.0),('h',1.35),('t',0.78),('d',0.52)]:
        size=(28,40) if direction in ['left','right'] else (36,36)
        bg=frame(recess,size,6,ImageEnhance.Brightness(olive).enhance(.60))
        bg=ImageEnhance.Brightness(bg).enhance(mult)
        scale=4; mask=Image.new('L',(size[0]*scale,size[1]*scale),0)
        points=[(-3,-7),(3,0),(-3,7),(-5,5),(0,0),(-5,-5)]
        def transform(p):
            x,y=p
            if direction=='left': x=-x
            elif direction=='up': x,y=y,-x
            elif direction=='down': x,y=y,x
            return (int((size[0]/2+x)*scale),int((size[1]/2+y)*scale))
        ImageDraw.Draw(mask).polygon([transform(p) for p in points],fill=255)
        mask=mask.resize(size,Image.Resampling.LANCZOS)
        shadow=Image.new('RGBA',size,(0,0,0,255)); shifted=Image.new('L',size);shifted.paste(mask,(1,2));bg.paste(shadow,(0,0),shifted)
        ink=tuple(int(v*(.48 if state=='d' else 1)) for v in (218,219,203))+(255,)
        bg.paste(Image.new('RGBA',size,ink),(0,0),mask)
        add('arrow_'+direction+'_'+state,bg,'Textured '+direction+' button: '+dict(e='enabled',h='hover',t='pressed',d='disabled')[state]+'. Keep text empty.')

manifest={'version':1,'raster_scale':2,'source_root':str(SOURCE),'source_crops':sources,'sources':[], 'textures':{}, 'outputs':[]}
for p in [SOURCE/'textures/ui/sqa_fk_panels.dds',SOURCE/'textures/ui/sqa_fk_controls.dds',SOURCE/'configs/ui/textures_descr/sqa_field_kit.xml',SOURCE/'configs/ui/ui_inventory_16.xml']:
    manifest['sources'].append({'path':str(p),'sha256':hashlib.sha256(p.read_bytes()).hexdigest()})
root=ET.Element('w')
for atlas_name,names,size in [
    ('sqa3_visual_panels',[k for k in assets if k.endswith('_panel')],(2048,2048)),
    ('sqa3_visual_controls',[k for k in assets if not k.endswith('_panel')],(1024,1024))]:
    atlas=Image.new('RGBA',size); file=ET.SubElement(root,'file',name='ui\\'+atlas_name)
    x=y=4; row=0
    for key in sorted(names,key=lambda k:(-assets[k].height,k)):
        im=assets[key]
        if x+im.width+4>size[0]: x=4;y+=row+8;row=0
        assert y+im.height+4<=size[1],key
        # Two-pixel edge extrusion protects UV sampling; four pixels remain between assets.
        atlas.paste(im,(x,y))
        atlas.paste(im.crop((0,0,im.width,1)).resize((im.width,2)),(x,y-2))
        atlas.paste(im.crop((0,im.height-1,im.width,im.height)).resize((im.width,2)),(x,y+im.height))
        atlas.paste(im.crop((0,0,1,im.height)).resize((2,im.height)),(x-2,y))
        atlas.paste(im.crop((im.width-1,0,im.width,im.height)).resize((2,im.height)),(x+im.width,y))
        ET.SubElement(file,'texture',id=key,x=str(x),y=str(y),width=str(im.width),height=str(im.height))
        manifest['textures'][key]={**specs[key],'atlas':atlas_name,'xywh':[x,y,im.width,im.height]}
        if key in ('sqa3v1_panel_frame','sqa3v1_slot_frame','sqa3v1_section_bar'):
            b=12 if key.endswith('panel_frame') else 6
            for suffix,ix,iy,iw,ih in [('lt',0,0,b,b),('t',b,0,im.width-2*b,b),('rt',im.width-b,0,b,b),('l',0,b,b,im.height-2*b),('back',b,b,im.width-2*b,im.height-2*b),('r',im.width-b,b,b,im.height-2*b),('lb',0,im.height-b,b,b),('b',b,im.height-b,im.width-2*b,b),('rb',im.width-b,im.height-b,b,b)]:
                ET.SubElement(file,'texture',id=key+'_'+suffix,x=str(x+ix),y=str(y+iy),width=str(iw),height=str(ih))
        x+=im.width+8;row=max(row,im.height)
    atlas.save(ROOT/(atlas_name+'.png'))
    atlas.save(OUT/('textures/ui/'+atlas_name+'.dds')) # uncompressed RGBA, no mipmaps
ET.indent(root)
ET.ElementTree(root).write(OUT/'configs/ui/textures_descr/sqa3_visual_assets.xml',encoding='utf-8',xml_declaration=True)

font_path='C:/Windows/Fonts/segoeui.ttf'
def font(n): return ImageFont.truetype(font_path,n)
sheet=Image.new('RGB',(1200,860),(27,29,30)); draw=ImageDraw.Draw(sheet)
draw.text((24,16),'SQA3 / FIELD KIT MATERIALS — source-derived asset review',font=font(22),fill='#ddd9c9')
draw.text((24,50),'Offline composition. No game rendering or gameplay acceptance implied.',font=font(15),fill='#9caa99')
for im,pos,label in [(eq.resize((260,768)),(24,82),'Equipment shell'),(inv.resize((382,768)),(300,82),'Storage / nearby shell')]:
    sheet.paste(im,pos,im);draw.text((pos[0]+16,pos[1]+16),label,font=font(15),fill='#aaa997')
sx=710
sheet.paste(header.resize((354,22)),(sx,92));draw.text((sx,65),'Olive separator / live text region',font=font(16),fill='#d6d2bd')
grid=Image.new('RGBA',(384,256))
for y in range(4):
    for x in range(6):grid.paste(cell,(x*64,y*64))
ImageDraw.Draw(grid).line((383,0,383,255,0,255),fill=(48,49,47),width=2)
sheet.paste(grid,(sx,150));draw.text((sx,124),'Thin grid — no cell frames',font=font(16),fill='#d6d2bd')
for i,dr in enumerate(['left','right','up','down']):
    draw.text((sx+i*110,440),dr,font=font(16),fill='#d6d2bd')
    for j,st in enumerate(['e','h','t','d']):
        button=assets['sqa3v1_arrow_'+dr+'_'+st]
        button=button.resize((button.width//2,button.height//2),Image.Resampling.LANCZOS)
        sheet.paste(button,(sx+i*110+j*24,470))
draw.text((sx,524),'Quickslot recesses / labels remain live',font=font(16),fill='#d6d2bd')
for i in range(6):
    sheet.paste(assets['sqa3v1_slot_quick'],(sx+i*76,553))
    draw.text((sx+i*76+5,558),'F'+str(i+1) if i<4 else ['G','B'][i-4],font=font(14),fill='#ddd9c9')
draw.text((sx,685),'Arrow states: enabled / hover / pressed / disabled',font=font(15),fill='#aaa997')
draw.text((sx,720),'Raster source: 2 px per logical UI unit\nPanel rims: 6 units; slot rims: 3 units\nGrid: 1-unit line at 32-unit reference pitch',font=font(16),fill='#aaa997',spacing=6)
sheet.save(ROOT/'contact-sheet.png')

# Empty layout proof at 2x logical canvas. Item slots are deliberately not populated.
proof=Image.new('RGBA',(2048,1536),(9,10,10,255))
proof.paste(eq,(0,0));proof.paste(inv,(521,0));proof.paste(inv.resize((763,1536)),(1285,0))
layout=[]
def place(key,x,y,w=None,h=None,label=None):
    im=assets['sqa3v1_'+key];w=w or im.width/2;h=h or im.height/2
    im=im.resize((round(w*2),round(h*2)),Image.Resampling.LANCZOS);proof.paste(im,(round(x*2),round(y*2)),im)
    layout.append({'texture':'sqa3v1_'+key,'x':x,'y':y,'width':w,'height':h})
    if label: ImageDraw.Draw(proof).text((x*2+8,y*2+h*2-28),label,font=font(18),fill='#a7aa9c')
for key,x,y,label in [('helmet',12,9,'Helmet'),('helmet',12,117,'Backpack'),('chest',93,9,'Armour / rig'),('small',174,9,'Torch'),('small',174,63,'PDA'),('small',174,117,'Detector'),('small',93,171,'Knife / binoculars'),('small',174,171,'Pistol (visual)'),('weapon',12,223,'Weapon 1'),('weapon',12,325,'Weapon 2')]:place('slot_'+key,x,y,label=label)
for i,x in enumerate([12,52.5,93,133.5,174,214.5]):place('slot_quick',x,433,label='F'+str(i+1) if i<4 else ['G','B'][i-4])
place('arrow_left_e',86,74);place('arrow_right_e',161,74)
for base in [260.5,642.5]:
    place('identity_bar',base+14,12)
    for y,title in [(112,'Rig' if base<600 else 'Nearby source'),(306,'Backpack' if base<600 else 'Selected source')]:
        place('section_bar',base+14,y);place('arrow_down_e',base+17,y+2)
        dr=ImageDraw.Draw(proof);dr.text(((base+191)*2,(y+4)*2),title,font=font(20),fill='#d1d1bf',anchor='mt')
    for gy,rows,cols in [(144,4,6),(338,10,8)]:
        gx=base+191-cols*16
        for row in range(rows):
            for col in range(cols):place('grid_cell',gx+col*32,gy+row*32)
        dr=ImageDraw.Draw(proof);dr.line(((gx+cols*32)*2,gy*2,(gx+cols*32)*2,(gy+rows*32)*2,gx*2,(gy+rows*32)*2),fill=(48,49,47),width=2)
ImageDraw.Draw(proof).text((560,1450),'OFFLINE EMPTY LAYOUT PROOF — weights/items/scrollbars supplied by Engine',font=font(21),fill='#a9a896')
proof.convert('RGB').save(ROOT/'layout-proof.png')
(ROOT/'layout-reference.json').write_text(json.dumps({'canvas':[1024,768],'scale':2,'note':'Illustrative geometry contract; not native XML replacement. Nearby column conditional. No equipment mutation.','placements':layout},indent=2)+'\n')
for p in sorted(OUT.rglob('*')):
    if p.is_file(): manifest['outputs'].append({'path':str(p.relative_to(ROOT)),'bytes':p.stat().st_size,'sha256':hashlib.sha256(p.read_bytes()).hexdigest()})
(ROOT/'asset-manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
print('Built',len(assets),'assets;',len(list(root.iter('texture'))),'texture IDs; XML + 2 DDS atlases')
