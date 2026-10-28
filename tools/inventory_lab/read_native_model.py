"""Read all schema6 variable masks, mounts and per-instance metadata strictly."""
import hashlib
from pathlib import Path
import zlib


def read_all(path):
    raw=Path(path).read_bytes();text=raw.decode('utf-8').replace('\r\n','\n')
    payload,footer=text.rsplit('CRC32 ',1)
    assert zlib.crc32(payload.encode())==int(footer.strip(),16),'model checksum'
    rows=payload.splitlines();header=rows.pop(0).split()
    assert header[:2]==['INVENTORY_NEXT','6'] and len(rows)==int(header[3]),'model schema/count'
    nodes={}
    for row in rows:
        f=row.split();identity=int(f[0]);assert identity not in nodes,'duplicate persistent identity'
        cursor=33
        def number():
            nonlocal cursor
            value=int(f[cursor]);cursor+=1;return value
        cells=[number() for _ in range(number())]
        names={}
        for _ in range(number()):
            region=number();assert region not in names;names[region]=f[cursor];cursor+=1
        mounts=[]
        for _ in range(number()):
            slot=number();name=f[cursor];cursor+=1
            mounts.append(dict(id=slot,name=name,width=number(),height=number(),accepts=number(),minimum_condition=number(),enabled=number()))
        pouch,mounted_slot,condition,max_stack=number(),number(),number(),number()
        attrs={}
        for _ in range(number()):
            key=f[cursor];encoded=f[cursor+1];cursor+=2
            assert key not in attrs and encoded.startswith('x'),'invalid native metadata'
            attrs[key]=bytes.fromhex(encoded[1:]).decode('utf-8')
        assert cursor==len(f),'unexpected model trailing fields'
        nodes[identity]=dict(parent=int(f[1]),kind=int(f[2]),grams=int(f[3]),category=int(f[4]),
            position=list(map(int,f[5:8])),empty=list(map(int,f[8:10])),half=list(map(int,f[10:12])),full=list(map(int,f[12:14])),
            container=int(f[14]),soft=int(f[15]),grid=list(map(int,f[16:18])),
            location=int(f[24]),equipped_slots=int(f[27]),cells=cells,compartment_names=names,mounts=mounts,
            pouch=pouch,mounted_slot=mounted_slot,condition=condition,max_stack_quantity=max_stack,attributes=attrs)
    return nodes,hashlib.sha256(raw).hexdigest()


def read(path):
    nodes,digest=read_all(path);items={identity:node for identity,node in nodes.items() if node['kind']==0}
    for node in items.values():
        assert all(key in node['attributes'] for key in ['native_section','native_live_id','native_place','native_slot']),'missing native facts'
    return items,digest
