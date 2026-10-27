"""Verify captured native exports from the bounded three-item placement fixture.
This verifies evidence files, not physical UI interaction or full inventory parity.
"""
import argparse, hashlib, json, zlib
from pathlib import Path

def read(path):
    raw=path.read_bytes()
    # Engine save_atomic writes canonical LF; accept filesystem text normalization.
    text=raw.decode().replace("\r\n","\n")
    payload,footer=text.rsplit("CRC32 ",1)
    assert zlib.crc32(payload.encode()) == int(footer.strip(),16), "export checksum"
    rows=payload.splitlines();header=rows.pop(0).split()
    assert header[:2]==["INVENTORY_NEXT","6"], "schema"
    nodes={}
    for row in rows:
        f=row.split();identity=int(f[0]);assert identity not in nodes
        if int(f[2]) != 0:continue
        attrs={}
        for key in ["native_section","native_live_id","native_place","native_slot"]:
            i=f.index(key);assert f[i+1].startswith("x")
            attrs[key]=bytes.fromhex(f[i+1][1:]).decode()
        nodes[identity]={"parent":int(f[1]),"grams":int(f[3]),"position":list(map(int,f[5:8])),"condition":int(f[38]),"attributes":attrs}
    assert len(rows)==int(header[3]), "declared count"
    return nodes,hashlib.sha256(raw).hexdigest()

def main():
    ap=argparse.ArgumentParser();ap.add_argument("edited",type=Path);ap.add_argument("reloaded",type=Path);ap.add_argument("--output",type=Path,required=True);a=ap.parse_args()
    first,h1=read(a.edited);second,h2=read(a.reloaded)
    assert set(first)==set(second)=={100,101,102}, "lost or duplicate identity"
    assert [first[x]["attributes"]["native_section"] for x in [100,101,102]]==["device_pda_1","bandage","medkit"]
    assert first[100]["position"]==[4,3,0], "accepted move not persisted"
    assert first[101]["position"]!=[4,3,0], "overlap not rejected"
    assert sum(x["grams"] for x in first.values())==638, "fixture mass"
    assert all(x["parent"]==1 for x in first.values()), "ownership changed"
    assert first==second, "restart changed item identity, placement, mass or native state"
    report={"status":"passed","count":3,"grams":638,"edited_sha256":h1,"reloaded_sha256":h2,"items":second,"limits":"Domain commands and native persistence only; physical mouse and gameplay parity unverified."}
    a.output.write_text(json.dumps(report,indent=2)+"\n");print(json.dumps(report,indent=2))
if __name__=="__main__":main()
