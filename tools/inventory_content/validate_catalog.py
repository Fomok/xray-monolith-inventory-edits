"""Produce reproducible catalog and offline validation evidence in one directory."""
import argparse
import io
import json
from pathlib import Path
import unittest
import zipfile
import extract_catalog as c
import test_catalog
import test_rules
import compile_rules


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output-dir', type=Path, required=True)
    args = parser.parse_args()
    args.output_dir.mkdir(parents=True, exist_ok=True)
    log = io.StringIO()
    suite = unittest.TestSuite(unittest.defaultTestLoader.loadTestsFromModule(module) for module in (test_catalog, test_rules))
    result = unittest.TextTestRunner(stream=log, verbosity=2).run(suite)
    catalog = c.build()
    data = c.encode(catalog)
    second = c.encode(c.build())
    rules = compile_rules.compile_catalog(catalog)
    rule_bytes = c.encode(rules)
    rule_repeat = c.encode(compile_rules.compile_catalog(catalog))
    (args.output_dir / 'rules-v1.json').write_bytes(rule_bytes)
    fixture_ids = {'amprig_bandolier', 'amprig_carrier', 'ampbox_food', 'ampbox_hip', 'ampbox_pistolcase',
                   'zb_bd4zhvqbyadxczm3utf11a7l3t_8hpwbhfzaik6o4ijgkcuu7a3v'}
    fixtures = {'rules_sha256': c.digest(rule_bytes), 'items': [i for i in rules['items'] if i['id'] in fixture_ids],
                'pouches': rules['pouches'], 'backpacks': rules['backpacks']}
    (args.output_dir / 'rule-fixtures-v1.json').write_bytes(c.encode(fixtures))
    archive = c.DEFAULT_SOURCE.parent / 'Squared Away 2.1.0 Hotfix 1.zip'
    with zipfile.ZipFile(archive) as package:
        members = [n for n in package.namelist() if not n.endswith('/')]
        differences = [n for n in members if not (c.DEFAULT_SOURCE / n).is_file()
                       or package.read(n) != (c.DEFAULT_SOURCE / n).read_bytes()]
    release_evidence = {'path': archive.as_posix(), 'sha256': c.digest(archive.read_bytes()),
                        'entries': len(members), 'source_differences': differences}
    (args.output_dir / 'catalog-v1.json').write_bytes(data)
    (args.output_dir / 'tests.txt').write_text(log.getvalue(), encoding='utf-8')
    report = {'schema_version': c.VERSION, 'test_count': result.testsRun,
              'test_success': result.wasSuccessful(), 'deterministic_rebuild': data == second,
              'catalog_sha256': c.digest(data), 'catalog_bytes': len(data),
              'rules_sha256': c.digest(rule_bytes), 'rules_deterministic_rebuild': rule_bytes == rule_repeat,
              'rule_items': len(rules['items']), 'pouch_rules': len(rules['pouches']),
              'unresolved_rules': rules['unresolved'],
              'input_manifest_sha256': catalog['input_manifest_sha256'],
              'release_archive': release_evidence,
              'files': len(catalog['files']), 'documents': len(catalog['documents']),
              'profiles': {p['id']: len(p['definitions']) for p in catalog['profiles']},
              'recipes': len(catalog['recipes']), 'localizations': len(catalog['localizations']),
              'texture_regions': len(catalog['texture_regions']),
              'diagnostics': catalog['diagnostics'], 'game_validation': 'not_run',
              'tool_files': {p.name: c.digest(p.read_bytes()) for p in sorted(Path(__file__).parent.glob('*.py'))}}
    (args.output_dir / 'validation.json').write_bytes(c.encode(report))
    print(json.dumps(report, indent=2))
    return 0 if result.wasSuccessful() and data == second and rule_bytes == rule_repeat and not differences else 1


if __name__ == '__main__':
    raise SystemExit(main())
