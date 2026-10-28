"""Package one verified CI engine and minimal SQA visual overlay for the isolated lab."""
import argparse,hashlib,json,zipfile,subprocess
from pathlib import Path

def main():
    ap=argparse.ArgumentParser();ap.add_argument("--engine",type=Path,required=True);ap.add_argument("--engine-sha256",required=True);ap.add_argument("--commit",required=True);ap.add_argument("--ci",required=True);ap.add_argument("--output",type=Path,required=True)
    ap.add_argument("--overlay-commit", help="Source commit for scripts/XML when newer than the engine commit")
    a=ap.parse_args()
    source=Path(__file__).parent
    repo=source.parents[1]
    overlay=subprocess.check_output(['git','rev-parse','--verify',(a.overlay_commit or a.commit)+'^{commit}'],cwd=repo,text=True).strip()
    def read_overlay(relative,optional=False):
        result=subprocess.run(['git','show',overlay+':'+relative],cwd=repo,capture_output=True)
        if result.returncode:
            if optional:return None
            raise RuntimeError('Missing overlay source at pinned commit: '+relative)
        return result.stdout
    engine=a.engine.read_bytes();assert hashlib.sha256(engine).hexdigest()==a.engine_sha256
    atlas=json.loads(read_overlay('tools/inventory_lab/native-visual-assets.json'));texture=Path(atlas["source"]).read_bytes()
    assert len(texture)==atlas["bytes"] and hashlib.sha256(texture).hexdigest()==atlas["sha256"]
    files={"bin/AnomalyDX11AVX.exe":engine,atlas["destination"]:texture}
    for extra in atlas.get("additional_textures",[]):
        texture=read_overlay(extra["repo_relative"]) if "repo_relative" in extra else Path(extra["source"]).read_bytes()
        assert len(texture)==extra["bytes"] and hashlib.sha256(texture).hexdigest()==extra["sha256"]
        assert extra["destination"] not in files
        files[extra["destination"]]=texture
    for rel in ["configs/sqa3_capacity.ltx","configs/ui/sqa3_field_kit.xml","configs/mod_system_sqa3_field_kit.ltx","configs/ui/textures_descr/sqa3_native.xml","configs/ui/textures_descr/sqa3_visual_assets.xml","configs/ui/textures_descr/sqa3_visual_assets_v2.xml","configs/ui/textures_descr/sqa3_rank_icons.xml","configs/ui/sqa3_actor_menu.xml","scripts/sqa3_native_autorun.script","scripts/ui_inventory.script"]:
        data=read_overlay('tools/inventory_lab/gamedata/'+rel,optional=rel=='configs/sqa3_capacity.ltx')
        if data is not None:files["gamedata/"+rel]=data
    files['gamedata/scripts/sqa3_growth_mcm.script']=read_overlay('tools/inventory_lab/gamedata/scripts/sqa3_growth_mcm.script')
    files['gamedata/configs/text/eng/st_sqa3_growth_mcm.xml']=read_overlay('tools/inventory_lab/gamedata/configs/text/eng/st_sqa3_growth_mcm.xml')
    catalog=read_overlay('tools/inventory_lab/native-catalog-assets.json',optional=True)
    if catalog is not None:
        for entry in json.loads(catalog)['files']:
            data=Path(entry['source']).read_bytes()
            assert len(data)==entry['bytes'] and hashlib.sha256(data).hexdigest()==entry['sha256']
            assert entry['path'].startswith('gamedata/') and '..' not in entry['path'].split('/')
            assert entry['path'] not in files
            files[entry['path']]=data
        for name in ['sqa3_catalog.ltx','sqa3_equipment.ltx','mod_system_sqa3_catalog.ltx']:
            files['gamedata/configs/'+name]=read_overlay('tools/inventory_lab/gamedata/configs/'+name)
        files['catalog-provenance.json']=catalog
        item_list=read_overlay('tools/inventory_lab/gamedata/scripts/sqa3_catalog_items.script',optional=True)
        if item_list is not None:files['gamedata/scripts/sqa3_catalog_items.script']=item_list
        actions_fixture=read_overlay('tools/inventory_lab/gamedata/scripts/sqa3_actions_fixture.script',optional=True)
        if actions_fixture is not None:files['gamedata/scripts/sqa3_actions_fixture.script']=actions_fixture
        definitions_fixture=read_overlay('tools/inventory_lab/gamedata/scripts/sqa3_definition_fixture.script',optional=True)
        if definitions_fixture is not None:files['gamedata/scripts/sqa3_definition_fixture.script']=definitions_fixture
        growth_fixture=read_overlay('tools/inventory_lab/gamedata/scripts/sqa3_growth_policy_fixture.script',optional=True)
        if growth_fixture is not None:files['gamedata/scripts/sqa3_growth_policy_fixture.script']=growth_fixture
        files['gamedata/scripts/sqa3_test_items.script']=read_overlay('tools/inventory_lab/gamedata/scripts/sqa3_test_items.script')
        files['MANUAL-TEST.md']=read_overlay('docs/inventory-next/STAGE1-3-MANUAL-TEST.md')
        parts=read_overlay('tools/inventory_lab/gamedata/configs/items/settings/mod_parts_sqa3_rigs.json',optional=True)
        if parts is not None:
            files['gamedata/configs/items/settings/mod_parts_sqa3_rigs.ltx']=read_overlay('tools/inventory_lab/gamedata/configs/items/settings/mod_parts_sqa3_rigs.ltx')
            files['gamedata/scripts/sqa3_item_actions.script']=read_overlay('tools/inventory_lab/gamedata/scripts/sqa3_item_actions.script')
            files['parts-provenance.json']=parts
    files["Start-Native-Input-Check.ps1"]=read_overlay('tools/inventory_lab/Start-Native-Input-Check.ps1')
    main_launcher=read_overlay('tools/inventory_lab/Start-Squared-Away-3.ps1',optional=True)
    if main_launcher is not None:files["Start-Squared-Away-3.ps1"]=main_launcher
    files["NATIVE-PLACEMENT.md"]=read_overlay('docs/inventory-next/NATIVE-PLACEMENT.md')
    logical=read_overlay('docs/inventory-next/NATIVE-LOGICAL-STORAGE.md',optional=True)
    ground_pickup=False
    owned_pickup=False
    trade_pickup=False
    world_storage=False
    ground_storage=False
    corpse_storage=False
    stash_storage=False
    if logical is not None:
        engine_source=subprocess.check_output(['git','show',a.commit+':src/xrGame/InventoryNextProbe.cpp'],cwd=repo)
        assert b'-sqa3-logical-storage' in engine_source, 'This engine revision has no native logical-storage mode'
        ground_pickup=b'-sqa3-strict-ground-pickup' in engine_source
        owned_pickup=b'-sqa3-strict-owned-pickup' in engine_source
        trade_pickup=b'-sqa3-strict-trade-pickup' in engine_source
        world_storage=b'-sqa3-world-storage' in engine_source
        ground_storage=b'-sqa3-ground-storage' in engine_source
        corpse_storage=b'-sqa3-corpse-storage' in engine_source
        stash_storage=b'-sqa3-stash-storage' in engine_source
        if stash_storage:
            for script in ['item_backpack.script','sqa3_stash.script','sqa3_stash_fixture.script']:
                files['gamedata/scripts/'+script]=read_overlay('tools/inventory_lab/gamedata/scripts/'+script)
        if ground_storage:
            files["gamedata/configs/mod_system_sqa3_ground.ltx"]=read_overlay('tools/inventory_lab/gamedata/configs/mod_system_sqa3_ground.ltx')
        files["NATIVE-LOGICAL-STORAGE.md"]=logical
        launcher=read_overlay('tools/inventory_lab/Start-Logical-Storage-Lab.ps1',optional=True)
        if launcher is not None:files["Start-Logical-Storage-Lab.ps1"]=launcher
    files["README.txt"]=("Squared Away 3 isolated native DEVELOPMENT lab\n\n"
        "For the matching isolated vanilla lab only. Not a public engine release.\n"
        "Run Start-Squared-Away-3.ps1: windowed1280x720, normal main menu, New Game/Load, second-instance guard.\n"
        "Read MANUAL-TEST.md for representative items, controls and remaining acceptance.\n"
        "Explicit test kit: console run_string sqa3_test_items.give() after loading.\n"
        "Fixtures stay inert without -sqa3-fixture and use separate disposable sqa3_* saves.\n"
        "Keep all launcher storage flags when loading its development saves. No SQA2 save migration.\n"
        "Preserve the original test save and one verified rollback. Never install this into the public game.\n"
        "Original Squared Away art; no3D player model. Current build/overlay/CI and every payload hash are in manifest.json.\n"
        "Backpack compartments are new development designs with original usable-cell totals preserved; original rig masks retained.\n"
        "Growth stages are provisional development balancing; external modpack action bridges and long-session parity are later work.\n"
        "MCM 1.7.2fix is an external dependency for the growth-collision setting. Install it separately; this ZIP includes only Squared Away's MCM entry. Default without MCM is refuse.\n"
        "In MCM: Squared Away 3 > When a filled container cannot grow > Refuse the item / Move container to Recovery.\n").encode()
    manifest={"commit":a.commit,"engine_commit":a.commit,"overlay_commit":overlay,"ci":a.ci,"purpose":"isolated native placement development","files":{k:{"sha256":hashlib.sha256(v).hexdigest(),"bytes":len(v)} for k,v in files.items()}}
    manifest['field_kit_opt_in']=b'-sqa3-field-kit' in engine_source
    manifest['logical_storage_opt_in']=logical is not None
    manifest['ground_pickup_opt_in']=ground_pickup
    manifest['owned_pickup_opt_in']=owned_pickup
    manifest['trade_pickup_opt_in']=trade_pickup
    manifest['world_storage_opt_in']=world_storage
    manifest['ground_storage_opt_in']=ground_storage
    manifest['corpse_storage_opt_in']=corpse_storage
    manifest['stash_storage_opt_in']=stash_storage
    files["manifest.json"]=(json.dumps(manifest,indent=2)+"\n").encode()
    a.output.parent.mkdir(parents=True,exist_ok=True)
    with zipfile.ZipFile(a.output,"w",zipfile.ZIP_DEFLATED,compresslevel=6) as z:
        for name,data in files.items():z.writestr(name,data)
    with zipfile.ZipFile(a.output) as z:
        assert z.testzip() is None
        for name,entry in manifest["files"].items():assert hashlib.sha256(z.read(name)).hexdigest()==entry["sha256"]
    print(json.dumps({"path":str(a.output),"bytes":a.output.stat().st_size,"sha256":hashlib.sha256(a.output.read_bytes()).hexdigest(),"payloads_verified":len(manifest["files"])},indent=2))
if __name__=="__main__":main()
