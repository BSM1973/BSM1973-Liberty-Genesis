#!/usr/bin/env python3
"""Fail the package when panel artwork or touchscreen layout violates the 1280x628 MPC design."""
import json,pathlib,sys
from PIL import Image
ui=json.loads(pathlib.Path(sys.argv[1]).read_text(encoding="utf-8"))
assets=pathlib.Path(sys.argv[2])
assert len(ui["pages"])==20
assert ui["qlinkGroups"]=="rows"
seen=set()
for page in ui["pages"]:
    assert len(page["name"])<=8,page["name"]
    assert page["name"] not in seen
    seen.add(page["name"])
    assert 0<len(page["controls"])<=16
    bg=assets/page["background"]
    assert bg.is_file(),bg
    with Image.open(bg) as im:
        assert im.size==(1280,628),(bg,im.size)
        assert im.mode in ("RGB","RGBA")
        assert im.getpixel((200,12))!=im.getpixel((200,200))
    for control in page["controls"]:
        for val in control.values():
            if isinstance(val,str):
                assert not any(t in val for t in ("\ufffd","Ã","Â","â€")),val
for page in ui["pages"]:
    for ctl in page["controls"]:
        name=ctl.get("param","")
        if name=="Delay Sync" or (name.startswith("M") and (name.endswith(" Source") or name.endswith(" Dest"))):
            assert ctl.get("widget")!="switch",(page["name"],name)
assert len(ui["pages"][0]["controls"])==16
assert ui["pages"][0]["name"]=="GENESIS"
print("GENESIS UI checked: 20 art panels, correct full-size assets, 16 main controls, no broken references.")
