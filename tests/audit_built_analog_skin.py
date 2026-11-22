#!/usr/bin/env python3
"""Assert the generated MPC TUI really uses a handcrafted 8x2 panel, not the old 4x4 template."""
import json,sys,pathlib
if len(sys.argv)!=2:raise SystemExit("Usage: audit_built_analog_skin.py <Plugin Skins folder>")
skin=pathlib.Path(sys.argv[1])
tui=json.loads((skin/"TUI.json").read_text(encoding="utf-8"))
page=tui["pageData"]
tabs=page["tabs"]
assert len(tabs)==20
assert len({(t["fnKeyIndex"],t["fnKeySubIndex"]) for t in tabs})==20
assert all(0<=t["fnKeyIndex"]<=5 for t in tabs)
main=next(t for t in tabs if t["tabName"]=="GENESIS")
controls=main["componentDefinition"]["componentsData"]
assert len(controls)==16,len(controls)
assert all(c["componentData"]["type"].startswith("genAnalog") for c in controls)
bounds=[tuple(int(v) for v in c["bounds"]["bounds"].split()) for c in controls]
assert len({b[0] for b in bounds[:8]})==8
assert all(b[1]==54 for b in bounds[:8])
assert all(b[1]==350 for b in bounds[8:])
assert all(0<=x and x+w<=1280 and 0<=y and y+h<=628 for x,y,w,h in bounds)
assert main["qlinkBoundsData"]==["14 44 627 262","638 44 627 262","14 340 627 262","638 340 627 262"]
defs={entry["key"] for entry in page["componentDefinitions"]["localComponentDefinitions"]}
assert {"genAnalogKnob","genAnalogMenu","genAnalogFader","genAnalogToggle"}.issubset(defs)
for f in ("genesis-panel-genesis.png","fader_v_uni.png","knob_uni.png","TUI.json","Q-Links.json"):
    assert (skin/f).is_file(),f
print("GENESIS revision 02 PASSED: real 16-control 8x2 touch GUI, 4 hardware banks and skin assets present.")
