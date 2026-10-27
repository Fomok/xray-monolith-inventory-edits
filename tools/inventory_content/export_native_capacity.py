"""Export audited backpack rules for the isolated native capacity loader."""
import argparse
import hashlib
import json
import re
from pathlib import Path


def export(rules_path, source, output):
    rules = json.loads(rules_path.read_text())
    assert rules['schema'] == 'sqa.inventory-rules' and rules['schema_version'] == '1.0.0'
    grid_path = source / 'gamedata/scripts/zzz_amp_grid.script'
    grid = grid_path.read_text(encoding='utf-8-sig')
    cells = int(re.search(r'^\s*grid_bare\s*=\s*(\d+)\s*,', grid, re.M)[1])
    columns = int(re.search(r'^\s*grid_w_bare\s*=\s*(\d+)\s*,', grid, re.M)[1])
    assert cells == 8 and columns == 4, 'Re-audit changed no-backpack defaults'
    groups = {name: {} for name in ['grid_pack_default', 'grid_pack_kind', 'grid_pack_section']}
    for entry in rules['backpacks']:
        group = groups[entry['selector']]
        assert entry['key'] not in group and entry['geometry'], 'Duplicate or unresolved backpack geometry'
        value = entry['raw']
        assert re.fullmatch(r'\d+(?:[xX]\d+)?', value), 'Invalid authored shape'
        group[entry['key']] = value
    assert groups['grid_pack_default'].get('size'), 'Missing default backpack shape'
    lines = ['; Generated from audited SQA2 rules; no legacy runtime scripts.',
             '; rules_sha256=' + hashlib.sha256(rules_path.read_bytes()).hexdigest(),
             '; pocket_source_sha256=' + hashlib.sha256(grid_path.read_bytes()).hexdigest(),
             '; Sizes are ROWS x COLUMNS; carry weight does not grant cells.',
             '[sqa3_pockets]', f'size = {cells}', f'columns = {columns}', '']
    for name, entries in groups.items():
        lines += ['['+name+']']
        lines += [key+' = '+value for key, value in sorted(entries.items())]
        lines += ['']
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text('\n'.join(lines), encoding='utf-8')
    print(json.dumps({'output': str(output), 'sha256': hashlib.sha256(output.read_bytes()).hexdigest(),
                      'backpack_rules': sum(map(len, groups.values())), 'gameplay': 'not_run'}))


if __name__ == '__main__':
    p = argparse.ArgumentParser()
    p.add_argument('--rules', type=Path, required=True)
    p.add_argument('--source', type=Path, required=True)
    p.add_argument('--output', type=Path, required=True)
    args = p.parse_args()
    export(args.rules, args.source, args.output)
