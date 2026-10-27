"""Compile catalog v1 into source-traceable SQA3 rules; no game execution."""
import argparse
import json
import math
import re
from pathlib import Path
import extract_catalog as c

SHAPES = {'2x2': (2, 2), '3x1': (1, 3), '2x1': (1, 2), '1x1': (1, 1)}
RUNTIME = 'release/gamedata/scripts/zzz_armor_mag_pouches.script'
GRID_RUNTIME = 'release/gamedata/scripts/zzz_amp_grid.script'
PINNED = {RUNTIME: '57a7f2cf05cac038caa3d27ae0fdeab4f00989a9009b6aca99cf904145785300',
          GRID_RUNTIME: '44009035be46f2a76bda34488ac28e994de59b5702dc47f66b59ad84996bc332'}


def trace(start, end, grid=False):
    return {'file': GRID_RUNTIME if grid else RUNTIME, 'line_start': start, 'line_end': end}


def mask(width, height, rectangles):
    if not 1 <= width <= 256 or not 1 <= height <= 256:
        raise ValueError('grid dimensions outside compiler bound')
    cells = [0] * (width * height)
    names = {}
    for region, rect in enumerate(rectangles, 1):
        x, y, w, h = (rect[k] for k in ('x', 'y', 'w', 'h'))
        if w < 1 or h < 1 or x < 0 or y < 0 or x+w > width or y+h > height:
            raise ValueError('compartment outside grid')
        if rect['name'] in names.values():
            raise ValueError('duplicate compartment name')
        names[str(region)] = rect['name']
        for row in range(y, y+h):
            for col in range(x, x+w):
                at = row * width + col
                if cells[at]:
                    raise ValueError('overlapping compartments')
                cells[at] = region
    return {'grid': {'w': width, 'h': height}, 'cells': cells,
            'compartment_names': names, 'compartments': rectangles}


def rig_geometry(fields):
    counts = {}
    for shape in SHAPES:
        key = 'amp_slot_' + shape
        if key not in fields:
            raise ValueError('unresolved inherited count: ' + key)
        counts[shape] = min(16, max(0, math.floor(float(fields[key]))))
    raw = fields.get('amp_layout', '')
    match = re.fullmatch(r'\s*(\d+)\s*:\s*(.+)', raw)
    if not match:
        raise ValueError('no authored layout; runtime packing requires separate policy')
    band = int(match[1])
    if not 1 <= band <= 8:
        raise ValueError('rig band outside 1..8')
    groups = {shape: [] for shape in SHAPES}
    for token in re.split(r'[|;]', match[2]):
        part = re.fullmatch(r'\s*(\w+)@(\d+)[.,](\d+)\s*', token)
        if not part or part[1] not in SHAPES:
            raise ValueError('invalid rig layout token')
        w, h = SHAPES[part[1]]
        x, y = int(part[2])-1, int(part[3])-1
        if x+w > band:
            raise ValueError('rig pocket exceeds band')
        groups[part[1]].append({'x': x, 'y': y, 'w': w, 'h': h})
    if any(len(groups[s]) != counts[s] for s in SHAPES):
        raise ValueError('rig layout/count mismatch')
    rectangles = [r for s in SHAPES for r in groups[s]]
    if not rectangles:
        raise ValueError('empty rig')
    for i, rect in enumerate(rectangles, 1):
        rect['name'] = 'b' + str(i)
    return mask(max(r['x']+r['w'] for r in rectangles), max(r['y']+r['h'] for r in rectangles), rectangles)


def box_geometry(fields):
    if fields.get('amp_box_fan') == 'true':
        raise ValueError('fan storage ignores item footprints; requires non-grid storage policy')
    if 'amp_box_w' not in fields or 'amp_box_h' not in fields:
        raise ValueError('unresolved inherited box dimensions')
    w, h = [min(12, max(1, math.floor(float(fields[k])))) for k in ('amp_box_w', 'amp_box_h')]
    raw = fields.get('amp_box_slots', '')
    rectangles = []
    if raw:
        for index, token in enumerate(raw.split('|'), 1):
            nums = re.findall(r'-?\d+', token)
            if len(nums) != 4:
                raise ValueError('box slot needs four numbers')
            x, y, rw, rh = map(int, nums)
            rectangles.append({'name': 'box' + str(index), 'x': x-1, 'y': y-1, 'w': rw, 'h': rh})
    elif fields.get('amp_box_cells') == 'true':
        rectangles = [{'name': 'box' + str(y*w+x+1), 'x': x, 'y': y, 'w': 1, 'h': 1} for y in range(h) for x in range(w)]
    else:
        rectangles = [{'name': 'box1', 'x': 0, 'y': 0, 'w': w, 'h': h}]
    # Deliberately reject malformed authored masks instead of silently dropping
    # invalid slots as the legacy renderer does. No broader fallback admission.
    return mask(w, h, rectangles)


