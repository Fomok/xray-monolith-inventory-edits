"""Build an isolated Bodycam-specific shader compatibility FOMOD."""
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
import argparse
import hashlib
import shutil
import xml.etree.ElementTree as ET
from package_compatibility import build, REPO, ARCHIVE_NAME

def package(output):
    build(output)
    stage = output / 'package'
    shaders = stage / 'components/bodycam/gamedata/shaders/r3'
    shaders.mkdir(parents=True)
    source = REPO / 'extras/dlss_bodycam/gamedata/shaders/r3'
    for item in source.iterdir():
        shutil.copyfile(item, shaders/item.name)
    shutil.copyfile(REPO/'gamedata/shaders/r3/scope_glass_nvg_focus.h', shaders/'scope_glass_nvg_focus.h')
    shutil.copyfile(REPO/'extras/dlss_bodycam/GRASS-CREDITS.txt', stage/'GRASS-CREDITS.txt')
    config = stage/'fomod/ModuleConfig.xml'
    tree = ET.parse(config)
    tree.find('moduleName').text = 'Squared Away Bodycam - DLSS / FSR 3 Test'
    ET.SubElement(tree.find('requiredInstallFiles'), 'folder', {
        'source': r'components\bodycam\gamedata', 'destination': 'gamedata', 'priority': '1000'})
    for desc in tree.findall('.//description'):
        desc.text = (desc.text or '').replace('0.5 DLSS/FSR3 modded binaries', 'matching Squared Away Bodycam DLSS test engine')
    # Bodycam ships its own Better Beef NVG shader and script. Preserve that pair
    # unconditionally instead of offering an incompatible stock NVG override.
    for group in tree.findall('.//group'):
        if group.get('name') == 'Select installed add-ons, or leave both unchecked':
            group.set('name', 'Optional WearableDevices support')
            plugins = group.find('plugins')
            for plugin in list(plugins):
                if 'Beef' in plugin.get('name', ''):
                    plugins.remove(plugin)
    conditional = tree.find('conditionalFileInstalls')
    if conditional is not None:
        tree.getroot().remove(conditional)
    tree.write(config, encoding='utf-8', xml_declaration=True)
    readme = (REPO/'docs/BODYCAM-DLSS-TEST.txt').read_text()
    (stage/'READ ME FIRST.txt').write_text(readme, encoding='utf-8')
    archive = output/'Squared Away Bodycam DLSS Shader Compatibility Test.zip'
    with ZipFile(archive, 'w', ZIP_DEFLATED, compresslevel=9) as z:
        for p in sorted(stage.rglob('*')):
            if p.is_file():
                z.write(p, p.relative_to(stage).as_posix())
    print(archive)
    print('SHA256', hashlib.sha256(archive.read_bytes()).hexdigest())

if __name__ == '__main__':
    p=argparse.ArgumentParser()
    p.add_argument('--output', required=True, type=Path)
    package(p.parse_args().output.resolve())
