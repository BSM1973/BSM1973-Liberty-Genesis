#!/usr/bin/env python3
"""Replace the generic 4x4 Genesis page with sixteen real touch controls
laid out as a two-deck analog instrument. Reuse SDK binding, overlay and
Q-Link primitives; no audio code or parameter IDs are changed.
"""
import pathlib,sys,shutil,json
if len(sys.argv)!=2:raise SystemExit("Usage: apply_analog_main.py <Plugin Skins folder>")
skins=pathlib.Path(sys.argv[1]).resolve()
sys.path.insert(0,str(skins.parents[4]/"tools"))
import gen_ui as g
from gen_ui import Obj,dump_json

def read(path):
    return json.loads(path.read_text(encoding="utf-8"),object_pairs_hook=Obj)
def get(ob,key):
    return next(v for k,v in ob if k==key)
def put(ob,key,value):
    for i,(k,v) in enumerate(ob):
        if k==key:ob[i]=(key,value);return
    raise ValueError("Missing "+key)

ui=read(skins/"TUI.json")
page=get(ui,"pageData")
tabs=get(page,"tabs")
main=next(t for t in tabs if get(t,"tabName")=="GENESIS")
root=get(main,"componentDefinition")
old=get(root,"componentsData")
order=["Wave A","Wave B","Mix","Sub Level","Cutoff","Resonance","Filter Mode","Filter Drive",
       "LFO Rate","LFO Depth","Chorus Mix","Delay Mix","Volume","Detune","Attack","Arp On"]
bindings={}
for ctl in old:
    data=get(ctl,"componentData")
    name=get(data,"name")
    if name in order:
        maps=get(get(ctl,"handle remapping"),"map")
        bindings[name]=get(maps[0],"value")
assert list(bindings)==order,(list(bindings),order)
defs=get(get(page,"componentDefinitions"),"localComponentDefinitions")

def defn(name,kind):
    label=g.entry("name","Label",
        g.label_data("Name","ffe5dccc",18.0,"horizontallyCentred"),
        "4 9 131 28","No","Show","Always")
    if kind in ("knob","menu"):
        widget=g.entry("knob","Knob",g.knob_filmstrip_data("knob_uni.png",127),
                       "24 46 90 90","No","Show","Always")
        value_y=150
    elif kind=="fader":
        widget=g.entry("fader","Knob",g.knob_filmstrip_data("fader_v_uni.png",127,"Vertical"),
                       "51 41 36 160","No","Show","Always")
        value_y=205
    else:
        widget=g.entry("switch","Button",g.button_data("toggle_on.png","toggle_off.png",1,1),
                       "37 83 64 32","No","Show","Always")
        value_y=145
    value=g.entry("value","Label",
        g.label_data("Value","fff7e9cf",20.0,"horizontallyCentred"),
        "2 %d 135 30"%value_y,"No","Show","Always")
    children=[label,widget,value]
    overlay="menu overlay" if kind=="menu" else "knob overlay"
    component=g.component_definition(g.control_actions(overlay,double_click=(kind!="switch")),children,"00000000")
    defs.append(g.obj(("key",name),("value",component)))

for name in ("genAnalogKnob","genAnalogMenu","genAnalogFader","genAnalogToggle"):
    kind={"genAnalogKnob":"knob","genAnalogMenu":"menu",
          "genAnalogFader":"fader","genAnalogToggle":"switch"}[name]
    defn(name,kind)
params_choices={"Wave A","Wave B","Filter Mode"}
params_faders={"Mix","Sub Level","LFO Depth","Chorus Mix","Delay Mix","Volume","Attack"}
controls=[]
for i,name in enumerate(order):
    kind=("genAnalogMenu" if name in params_choices else
          "genAnalogFader" if name in params_faders else
          "genAnalogToggle" if name=="Arp On" else "genAnalogKnob")
    x=24+(i%8)*156
    y=54+(i//8)*296
    entry=g.entry(name,kind,g.obj(("version",1),("handleName","Data")),
        f"{x} {y} 139 242","Yes","Hide","Always",
        remap=[("Data",bindings[name])])
    controls.append(entry)
put(root,"componentsData",controls)
# Four hardware banks match four consecutive blocks of four horizontal modules.
put(main,"qlinkBoundsData",[
    "14 44 627 262","638 44 627 262",
    "14 340 627 262","638 340 627 262"])
# Vertical asset is copied from the exact kit cached by gen_ui for this plugin.
cached=list(skins.parents[1].glob("ui-kit-*/fader_v_uni.png"))
assert len(cached)==1,cached
shutil.copy2(cached[0],skins/"fader_v_uni.png")
(skins/"TUI.json").write_text(dump_json(ui)+"\n",encoding="utf-8")
assert len(get(root,"componentsData"))==16
assert (skins/"fader_v_uni.png").is_file()
print("GENESIS: custom analog touch panel: 8+8 physical modules; 16 bound controls; 4 correct Q-Link banks.")
