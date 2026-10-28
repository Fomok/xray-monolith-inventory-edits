"""Compile audited source geometry into native SQA3 storage definitions.

Outputs development files only. Growth stages are explicit provisional balancing,
not recovered source facts. Released interior coordinates remain unchanged.
"""
import argparse, hashlib, json, math
from pathlib import Path

def rectangles(rects, width, height):
    cells=[0]*(width*height)
    for region,(x,y,w,h) in enumerate(rects,1):
        assert x>=0 and y>=0 and w>0 and h>0 and x+w<=width and y+h<=height
        for yy in range(y,y+h):
            for xx in range(x,x+w):
                assert not cells[yy*width+xx], 'overlapping authored compartments'
                cells[yy*width+xx]=region
    assert any(cells)
    return cells

def pouch_layout(grants):
    width=max(max(w for w,h in grants),min(8,max(3,int(math.sqrt(sum(w*h for w,h in grants)*2)+0.5))))
    skyline=[0]*width;rects=[]
    for w,h in grants:
        y,x=min((max(skyline[x:x+w]),x) for x in range(width-w+1))
        assert y+h<=64
        rects.append((x,y,w,h));skyline[x:x+w]=[y+h]*w
    height=max(y+h for x,y,w,h in rects)
    occupied_width=max(x+w for x,y,w,h in rects)
    return occupied_width,height,rectangles(rects,occupied_width,height)

def compile_catalog(source):
    assert source['schema']=='sqa3.catalog.source.v1'
    definitions={}
    rules={item['section']:{key:value['value'] for key,value in item['fields'].items()}
           for item in source['auxiliary_sections']}
    for item in source['items']:
        section=item['section'];assert section not in definitions and len(section)<=63
        family=item['family'];fields=item['effective_fields']
        value=lambda key,default=None:fields.get(key,{}).get('value',default)
        interior=item.get('interior') or {};w,h=interior.get('width'),interior.get('height')
        cells=interior.get('cells')
        if family=='rig' and w is None:
            grants=[]
            for key,size in [('amp_slot_2x2',(2,2)),('amp_slot_3x1',(1,3)),('amp_slot_2x1',(1,2)),('amp_slot_1x1',(1,1))]:
                count=int(value(key,'0'));assert 0<=count<=16
                grants += [size]*count
            assert grants,'rig has no authored compartments or source fallback counts: '+section
            w,h,cells=pouch_layout(grants)
        elif family=='box':
            w,h=int(value('amp_box_w')),int(value('amp_box_h'))
            rects=[]
            if value('amp_box_slots'):
                for raw in value('amp_box_slots').split('|'):
                    x,y,ww,hh=map(int,raw.strip().split(','));rects.append((x-1,y-1,ww,hh))
            else:rects=[(0,0,w,h)]
            cells=rectangles(rects,w,h)
        elif family=='pouch':
            grants=source['pouch_to_mount_compatibility']['base_grants'].get(section)
            if grants:grants=[(g['width'],g['height']) for g in grants]
            else:
                grants=[]
                for key,size in source['pouch_to_mount_compatibility']['configured_grants'].items():
                    grants += [tuple(size)]*int(value(key,'0'))
            assert grants, 'unresolved pouch grants: '+section
            w,h,cells=pouch_layout(grants)
        assert isinstance(w,int) and isinstance(h,int) and 0<w<=64 and 0<h<=64
        assert cells and len(cells)==w*h and all(isinstance(c,int) and 0<=c<=4096 for c in cells)
        ew,eh=item['exterior']['width'],item['exterior']['height']
        assert ew and eh and float(ew).is_integer() and float(eh).is_integer()
        ew,eh=int(ew),int(eh);assert 0<ew<=64 and 0<eh<=64
        growing=family in ('backpack','rig')
        empty=(max(1,ew-1),max(1,eh-1)) if growing else (ew,eh)
        half=(ew,eh);full=(ew,min(64,eh+1)) if growing else half
        definitions[section]={'family':family,'width':w,'height':h,'cells':cells,
            'empty':empty,'half':half,'full':full,'growing':growing,
            'max_contents_grams':round(float(value('amp_box_kg','0'))*1000),
            'stack_limit':int(value('amp_box_stack','0')),
            'allow_container_items':False,'exterior_independent':True,
            'tear_thresholds':[8000,6000,4000,2500,1000] if family=='rig' else [],
            'growth_provenance':'provisional SQA3 test balancing; configurable' if growing else 'fixed source exterior',
            'source_section':section}
        if family=='box' and value('amp_box_takes','anything')!='anything':
            rule=rules['amp_box_'+value('amp_box_takes')]
            tokens=lambda raw:'|'.join(part.strip() for part in raw.split(',') if part.strip())
            definitions[section]['acceptance']={
                'accept_kinds':tokens(rule.get('kinds','')),
                'accept_sections':tokens(rule.get('also','')),
                'reject_sections':tokens(rule.get('never','')),
                'accept_flags':'|'.join(flag for key,flag in [('ammo','ammo'),('guns','weapon'),('mags','magazine'),('rigs','rig')] if rule.get(key)=='true')}
    return definitions

def main():
    parser=argparse.ArgumentParser();parser.add_argument('catalog',type=Path);parser.add_argument('--output',type=Path,required=True)
    parser.add_argument('--items-script',type=Path)
    args=parser.parse_args();raw=args.catalog.read_bytes();source=json.loads(raw);definitions=compile_catalog(source)
    lines=['; SQA3 native catalog, schema1. Generated from audited source data.',
        '; Growth stages are provisional test balancing; edit empty/half/full per definition.',
        '; Dimensions use ROWS x COLUMNS. Mask rows use | and cell IDs use commas.',
        '[sqa3_catalog_metadata]','schema_version = 1','',
        '[sqa3_storage_definitions]']
    lines += [f'{name} = sqa3_storage_{name}' for name in definitions]
    for name,d in definitions.items():
        w,h=d['width'],d['height']
        lines += ['',f'[sqa3_storage_{name}]',f'family = {d["family"]}',f'size = {h}x{w}',
            'mask = '+'|'.join(','.join(str(c) for c in d['cells'][y*w:(y+1)*w]) for y in range(h)),
            'allow_container_items = false','exterior_independent = true',f'growing = {str(d["growing"]).lower()}']
        for key in ['empty','half','full']:
            width,height=d[key];lines.append(f'{key} = {height}x{width}')
        lines += [f'max_contents_grams = {d["max_contents_grams"]}',f'stack_limit = {d["stack_limit"]}']
        if d['family']=='rig':
            lines += ['; Released rig tear thresholds, basis points; largest pockets first.',
                      'tear_thresholds = 8000,6000,4000,2500,1000']
        for key,value in d.get('acceptance',{}).items():lines.append(f'{key} = {value}'.rstrip())
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text('\n'.join(lines)+'\n',encoding='utf-8')
    report={'schema':'sqa3.native.catalog.v1','source_sha256':hashlib.sha256(raw).hexdigest(),'definitions':definitions}
    args.output.with_suffix('.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    if args.items_script:
        args.items_script.parent.mkdir(parents=True,exist_ok=True)
        args.items_script.write_text('-- Generated catalog IDs; no automatic spawn.\nsections = {\n'+''.join('    '+json.dumps(name)+',\n' for name in definitions)+'}\n',encoding='utf-8')
    print(json.dumps({'definitions':len(definitions),'output':str(args.output)}))

if __name__=='__main__':main()