def grant_templates(shapes):
    return [{'mount_name': 'p' + str(mount), 'mount_id': mount,
             'compartments': [{'name': f'p{mount}_{i}', 'grid': {'w': w, 'h': h},
                               'cells': [1]*(w*h), 'compartment_names': {'1': f'p{mount}_{i}'}}
                              for i, (w, h) in enumerate(shapes, 1)],
             'parent_grid_placement': None} for mount in (1, 2)]


def stack_limit(policy, section, kind, flags, *, section_overrides=None, kind_overrides=None, multiplier=1, enabled=True):
    """Reference evaluator; caller must provide resolved kind/flags and settings."""
    if not enabled or not section:
        return 1
    overrides = [(section_overrides or {}).get(section), (kind_overrides or {}).get(kind)]
    for number in overrides:
        if number is not None:
            return max(1, math.floor(number + .5))
    for rule in policy['rules']:
        mode, key = rule['match'], rule['key']
        matches = ((mode == 'section' and section == key) or (mode == 'substring' and key in section)
                   or (mode == 'flag' and key in flags) or (mode == 'kind' and kind == key))
        if matches:
            return max(1, math.floor(rule['limit'] * multiplier + .5))
    return 1


def accepts(policy, facts):
    """Family predicate only. Nesting/host classification must be resolved first."""
    required = ('section', 'is_rig', 'is_ammo', 'is_mag', 'class', 'kind')
    if any(k not in facts for k in required):
        return None
    if facts['is_rig'] and not policy['rigs']:
        return False
    if facts['section'] in policy['never']:
        return False
    if facts['section'] in policy['also']:
        return True
    return bool((policy['rigs'] and facts['is_rig']) or (policy['ammo'] and facts['is_ammo'])
                or (policy['mags'] and facts['is_mag']) or (policy['guns'] and facts['class'].startswith('WP_')
                and facts['class'] not in ('WP_SILEN', 'WP_SCOPE', 'WP_GLAUN')) or facts['kind'] in policy['kinds'])


