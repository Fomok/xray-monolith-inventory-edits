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
    for rel in ["configs/sqa3_capacity.ltx","configs/ui/textures_descr/sqa3_native.xml","configs/ui/sqa3_actor_menu.xml","scripts/sqa3_native_autorun.script","scripts/ui_inventory.script"]:
        data=read_overlay('tools/inventory_lab/gamedata/'+rel,optional=rel=='configs/sqa3_capacity.ltx')
        if data is not None:files["gamedata/"+rel]=data
    files["Start-Native-Input-Check.ps1"]=read_overlay('tools/inventory_lab/Start-Native-Input-Check.ps1')
    files["NATIVE-PLACEMENT.md"]=read_overlay('docs/inventory-next/NATIVE-PLACEMENT.md')
    files["README.txt"]=("Squared Away 3 native placement DEVELOPMENT lab\n\n"+
        "Only for the isolated vanilla lab with matching upstream2026.10.11 data overlay.\n"+
        "Not a complete playable mod or public engine release. Preserve test save and one rollback.\n"+
        "Start windowed with -dbg -inventory-lab. Use the normal inventory key; one replacement menu also handles loot/trade.\n"+
        "Fixture is inert without -sqa3-fixture; it writes only separate sqa3_* test saves.\n"+
        "No3D player model. Final pockets, full native recovery and complete gameplay parity remain unfinished.\n").encode()
    manifest={"commit":a.commit,"engine_commit":a.commit,"overlay_commit":overlay,"ci":a.ci,"purpose":"isolated native placement development","files":{k:{"sha256":hashlib.sha256(v).hexdigest(),"bytes":len(v)} for k,v in files.items()}}
    files["manifest.json"]=(json.dumps(manifest,indent=2)+"\n").encode()
    a.output.parent.mkdir(parents=True,exist_ok=True)
    with zipfile.ZipFile(a.output,"w",zipfile.ZIP_DEFLATED,compresslevel=6) as z:
        for name,data in files.items():z.writestr(name,data)
    with zipfile.ZipFile(a.output) as z:
        assert z.testzip() is None
        for name,entry in manifest["files"].items():assert hashlib.sha256(z.read(name)).hexdigest()==entry["sha256"]
    print(json.dumps({"path":str(a.output),"bytes":a.output.stat().st_size,"sha256":hashlib.sha256(a.output.read_bytes()).hexdigest(),"payloads_verified":len(manifest["files"])},indent=2))
if __name__=="__main__":main()
