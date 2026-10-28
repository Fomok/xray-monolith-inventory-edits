"""Validated, versioned local SQA3 definitions and reversible preset operations."""
import argparse
import configparser
import json
import os
from pathlib import Path
import re
import tempfile

SCHEMA = 'sqa3.storage.preset.v1'
FAMILIES = {'backpack', 'rig', 'pouch', 'box'}
FIELDS = {'family', 'width', 'height', 'cells', 'empty', 'half', 'full', 'growing',
          'allow_container_items', 'exterior_independent', 'max_contents_grams',
          'stack_limit', 'tear_thresholds', 'acceptance', 'provenance'}
TOKEN = re.compile(r'^[a-z0-9_]+$')


def require(condition, path, message):
    if not condition:
        raise ValueError(f'{path}: {message}')


def integer(value, low, high, path):
    require(type(value) is int and low <= value <= high, path, f'expected integer {low}..{high}')


def validate(document):
    require(document.get('schema') == SCHEMA, 'schema', f'expected {SCHEMA}')
    definitions = document.get('definitions')
    require(isinstance(definitions, dict) and 0 < len(definitions) <= 1024, 'definitions', 'expected 1..1024 sections')
    for section, d in definitions.items():
        path = f'definitions.{section}'
        require(isinstance(section, str) and len(section) <= 63 and TOKEN.fullmatch(section), path, 'invalid native section ID')
        require(isinstance(d, dict), path, 'expected object')
        require(not set(d)-FIELDS, path, 'unknown fields: '+','.join(sorted(set(d)-FIELDS)))
        require(d.get('family') in FAMILIES, path+'.family', 'unsupported family')
        for key in ['width', 'height']:
            integer(d.get(key), 1, 64, path+'.'+key)
        cells = d.get('cells')
        require(isinstance(cells, list) and len(cells) == d['width']*d['height'], path+'.cells', 'mask must match width*height')
        for i, cell in enumerate(cells):
            integer(cell, 0, 4096, path+f'.cells[{i}]')
        require(any(cells), path+'.cells', 'authored mask must have a usable compartment')
        for key in ['growing', 'allow_container_items', 'exterior_independent']:
            require(type(d.get(key)) is bool, path+'.'+key, 'expected boolean')
        require(not d['growing'] or d['family'] in {'backpack','rig'}, path+'.growing', 'only backpacks and rigs may grow')
        for key in ['empty','half','full']:
            shape = d.get(key)
            require(isinstance(shape, list) and len(shape)==2, path+'.'+key, 'expected [width,height]')
            for i, v in enumerate(shape): integer(v,1,64,path+f'.{key}[{i}]')
        if d['growing']:
            require(all(d['empty'][i] <= d['half'][i] <= d['full'][i] for i in (0,1)), path, 'growth dimensions must be monotonic')
            if not d['exterior_independent']:
                require(d['full'][0]*d['full'][1] >= sum(bool(c) for c in cells),path+'.full','exterior smaller than interior without explicit independent policy')
        else:
            require(d['empty']==d['half']==d['full'], path, 'fixed family has differing exterior stages')
        integer(d.get('max_contents_grams'),0,10**12,path+'.max_contents_grams')
        integer(d.get('stack_limit'),0,1000000,path+'.stack_limit')
        thresholds=d.get('tear_thresholds',[])
        require(isinstance(thresholds,list) and len(thresholds)<=64,path+'.tear_thresholds','expected at most 64 thresholds')
        for i,v in enumerate(thresholds): integer(v,0,10000,path+f'.tear_thresholds[{i}]')
        require(not thresholds or d['family']=='rig',path+'.tear_thresholds','only rigs may tear')
        require(all(a>b for a,b in zip(thresholds,thresholds[1:])),path+'.tear_thresholds','thresholds must descend without duplicates')
        if 'acceptance' in d:
            rules=d['acceptance'];keys={'accept_kinds','accept_sections','reject_sections','accept_flags'}
            require(isinstance(rules,dict) and set(rules)==keys,path+'.acceptance','expected all four acceptance fields')
            for key,value in rules.items():
                require(isinstance(value,str) and len(value)<=1024,path+'.acceptance.'+key,'expected bounded token list')
                tokens=value.split('|') if value else []
                require(all(TOKEN.fullmatch(t) for t in tokens) and len(set(tokens))==len(tokens),path+'.acceptance.'+key,'invalid or duplicate token')
                if key=='accept_flags':require(set(tokens)<={'ammo','weapon','magazine','rig'},path+'.acceptance.'+key,'unsupported classifier')
    return document


