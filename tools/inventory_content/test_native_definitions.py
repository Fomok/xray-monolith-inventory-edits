"""Roundtrip and atomic rejection tests for the local authoring boundary."""
import copy
import json
from pathlib import Path
import tempfile
import unittest

from native_definitions import migrate, validate, render, parse_ltx, atomic_write
from import_sqa3_catalog import pouch_layout


class Definitions(unittest.TestCase):
    def test_pouch_skyline_trims_only_unused_trailing_columns(self):
        self.assertEqual(pouch_layout([(1,2)]),(1,2,[1,1]))
        self.assertEqual(pouch_layout([(1,2),(1,2)]),(2,2,[1,2,1,2]))
    def setUp(self):
        source=Path(__file__).parents[1]/'inventory_lab/gamedata/configs/sqa3_catalog.json'
        self.preset=migrate(json.loads(source.read_text(encoding='utf-8')))

    def test_all_catalog_geometry_and_acceptance_survive_native_roundtrip(self):
        with tempfile.TemporaryDirectory(dir=Path(__file__).parent) as directory:
            path=Path(directory)/'catalog.ltx';atomic_write(path,render(self.preset))
            actual=parse_ltx(path)
            expected=copy.deepcopy(self.preset)
            for definition in expected['definitions'].values():definition.pop('provenance',None)
            self.assertEqual(expected,actual)

    def test_invalid_presets_leave_existing_output_unchanged(self):
        with tempfile.TemporaryDirectory(dir=Path(__file__).parent) as directory:
            path=Path(directory)/'catalog.ltx';path.write_bytes(b'known-good')
            changes=[('width',True),('height',65),('cells',[0]),('family','unknown'),
                     ('half',[0,2]),('tear_thresholds',[1000,8000]),('stack_limit',-1),
                     ('allow_container_items','false'),('typo',1)]
            for key,value in changes:
                preset=copy.deepcopy(self.preset);name=next(iter(preset['definitions']))
                preset['definitions'][name][key]=value
                with self.assertRaises(ValueError):atomic_write(path,render(preset))
                self.assertEqual(path.read_bytes(),b'known-good')

    def test_reject_unknown_schema_and_nonrig_growth(self):
        bad=copy.deepcopy(self.preset);bad['schema']='sqa3.storage.preset.v9'
        with self.assertRaisesRegex(ValueError,'unsupported version'):migrate(bad)
        bad=copy.deepcopy(self.preset);bad['definitions']['ampbox_meds']['growing']=True
        with self.assertRaisesRegex(ValueError,'only backpacks and rigs'):validate(bad)

    def test_reject_unknown_acceptance_flags_and_bad_row_structure(self):
        bad=copy.deepcopy(self.preset);bad['definitions']['ampbox_meds']['acceptance']['accept_flags']='everything'
        with self.assertRaisesRegex(ValueError,'unsupported classifier'):validate(bad)
        with tempfile.TemporaryDirectory(dir=Path(__file__).parent) as directory:
            path=Path(directory)/'bad.ltx';text=render(self.preset)
            text=text.replace('mask = 1,1,2|1,1,2|3,4,5','mask = 1,1|2,1,1,2|3,4,5',1);path.write_text(text)
            with self.assertRaisesRegex(ValueError,'row dimensions'):parse_ltx(path)


if __name__=='__main__':unittest.main()
