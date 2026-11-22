#!/usr/bin/env python3
"""Preserve SDK's duplicate version keys while assigning all twenty panels to six MPC function keys.
The same key cycles its subpages; Q-Link maps follow key/subpage, never the old 20-key indexing.
"""
import json
import pathlib
import sys
if len(sys.argv)!=2: raise SystemExit("Usage: group_skin_tabs.py <Plugin Skins directory>")
skins=pathlib.Path(sys.argv[1])
sys.path.insert(0,str((skins.parents[4]/"tools").resolve()))
from gen_ui import Obj, dump_json
def read(path):
    return json.loads(path.read_text(encoding="utf-8"),object_pairs_hook=Obj)
def get(ob,key):
    return next(v for k,v in ob if k==key)
def put(ob,key,value):
    for i,(k,v) in enumerate(ob):
        if k==key:
            ob[i]=(key,value)
            return
    raise ValueError("Missing expected key "+key)
groups=(
    ("GENESIS","OSC MOD","TUNING","UNISON"),
    ("FILTER","AMP ENV","MOD ENV"),
    ("CHORUS","DELAY","REVERB"),
    ("PERFORM","ARP"),
    ("MATRIX 1","MATRIX 2","MATRIX 3","MATRIX 4"),
    ("MATRIX 5","MATRIX 6","MATRIX 7","MATRIX 8"),
)
mapping={(name):(key,sub) for key,grp in enumerate(groups) for sub,name in enumerate(grp)}
assert len(mapping)==20
tuip=skins/"TUI.json"; qp=skins/"Q-Links.json"
tui=read(tuip); q=read(qp)
tabs=get(get(tui,"pageData"),"tabs")
qlinks=get(get(q,"Screen Mode Q-Links"),"map")
assert len(tabs)==len(qlinks)==20
seen=set()
for tab,qlink in zip(tabs,qlinks):
    name=get(tab,"tabName")
    assert name in mapping and name not in seen,name
    seen.add(name)
    key,sub=mapping[name]
    put(tab,"fnKeyIndex",key)
    put(tab,"fnKeySubIndex",sub)
    put(qlink,"Tab",key+1)
    put(qlink,"SubTab",sub+1)
assert seen==set(mapping)
tuip.write_text(dump_json(tui)+"\n",encoding="utf-8")
qp.write_text(dump_json(q)+"\n",encoding="utf-8")
print("GENESIS: 20 touchscreen panels across 6 MPC function keys; Q-Links remapped.")