def compile_catalog(catalog):
    if catalog['schema_version'] != '1.0.0':
        raise ValueError('unsupported catalog schema')
    hashes = {f['id']: f['sha256'] for f in catalog['files']}
    if any(hashes.get(k) != v for k, v in PINNED.items()):
        raise ValueError('runtime semantics hash changed: re-audit required')
    out = {'schema': 'sqa.inventory-rules', 'schema_version': '1.0.0',
           'catalog_sha256': c.digest(c.encode(catalog)), 'runtime_semantics': PINNED,
           'items': [], 'pouches': [], 'backpacks': [], 'acceptance_families': [],
           'stack_policy': {'precedence': ['menu_section', 'menu_kind', 'section', 'substring', 'flag', 'kind', 'fallback_one'],
                            'substring_order': 'length_desc_then_lexical', 'flag_order': 'lexical',
                            'rounding': 'max(1,floor(base*multiplier+0.5))', 'menu_overrides_scale': False,
                            'runtime_settings_required': ['enabled', 'stack', 'multiplier', 'section_overrides', 'kind_overrides'],
                            'rules': [], 'source': trace(3413, 3656, True)},
           'unresolved': []}
    for profile in catalog['profiles']:
        for item in profile['definitions']:
            fields = {k: v['value'] for k, v in item['fields'].items()}
            source = {k: v['source'] for k, v in item['fields'].items() if k.startswith('amp_')}
            kind = 'rig' if fields.get('amp_rig') == 'true' else 'box' if fields.get('amp_box') == 'true' else None
            if kind:
                rule = {'id': item['id'], 'profile': profile['id'], 'kind': kind, 'source': source,
                        'host_dependencies': item['unresolved'], 'geometry': None, 'status': 'compiled_geometry'}
                try:
                    rule['geometry'] = rig_geometry(fields) if kind == 'rig' else box_geometry(fields)
                except (ValueError, OverflowError) as exc:
                    rule['status'] = 'unresolved_geometry'
                    rule['reason'] = str(exc)
                    out['unresolved'].append({'id': item['id'], 'reason': str(exc)})
                if kind == 'rig':
                    rule['runtime_source'] = trace(1153, 1424)
                    rule['mounts'] = [{'id': i, 'name': 'p'+str(i), 'minimum_condition': condition,
                                       'footprint': None, 'accepts_predicate': 'pouch_grant_exists'} for i, condition in ((1, 1000), (2, 5000))]
                    rule['mount_source'] = trace(4253, 4283)
                    rule['mount_unresolved'] = ['native_mount_footprint_and_category_mapping', 'grant_parent_grid_placement', 'inactive_mount_retention_and_grant_visibility']
                else:
                    rule['runtime_source'] = trace(16620, 16718)
                    rule['acceptance_family'] = fields.get('amp_box_takes')
                    rule['max_stack_quantity'] = int(fields['amp_box_stack']) if 'amp_box_stack' in fields else None
                    rule['nesting_policy'] = 'requires_SQA3_rigid_soft_classification; legacy_blanket_box_rejection_not_imported'
                out['items'].append(rule)
            if fields.get('amp_pouch') == 'true':
                shapes = []
                missing = []
                for shape, dimensions in SHAPES.items():
                    key = 'amp_pgrant_' + shape
                    if key not in fields:
                        missing.append(key)
                    else:
                        count = int(fields[key])
                        if not 0 <= count <= 256:
                            raise ValueError('invalid pouch grant count')
                        shapes.extend([dimensions]*count)
                out['pouches'].append({'id': item['id'], 'profile': profile['id'], 'source': source,
                                       'grant_templates': grant_templates(shapes) if not missing else [],
                                       'unresolved': missing, 'runtime_source': trace(1333, 1384)})
    for name, shapes in [('af_magpouch_s', [(1, 2)]), ('af_magpouch_m', [(1, 2), (1, 2)]), ('af_magpouch_l', [(2, 2)])]:
        out['pouches'].append({'id': name, 'profile': 'release', 'grant_templates': grant_templates(shapes),
                               'runtime_source': trace(1333, 1360), 'precedence': 'runtime_named_grant_before_section_keys',
                               'unresolved': ['external_item_definition']})
    section_modes = {'grid_stack_section': 'section', 'grid_stack_like': 'substring', 'grid_stack_flag': 'flag', 'grid_stack_kind': 'kind'}
    for doc in catalog['documents']:
        if not doc['file'].startswith('release/'):
            continue
        for section in doc.get('sections', []):
            if doc['category'] == 'stack_policy' and section['name'] in section_modes:
                for entry in section['entries']:
                    number = float(entry['value'])
                    if not math.isfinite(number) or number <= 0:
                        raise ValueError('invalid stack rule')
                    out['stack_policy']['rules'].append({'match': section_modes[section['name']], 'key': entry['key'], 'limit': number, 'source': entry['source']})
            if doc['category'] == 'box_acceptance':
                values = {e['key']: e['value'] for e in section['entries']}
                policy = {'id': section['name'].removeprefix('amp_box_'), 'source': section['source'],
                          'runtime_source': trace(17020, 17175), 'precedence': ['rig_gate', 'never', 'also', 'rigs', 'ammo', 'mags', 'guns', 'kinds', 'deny'],
                          'host_facts_required': ['is_rig', 'is_ammo', 'is_mag', 'class', 'kind'],
                          'message': values.get('no'), 'nesting_gate': 'separate_SQA3_policy'}
                for key in ('kinds', 'also', 'never'):
                    policy[key] = [v.strip() for v in (values.get(key) or '').split(',') if v.strip()]
                for key in ('ammo', 'mags', 'guns', 'rigs'):
                    policy[key] = values.get(key) == 'true'
                out['acceptance_families'].append(policy)
    rank = {'section': 0, 'substring': 1, 'flag': 2, 'kind': 3}
    out['stack_policy']['rules'].sort(key=lambda r: (rank[r['match']], -len(r['key']) if r['match'] == 'substring' else 0, r['key']))
    families = {p['id'] for p in out['acceptance_families']}
    for item in out['items']:
        if item['kind'] == 'box' and item['acceptance_family'] not in families:
            out['unresolved'].append({'id': item['id'], 'reason': 'missing family; legacy accepts non-box/non-rig; SQA3 policy required'})
    for size in catalog['backpack_sizes']:
        record = dict(size)
        record['geometry'] = mask(size['columns'], size['rows'], [{'name': 'storage', 'x': 0, 'y': 0, 'w': size['columns'], 'h': size['rows']}]) if size.get('rows') and size.get('columns') else None
        record['unresolved'] = ['host_item_definition_and_exterior', 'UI_reflow_separate_from_authored_capacity']
        out['backpacks'].append(record)
    return out


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--catalog', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    rules = compile_catalog(json.loads(args.catalog.read_bytes()))
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(c.encode(rules))
    print(json.dumps({'sha256': c.digest(c.encode(rules)), 'items': len(rules['items']), 'unresolved': rules['unresolved']}))


if __name__ == '__main__':
    main()
