"""Package one verified CI engine and minimal SQA visual overlay for the isolated lab."""
import argparse,hashlib,json,zipfile
from pathlib import Path

def main():
    ap=argparse.ArgumentParser();ap.add_argument("--engine",type=Path,required=True);ap.add_argument("--engine-sha256",required=True);ap.add_argument("--commit",required=True);ap.add_argument("--ci",required=True);ap.add_argument("--output",type=Path,required=True);a=ap.parse_args()
    source=Path(__file__).parent
    engine=a.engine.read_bytes();assert hashlib.sha256(engine).hexdigest()==a.engine_sha256
    atlas=json.loads((source/"native-visual-assets.json").read_text());texture=Path(atlas["source"]).read_bytes()
    assert len(texture)==atlas["bytes"] and hashlib.sha256(texture).hexdigest()==atlas["sha256"]
    files={"bin/AnomalyDX11AVX.exe":engine,atlas["destination"]:texture}
    for rel in ["configs/ui/textures_descr/sqa3_native.xml","scripts/sqa3_native_autorun.script"]:
        files["gamedata/"+rel]=(source/"gamedata"/rel).read_bytes()
    files["Start-Native-Input-Check.ps1"]=(source/"Start-Native-Input-Check.ps1").read_bytes()
    files["NATIVE-PLACEMENT.md"]=(source.parents[1]/"docs/inventory-next/NATIVE-PLACEMENT.md").read_bytes()
    files["README.txt"]=("Squared Away 3 native placement DEVELOPMENT lab\n\n"+
        "Only for the isolated vanilla lab with matching upstream2026.10.11 data overlay.\n"+
        "Not a complete playable mod or public engine release. Preserve test save and one rollback.\n"+
        "Start windowed with -dbg -inventory-lab. Console: inventory_next_probe view.\n"+
        "Fixture is inert without -sqa3-fixture; it writes only separate sqa3_layout saves.\n"+
        "Ownership transfers, final pockets, equipment, gameplay parity and physical input remain unverified.\n").encode()
    manifest={"commit":a.commit,"ci":a.ci,"purpose":"isolated native placement development","files":{k:{"sha256":hashlib.sha256(v).hexdigest(),"bytes":len(v)} for k,v in files.items()}}
    files["manifest.json"]=(json.dumps(manifest,indent=2)+"\n").encode()
    a.output.parent.mkdir(parents=True,exist_ok=True)
    with zipfile.ZipFile(a.output,"w",zipfile.ZIP_DEFLATED,compresslevel=6) as z:
        for name,data in files.items():z.writestr(name,data)
    with zipfile.ZipFile(a.output) as z:
        assert z.testzip() is None
        for name,entry in manifest["files"].items():assert hashlib.sha256(z.read(name)).hexdigest()==entry["sha256"]
    print(json.dumps({"path":str(a.output),"bytes":a.output.stat().st_size,"sha256":hashlib.sha256(a.output.read_bytes()).hexdigest(),"payloads_verified":len(manifest["files"])},indent=2))
if __name__=="__main__":main()