def migrate(document):
    if document.get('schema') == SCHEMA: return validate(document)
    require(document.get('schema')=='sqa3.native.catalog.v1','schema','unsupported version; SQA2 saves are not authoring presets')
    definitions={}
    for section, original in document['definitions'].items():
        require(not set(original)-(FIELDS|{'source_section','growth_provenance','layout_provenance'}),f'definitions.{section}','unknown legacy definition fields')
        d={k:v for k,v in original.items() if k in FIELDS}
        d.setdefault('allow_container_items',False);d.setdefault('exterior_independent',True)
        d.setdefault('tear_thresholds',[8000,6000,4000,2500,1000] if d.get('family')=='rig' else [])
        d['provenance']={k:original[k] for k in ['source_section','growth_provenance','layout_provenance'] if k in original}
        definitions[section]=d
    return validate(dict(schema=SCHEMA,definitions=definitions))


def render(document):
    validate(document)
    lines=['; SQA3 validated local authoring preset; schema1.',
           '[sqa3_catalog_metadata]','schema_version = 1','','[sqa3_storage_definitions]']
    lines += [f'{name} = sqa3_storage_{name}' for name in document['definitions']]
    for name,d in document['definitions'].items():
        w,h=d['width'],d['height']
        lines += ['',f'[sqa3_storage_{name}]',f'family = {d["family"]}',f'size = {h}x{w}',
                  'mask = '+'|'.join(','.join(map(str,d['cells'][y*w:(y+1)*w])) for y in range(h))]
        for key in ['growing','allow_container_items','exterior_independent']:
            lines.append(f'{key} = {str(d[key]).lower()}')
        for key in ['empty','half','full']:
            width,height=d[key];lines.append(f'{key} = {height}x{width}')
        for key in ['max_contents_grams','stack_limit']:lines.append(f'{key} = {d[key]}')
        if d.get('tear_thresholds'):lines.append('tear_thresholds = '+','.join(map(str,d['tear_thresholds'])))
        for key,value in d.get('acceptance',{}).items():lines.append(f'{key} = {value}'.rstrip())
    return '\n'.join(lines)+'\n'


def parse_ltx(path):
    config=configparser.ConfigParser(interpolation=None,strict=True,delimiters=('=',),comment_prefixes=(';',))
    config.read_string(path.read_text(encoding='utf-8-sig'))
    require(config.getint('sqa3_catalog_metadata','schema_version')==1,'schema_version','unsupported native definition version')
    definitions={}
    for name,target in config['sqa3_storage_definitions'].items():
        s=config[target];h,w=map(int,s['size'].lower().split('x'));d=dict(family=s['family'],width=w,height=h,
            cells=[int(c) for row in s['mask'].split('|') for c in row.split(',')])
        require(len(s['mask'].split('|'))==h and all(len(row.split(','))==w for row in s['mask'].split('|')),name+'.mask','row dimensions mismatch')
        for key in ['empty','half','full']:
            hh,ww=map(int,s[key].lower().split('x'));d[key]=[ww,hh]
        for key in ['growing','allow_container_items','exterior_independent']:d[key]=s.getboolean(key)
        for key in ['max_contents_grams','stack_limit']:d[key]=s.getint(key)
        d['tear_thresholds']=[int(v) for v in s.get('tear_thresholds','').split(',') if v]
        if 'accept_kinds' in s:d['acceptance']={key:s[key] for key in ['accept_kinds','accept_sections','reject_sections','accept_flags']}
        definitions[name]=d
    return validate(dict(schema=SCHEMA,definitions=definitions))


def atomic_write(path,text):
    path.parent.mkdir(parents=True,exist_ok=True)
    temporary=None
    try:
        with tempfile.NamedTemporaryFile('w',encoding='utf-8',dir=path.parent,delete=False) as f:
            temporary=Path(f.name);f.write(text);f.flush();os.fsync(f.fileno())
        os.replace(temporary,path)
    finally:
        if temporary and temporary.exists():temporary.unlink()


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('action',choices=['validate','import','export','reset'])
    ap.add_argument('source',type=Path,help='JSON preset/catalog, or native LTX for export; reset uses the pinned default catalog JSON')
    ap.add_argument('--output',type=Path)
    args=ap.parse_args()
    try:
        document=parse_ltx(args.source) if args.action=='export' else migrate(json.loads(args.source.read_text(encoding='utf-8-sig')))
        if args.action!='validate':
            require(args.output is not None,'output','required for writing; source is never modified')
            require(args.output.resolve()!=args.source.resolve(),'output','must differ from source')
            text=json.dumps(document,indent=2)+'\n' if args.action=='export' else render(document)
            atomic_write(args.output,text)
        print(json.dumps(dict(status='passed',schema=SCHEMA,definitions=len(document['definitions']),action=args.action)))
    except (ValueError,KeyError,TypeError,configparser.Error) as error:
        ap.exit(2,f'Definition validation failed: {error}\n')


if __name__=='__main__':main()
