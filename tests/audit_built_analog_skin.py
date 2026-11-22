#!/usr/bin/env python3
"""Gate native release on all 20 live pages and accessibility of active controls."""
import json,pathlib,sys
skins=pathlib.Path(sys.argv[1])
tui=json.loads((skins/"TUI.json").read_text(encoding="utf-8"))
spec=json.loads((skins.parents[2]/"ui.json").read_text(encoding="utf-8"))
all_tabs=tui["pageData"]["tabs"]
# Background assets retain unique stable identities even when four consecutive
# sub-tabs deliberately share a larger, easily hit MATRIX function-key title.
tabs={t["componentDefinition"]["backgroundData"]["unfocussed"]["image"]:t for t in all_tabs}
defs={d["key"]:d["value"] for d in tui["pageData"]["componentDefinitions"]["localComponentDefinitions"]}
assert len(tabs)==20 and len({(t["fnKeyIndex"],t["fnKeySubIndex"]) for t in all_tabs})==20
mapped=set()
for p in spec["pages"]:
    tab=tabs[p["background"]];c=tab["componentDefinition"]["componentsData"]
    if p["name"].startswith("MATRIX "):
        n=int(p["name"].split()[1])
        assert tab["tabName"]==("M1-4" if n<=4 else "M5-8")
        assert tab["fnKeyIndex"]==(4 if n<=4 else 5)
        assert tab["fnKeySubIndex"]==(n-1)%4
    else:
        assert tab["tabName"]==p["name"]
    assert len(c)==len(p["controls"]),(p["name"],len(c),len(p["controls"]))
    assert len(tab["qlinkBoundsData"])==(len(c)+3)//4
    rects=[]
    for source,ctrl in zip(p["controls"],c):
        assert ctrl["componentData"]["name"]==source["param"]
        key=ctrl["componentData"]["type"]
        assert key.startswith("genMain" if p["name"]=="GENESIS" else "genDetail")
        names=[x for x in defs[key]["componentsData"] if x["componentData"]["name"]=="Name"]
        assert len(names)==1
        assert names[0]["componentData"]["data"]["textStyle"]["font"]["height"]>=25
        binding=ctrl["handle remapping"]["map"]
        assert len(binding)==1
        mapped.add(binding[0]["value"])
        x,y,w,h=map(int,ctrl["bounds"]["bounds"].split())
        assert x>=0 and y>=0 and x+w<=1280 and y+h<=628
        rects.append((x,y,x+w,y+h))
    for i,a in enumerate(rects):
        for b in rects[i+1:]:
            assert a[2]<=b[0] or b[2]<=a[0] or a[3]<=b[1] or b[3]<=a[1],(p["name"],a,b)
    for i,r in enumerate(tab["qlinkBoundsData"]):
        x,y,w,h=map(int,r.split())
        for bx,by,bx2,by2 in rects[i*4:i*4+4]:
            assert x<=bx and y<=by and bx2<=x+w and by2<=y+h
    assert (skins/p["background"]).is_file()
assert len(mapped)==129,len(mapped)
for asset in ("knob_uni.png","fader_h_uni.png","TUI.json","Q-Links.json"):
    assert (skins/asset).is_file(),asset
print("GENESIS UI 03 PASSED: all 20 accessible pages, 129 params, 8 MATRIX subpages using large bank keys.")
