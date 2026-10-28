"""Render the audited catalog for the isolated native lab, without legacy scripts.

Large art remains in the hash-pinned content handoff, like the other lab atlases.
The packager verifies every byte before including it.
"""
import argparse
import hashlib
import json
import re
from pathlib import Path

def prepare(package, output):
    manifest = json.loads((package / 'manifest.json').read_text(encoding='utf-8'))
    pinned = []
    for entry in manifest['payload_files']:
        relative = entry['path'].replace('\\', '/')
        path = package / relative
        data = path.read_bytes()
        assert len(data) == entry['bytes']
        assert hashlib.sha256(data).hexdigest() == entry['sha256']
        if relative.startswith('gamedata/'):
            pinned.append(dict(entry, path=relative, source=str(path.resolve())))
    template = (package / 'templates/sqa3_items.ltx.in').read_text(encoding='utf-8')
    backpacks = {'itm_backpack', 'equ_small_pack', 'equ_small_military_pack',
                 'equ_military_pack', 'equ_tourist_pack'}
    rendered = ['; Native SQA3 lab content. Existing backpacks retain II_BPACK.',
                '; Missing original kevlar mesh explicitly mapped to SQA rig_shared.',
                '; Portable item custody uses II_ATTCH, never old rig/pouch slots.']
    sections = re.split(r'^\[([^\]]+)\]\s*$', template, flags=re.M)
    assert len(sections) == 99
    for name, body in zip(sections[1::2], sections[2::2]):
        backpack = name in backpacks
        rendered.append(f'![{name}]' if backpack else f'[{name}]:tch_junk')
        if not backpack:
            rendered += ['slot = -1', 'auto_attach = false']
        for line in body.splitlines():
            if not line.strip() or line.lstrip().startswith(';'):
                continue
            if backpack and line.startswith('class ='):
                continue
            line = line.replace('${SQA3_NATIVE_ITEM_CLASS}', 'II_ATTCH')
            line = line.replace('dynamics\\equipments\\aa\\kevlar', 'squared_away\\rig_shared.ogf')
            rendered.append(line)
        rendered.append('')
    text = '\n'.join(rendered) + '\n'
    assert '${' not in text and 'II_CONTR' not in text
    config = output / 'gamedata/configs/mod_system_sqa3_catalog.ltx'
    config.parent.mkdir(parents=True, exist_ok=True)
    config.write_text(text, encoding='utf-8')
    metadata = dict(schema='sqa3.native.content.v1', files=pinned,
                    source_catalog_sha256=manifest['source_catalog_sha256'],
                    missing_original_mesh='dynamics/equipments/aa/kevlar.ogf',
                    explicit_mesh_mapping='squared_away/rig_shared.ogf',
                    supplying_mods=manifest['files'], localization=manifest['localization'])
    (output / 'native-catalog-assets.json').write_text(json.dumps(metadata, indent=2)+'\n', encoding='utf-8')
    return {'sections': 49, 'pinned_asset_files': len(pinned)}


if __name__ == '__main__':
    ap = argparse.ArgumentParser()
    ap.add_argument('package', type=Path)
    ap.add_argument('--output', type=Path, required=True)
    args = ap.parse_args()
    print(json.dumps(prepare(args.package, args.output)))
