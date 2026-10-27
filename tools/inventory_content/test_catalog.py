import json
from pathlib import Path
import unittest
import extract_catalog as c


class ParserTests(unittest.TestCase):
    def resolve(self, text):
        blocks, _, _ = c.parse_ltx(text, "fixture")
        errors = []
        return {v["id"]: v for v in c.resolve_system(blocks, errors)}, errors

    def test_inheritance_override_and_provenance(self):
        nodes, errors = self.resolve('[base]\nx=1\ny=2\n[child]:base\ny=3\n![child]\ny=4')
        self.assertFalse(errors)
        self.assertEqual(nodes['child']['fields']['x']['value'], '1')
        self.assertEqual(nodes['child']['fields']['y'], {'value': '4', 'source': {'file': 'fixture', 'line': 7}})
        self.assertEqual(len(nodes['child']['history']), 2)

    def test_external_and_cycle_do_not_claim_complete(self):
        nodes, _ = self.resolve('[a]:b\nx=1\n[b]:a\n![external]\ny=2\n[c]:missing')
        self.assertTrue(all(n['resolution'] == 'partial' for n in nodes.values()))
        self.assertIn('external_parent:missing', nodes['c']['unresolved'])

    def test_identifier_failures_and_duplicate_declaration(self):
        _, errors = self.resolve('[]\n[' + 'a'*64 + ']\n[x]\n[x]')
        self.assertEqual([e['code'] for e in errors].count('invalid_system_identifier'), 2)
        self.assertIn('duplicate_declaration', [e['code'] for e in errors])

    def test_preserve_duplicates_unknown_and_quoted_semicolon(self):
        blocks, directives, unknown = c.parse_ltx('orphan\n#include "a.ltx"\n[a]\nx="a;b" ; comment\nx=2\n!x\n', 'fixture')
        self.assertEqual(len(blocks[0]['entries']), 3)
        self.assertEqual(blocks[0]['entries'][0]['value'], '"a;b"')
        self.assertEqual(len(directives), 1)
        self.assertEqual(len(unknown), 1)
        nodes, _ = self.resolve('[a]\n!x')
        self.assertEqual(nodes['a']['resolution'], 'partial')


class ReleaseTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.catalog = c.build()
        cls.items = {n['id']: n for n in cls.catalog['profiles'][0]['definitions']}

    def field(self, name, key):
        return self.items[name]['fields'][key]['value']

    def test_no_extraction_errors(self):
        self.assertFalse([d for d in self.catalog['diagnostics'] if d['severity'] == 'error'])

    def test_rig_and_native_override(self):
        self.assertEqual(self.field('amprig_bandolier', 'amp_slot_2x2'), '1')
        self.assertEqual(self.field('amprig_bandolier', 'inv_grid_height'), '6')
        self.assertEqual(self.field('amprig_bandolier', 'slot'), '15')
        self.assertGreater(len(self.items['amprig_bandolier']['history']), 1)
        self.assertIn('external_parent:af_base', self.items['amprig_bandolier']['unresolved'])

    def test_pouch_box_and_full_length_ids(self):
        pouch = 'zb_pd4zhvqbyadxczm3utf11a7l3t_31hlfiatyyljkszf5y7j9pcdj'
        bag = 'zb_bd4zhvqbyadxczm3utf11a7l3t_8hpwbhfzaik6o4ijgkcuu7a3v'
        self.assertEqual(self.field(pouch, 'amp_pgrant_3x1'), '1')
        self.assertEqual(self.field(bag, 'amp_box_stack'), '99')
        self.assertEqual(self.field('ampbox_pistolcase', 'inv_grid_width'), '4')

    def test_backpack_orientation_and_capacity(self):
        packs = {s['key']: s for s in self.catalog['backpack_sizes']}
        self.assertEqual((packs['equ_tourist_pack']['rows'], packs['equ_tourist_pack']['columns'], packs['equ_tourist_pack']['cells']), (15, 8, 120))

    def test_domain_records_and_recipe_references(self):
        categories = {t['category'] for t in self.catalog['tables']}
        self.assertTrue({'repair', 'traders', 'loot_and_loadouts', 'box_acceptance', 'stack_policy', 'rig_and_pouch_grants'} <= categories)
        recipe = next(r for r in self.catalog['recipes'] if r['output'] == 'amprig_bandolier')
        self.assertEqual(recipe['ingredients'][0], {'id': 'af_magpouch_l', 'count': 1})
        self.assertTrue(any(d.get('target') == 'sewing_thread' and d['status'] == 'external_definition_required' for d in self.catalog['dependencies']))

    def test_stack_and_acceptance(self):
        def entry(category, section, key):
            return next(e['value'] for d in self.catalog['documents'] if d['category'] == category for s in d.get('sections', []) if s['name'] == section for e in s['entries'] if e['key'] == key)
        self.assertEqual(entry('stack_policy', 'grid_stack_section', 'bolt'), '40')
        self.assertEqual(entry('box_acceptance', 'amp_box_armor', 'rigs'), 'true')

    def test_optional_layers_and_localized_descriptions(self):
        self.assertIn('stash', [p['id'] for p in self.catalog['profiles']])
        self.assertIn('optional/sota', [p['id'] for p in self.catalog['profiles']])
        description = self.field('amprig_bandolier', 'description')
        self.assertTrue(any(v['id'] == description and v['language'] == 'eng' for v in self.catalog['localizations']))
        self.assertTrue(self.catalog['texture_regions'])

    def test_every_file_hash_and_round_trip(self):
        for f in self.catalog['files']:
            profile = f['profile']
            root = c.DEFAULT_STASH if profile == 'stash' else c.DEFAULT_SOURCE
            if profile.startswith('optional/'):
                root = root / profile
            self.assertEqual(c.digest((root / f['path']).read_bytes()), f['sha256'])
        self.assertEqual(json.loads(c.encode(self.catalog)), self.catalog)

    def test_deterministic_rebuild(self):
        self.assertEqual(c.encode(self.catalog), c.encode(c.build()))


if __name__ == '__main__':
    unittest.main(verbosity=2)
