"""Copy audited rig/pouch parts data, without the old inventory runtime."""
import argparse
import hashlib
import json
from pathlib import Path
import re


def main():
    ap=argparse.ArgumentParser();ap.add_argument('catalog',type=Path);ap.add_argument('native_parts_config',type=Path)
    ap.add_argument('--source',type=Path,action='append',required=True);ap.add_argument('--output',type=Path,required=True)
    a=ap.parse_args();catalog=json.loads(a.catalog.read_text(encoding='utf-8'));supported=set(catalog['definitions'])
    known=set(re.findall(r'^\[([^\]:]+)',a.native_parts_config.read_text(encoding='utf-8-sig'),re.M))
    groups={'nor_parts_list':{},'con_parts_list':{}};sources=[]
    for source in a.source:
        raw=source.read_bytes();group=None
        for number,line in enumerate(raw.decode('utf-8-sig').splitlines(),1):
            line=line.split(';',1)[0].strip()
            if not line:continue
            match=re.fullmatch(r'!\[([^\]]+)\]',line)
            if match:group=match[1];assert group in groups;continue
            if group and '=' in line:
                name,value=map(str.strip,line.split('=',1))
                if name not in supported:continue
                parts=[part.strip() for part in value.split(',')]
                assert parts and all(part in known for part in parts),f'{source}:{number}: unavailable native part'
                assert name not in groups[group],f'duplicate source recipe: {name}'
                groups[group][name]=parts
        sources.append(dict(path=str(source.resolve()),sha256=hashlib.sha256(raw).hexdigest(),bytes=len(raw)))
    lines=['; Original Squared Away rig and Mags Reloaded pouch parts data.',
           '; Native preflight returns fitted pouches separately; no runtime copied.']
    for group,recipes in groups.items():
        lines += ['',f'![{group}]']+[f'{name} = '+', '.join(parts) for name,parts in recipes.items()]
    a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_text('\n'.join(lines)+'\n',encoding='utf-8')
    report=dict(schema='sqa3.native.parts.v1',sources=sources,native_parts_config_sha256=hashlib.sha256(a.native_parts_config.read_bytes()).hexdigest(),recipes=groups,
                supplying_mods=['released Squared Away 2.1.0 Hotfix1','Mags Reloaded Fork by Priler UPDATE6 (read-only recipe input)'])
    a.output.with_suffix('.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(json.dumps({key:len(value) for key,value in groups.items()}))


if __name__=='__main__':main()
