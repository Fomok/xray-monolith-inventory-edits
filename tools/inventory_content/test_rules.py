import copy
import unittest
import extract_catalog as catalog
import compile_rules as rules


class RuleTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.catalog = catalog.build()
        cls.rules = rules.compile_catalog(cls.catalog)
        cls.items = {i['id']: i for i in cls.rules['items']}

    def test_bandolier_exact_cells_and_canonical_names(self):
        g = self.items['amprig_bandolier']['geometry']
        self.assertEqual(g['grid'], {'w': 3, 'h': 3})
        self.assertEqual(g['cells'], [1, 1, 2, 1, 1, 2, 3, 4, 5])
        self.assertEqual(g['compartment_names'], {str(i): 'b'+str(i) for i in range(1, 6)})

    def test_canonical_order_not_authored_token_order(self):
        g = self.items['amprig_carrier']['geometry']
        self.assertEqual([r['w']*r['h'] for r in g['compartments']], [4]*4 + [3]*2 + [1]*4)

    def test_touching_box_regions_do_not_merge(self):
        g = self.items['ampbox_food']['geometry']
        self.assertEqual(g['cells'], [1, 1, 1, 2, 2, 2]*4)
        # A horizontal 4-cell footprint cannot fit in either 3-cell-wide region.
        self.assertNotEqual(g['cells'][2], g['cells'][3])

    def test_known_masks(self):
        hip = self.items['ampbox_hip']['geometry']
        self.assertEqual(hip['cells'], list(range(1, 16)))
        pistol = self.items['ampbox_pistolcase']['geometry']
        self.assertEqual(len(pistol['compartments']), 13)
        bag = self.items['zb_bd4zhvqbyadxczm3utf11a7l3t_8hpwbhfzaik6o4ijgkcuu7a3v']['geometry']
        self.assertEqual([bag['cells'].count(i) for i in range(1, 7)], [4]*6)

    def test_holes_are_blocked_and_invalid_overlap_rejected(self):
        g = rules.mask(3, 1, [{'name': 'a', 'x': 0, 'y': 0, 'w': 1, 'h': 1}, {'name': 'b', 'x': 2, 'y': 0, 'w': 1, 'h': 1}])
        self.assertEqual(g['cells'], [1, 0, 2])
        with self.assertRaises(ValueError):
            rules.box_geometry({'amp_box_w': '3', 'amp_box_h': '1', 'amp_box_slots': '1,1,2,1|2,1,2,1'})
        with self.assertRaises(ValueError):
            rules.box_geometry({'amp_box_w': '3', 'amp_box_h': '1', 'amp_box_slots': '3,1,2,1'})

    def test_unresolved_layout_and_fan_not_invented(self):
        for name in ('amprig_nylon', 'ampbox_docs'):
            self.assertIsNone(self.items[name]['geometry'])
            self.assertEqual(self.items[name]['status'], 'unresolved_geometry')

    def test_grant_keys_survive_other_mount_removal(self):
        p = next(p for p in self.rules['pouches'] if p['id'] == 'af_magpouch_m')
        first, second = p['grant_templates']
        self.assertEqual([g['name'] for g in first['compartments']], ['p1_1', 'p1_2'])
        self.assertEqual([g['name'] for g in second['compartments']], ['p2_1', 'p2_2'])
        remaining = [second]
        self.assertEqual(remaining[0]['mount_id'], 2)
        self.assertIsNone(second['parent_grid_placement'])
        self.assertEqual([m['minimum_condition'] for m in self.items['amprig_bandolier']['mounts']], [1000, 5000])

    def test_tall_pouch_and_backpack(self):
        p = next(p for p in self.rules['pouches'] if p['id'].endswith('64v17nxkaz5z6q30xrpssqh1p'))
        self.assertEqual([x['grid'] for x in p['grant_templates'][1]['compartments']], [{'w': 1, 'h': 3}]*2)
        b = next(b for b in self.rules['backpacks'] if b['key'] == 'equ_tourist_pack')
        self.assertEqual(b['geometry']['grid'], {'w': 8, 'h': 15})
        self.assertEqual(len(b['geometry']['cells']), 120)

    def test_stack_precedence_and_absolute_menu_override(self):
        p = self.rules['stack_policy']
        self.assertEqual(rules.stack_limit(p, 'bolt', 'w_ammo', {'is_mag'}), 40)
        self.assertEqual(rules.stack_limit(p, 'foo_patch', 'w_ammo', {'is_mag'}), 20)
        self.assertEqual(rules.stack_limit(p, 'magazine', 'w_ammo', {'is_mag'}), 1)
        self.assertEqual(rules.stack_limit(p, 'ammo', 'w_ammo', set(), multiplier=1.25), 25)
        self.assertEqual(rules.stack_limit(p, 'bolt', 'w_ammo', set(), kind_overrides={'w_ammo': 7}, multiplier=2), 7)
        self.assertEqual(rules.stack_limit(p, 'unknown', 'unknown', set(), multiplier=10), 1)
        self.assertEqual(rules.stack_limit(p, 'bolt', 'w_ammo', set(), enabled=False), 1)

    def test_longest_substring_and_lexical_flag_order(self):
        p = copy.deepcopy(self.catalog)
        doc = next(d for d in p['documents'] if d['category'] == 'stack_policy')
        for name, entries in [('grid_stack_like', [('patch', '2'), ('long_patch', '9')]), ('grid_stack_flag', [('z_flag', '4'), ('a_flag', '3')])]:
            s = next(s for s in doc['sections'] if s['name'] == name)
            s['entries'] = [{'key': k, 'value': v, 'source': {'file': 'fixture', 'line': 1}} for k, v in entries]
        policy = rules.compile_catalog(p)['stack_policy']
        self.assertEqual(rules.stack_limit(policy, 'long_patch', '', set()), 9)
        self.assertEqual(rules.stack_limit(policy, 'other', '', {'z_flag', 'a_flag'}), 3)

    def test_acceptance_order_and_weapon_exclusions(self):
        p = copy.deepcopy(next(p for p in self.rules['acceptance_families'] if p['id'] == 'weapon'))
        facts = {'section': 'test', 'is_rig': False, 'is_ammo': False, 'is_mag': False, 'class': 'WP_AK74', 'kind': ''}
        self.assertTrue(rules.accepts(p, facts))
        for cls in ('WP_SCOPE', 'WP_SILEN', 'WP_GLAUN'):
            self.assertFalse(rules.accepts(p, dict(facts, **{'class': cls})))
        p['also'] = p['never'] = ['test']
        self.assertFalse(rules.accepts(p, facts))
        p['never'] = []
        self.assertFalse(rules.accepts(p, dict(facts, is_rig=True)))
        self.assertIsNone(rules.accepts(p, {'section': 'unknown'}))

    def test_runtime_pin_and_determinism(self):
        self.assertEqual(catalog.encode(self.rules), catalog.encode(rules.compile_catalog(self.catalog)))
        changed = copy.deepcopy(self.catalog)
        next(f for f in changed['files'] if f['id'] == rules.RUNTIME)['sha256'] = 'changed'
        with self.assertRaises(ValueError):
            rules.compile_catalog(changed)


if __name__ == '__main__':
    unittest.main(verbosity=2)
