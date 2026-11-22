#!/usr/bin/env python3
"""Transform generated MPC skins into functional, accessible, non-empty analog panels.
Preserves all 129 parameter handles, Q-Link mappings and duplicate overlay version keys.
"""
import json,pathlib,sys
if len(sys.argv)!=2:raise SystemExit("Usage: apply_analog_main.py <Plugin Skins>")
skins=pathlib.Path(sys.argv[1]).resolve()
sys.path.insert(0,str(skins.parents[4]/"tools"))
import gen_ui as g
from gen_ui import Obj,dump_json
def read(p):return json.loads(p.read_text(encoding="utf-8"),object_pairs_hook=Obj)
def get(ob,k):return next(v for key,v in ob if key==k)
def put(ob,k,v):
    for i,(key,old) in enumerate(ob):
        if key==k:ob[i]=(key,v);return
    raise AssertionError(k)
spec=json.loads((skins.parents[2]/"ui.json").read_text(encoding="utf-8"))
tui=read(skins/"TUI.json")
page=get(tui,"pageData")
tabs={get(t,"tabName"):t for t in get(page,"tabs")}
defs=get(get(page,"componentDefinitions"),"localComponentDefinitions")
def layout(n,main):
    if main:
        assert n==16
        return [(23+(i%4)*320,(45,196,346,497)[i//4],278,107) for i in range(n)]
    out=[]
    for row,count in enumerate((min(4,n),max(0,n-4))):
        if count:
            x0={1:503,2:343,3:183,4:23}[count]
            y=174 if n<=4 else (65 if row==0 else 351)
            out.extend((x0+i*320,y,278,228) for i in range(count))
    return out
def lab(id,colour,size,rect,align="left"):
    return g.entry(id,"Label",g.label_data(id,colour,size,align),
                   rect,"No","Show","Always")
def make_definition(name,kind,main):
    if main:
        if kind in ("knob","menu"):
            children=[lab("Name","ffe9e0d0",25,"104 9 171 37"),
              g.entry("knob","Knob",g.knob_filmstrip_data("knob_uni.png",127),
                      "7 8 90 90","No","Show","Always"),
              lab("Value","ffffedce",27,"104 58 170 38")]
        elif kind=="fader":
            children=[lab("Name","ffe9e0d0",25,"13 6 154 39"),
              lab("Value","ffffedce",27,"168 6 100 39","right"),
              g.entry("fader","Knob",g.knob_filmstrip_data("fader_h_uni.png",127,"Horizontal"),
                      "15 57 248 36","No","Show","Always")]
        else:
            children=[lab("Name","ffe9e0d0",25,"13 10 170 38"),
              lab("Value","ffffedce",27,"13 54 168 38"),
              g.entry("switch","Button",g.button_data("toggle_on.png","toggle_off.png",1,1),
                      "191 37 64 32","No","Show","Always")]
    else:
        if kind in ("knob","menu"):
            children=[lab("Name","ffe9e0d0",29,"13 16 250 43","horizontallyCentred"),
              g.entry("knob","Knob",g.knob_filmstrip_data("knob_uni.png",127),
                      "94 66 90 90","No","Show","Always"),
              lab("Value","ffffedce",32,"16 165 245 45","horizontallyCentred")]
        elif kind=="fader":
            children=[lab("Name","ffe9e0d0",29,"13 17 250 43","horizontallyCentred"),
              g.entry("fader","Knob",g.knob_filmstrip_data("fader_h_uni.png",127,"Horizontal"),
                      "15 108 248 36","No","Show","Always"),
              lab("Value","ffffedce",32,"16 162 245 45","horizontallyCentred")]
        else:
            children=[lab("Name","ffe9e0d0",29,"13 17 250 43","horizontallyCentred"),
              g.entry("switch","Button",g.button_data("toggle_on.png","toggle_off.png",1,1),
                      "106 87 64 32","No","Show","Always"),
              lab("Value","ffffedce",32,"16 165 245 45","horizontallyCentred")]
    overlay="menu overlay" if kind=="menu" else "knob overlay"
    comp=g.component_definition(g.control_actions(overlay,double_click=(kind!="toggle")),children,"00000000")
    defs.append(g.obj(("key",name),("value",comp)))
for main in (True,False):
    for kind in ("knob","menu","fader","toggle"):
        make_definition(("genMain" if main else "genDetail")+kind.capitalize(),kind,main)
handles=set()
for p in spec["pages"]:
    name=p["name"];tab=tabs[name];root=get(tab,"componentDefinition")
    old=get(root,"componentsData")
    controls=p["controls"];names={c["param"] for c in controls}
    mapping={}
    for ctrl in old:
        ctrlname=get(get(ctrl,"componentData"),"name")
        if ctrlname in names:
            m=get(get(ctrl,"handle remapping"),"map")
            assert len(m)==1
            mapping[ctrlname]=get(m[0],"value")
    assert set(mapping)==names,(name,set(mapping),names)
    main=name=="GENESIS";positions=layout(len(controls),main)
    new=[]
    for c,(x,y,w,h) in zip(controls,positions):
        pname=c["param"];widget=c["widget"]
        is_toggle=widget=="switch" and (pname in {"Arp On","Arp Hold","Osc Sync","Uni CPU"} or pname.endswith(" Polarity"))
        kind="fader" if widget=="fader" else "toggle" if is_toggle else "menu" if widget=="switch" or c.get("overlay")=="menu" else "knob"
        definition=("genMain" if main else "genDetail")+kind.capitalize()
        new.append(g.entry(pname,definition,g.obj(("version",1),("handleName","Data")),
                  f"{x} {y} {w} {h}","Yes","Hide","Always",remap=[("Data",mapping[pname])]))
        handles.add(mapping[pname])
    put(root,"componentsData",new)
    rectangles=[]
    for i in range(0,len(positions),4):
        chunk=positions[i:i+4]
        x0=min(x for x,y,w,h in chunk)-10;y0=min(y for x,y,w,h in chunk)-10
        x1=max(x+w for x,y,w,h in chunk)+10;y1=max(y+h for x,y,w,h in chunk)+10
        assert 0<=x0<x1<=1280 and 0<=y0<y1<=628
        rectangles.append(f"{x0} {y0} {x1-x0} {y1-y0}")
    put(tab,"qlinkBoundsData",rectangles)
assert len(handles)==129,(len(handles),handles)
# MPC firmware supplies only small sub-tab position indicators. The actual large
# function keys are the supported touch targets: give each of the two matrix
# banks a stable, descriptive large caption regardless of current sub-page.
for n in range(1,9):
    tab=tabs[f"MATRIX {n}"]
    label="M1-4" if n<=4 else "M5-8"
    assert get(tab,"fnKeyIndex")==(4 if n<=4 else 5)
    assert get(tab,"fnKeySubIndex")==((n-1)%4)
    put(tab,"tabName",label)
(skins/"TUI.json").write_text(dump_json(tui)+"\n",encoding="utf-8")
print("Genesis Analog UI 03: 20 touch panels, 129 params, large M1-4 / M5-8 cycling keys.")
