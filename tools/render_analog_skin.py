#!/usr/bin/env python3
"""Render original Genesis analog front-panel artwork INTO the MPC plugin package.
No external imagery; the user-facing output remains a functional touch plugin.
The drawings are static hardware panels, not simulated live states.
"""
import json
import math
import pathlib
import random
import sys
from PIL import Image, ImageDraw, ImageFont

WIDTH, HEIGHT = 1280, 628
SLOT_X = (23, 343, 663, 983)
SLOT_Y = (45, 196, 346, 497)
ROW_STARTS = (0, 151, 301, 452)
# Studio analogue instrument: oxidised red, cold steel blue, black anodised,
# bronze accents, gently grained wooden cheeks.
BARS = ((151, 44, 49), (45, 88, 114), (127, 45, 45), (48, 83, 110))
PALETTE = {
    "back": (14, 16, 19), "panel": (25, 26, 28),
    "light": (79, 82, 82), "text": (245, 237, 224),
    "muted": (184, 178, 164), "bronze": (241, 173, 51),
}

def font(size, bold=False):
    files = (
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf" if bold
        else "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation2/LiberationSans-Bold.ttf" if bold
        else "/usr/share/fonts/truetype/liberation2/LiberationSans-Regular.ttf",
    )
    for file in files:
        try:
            return ImageFont.truetype(file, size)
        except OSError:
            pass
    return ImageFont.load_default()

def label(draw, x, y, text, size=13, fill=None, bold=False, max_width=220):
    text = str(text).upper()
    size = min(size, 17)
    f = font(size, bold)
    while size > 9 and draw.textbbox((0, 0), text, font=f)[2] > max_width:
        size -= 1
        f = font(size, bold)
    draw.text((x, y), text, font=f, fill=fill or PALETTE["text"])

def shade(draw, top, bottom, rgb1, rgb2, x0=18, x1=1262):
    size = max(1, bottom - top)
    for j in range(size):
        t = j / size
        col = tuple(round(a * (1-t) + b*t) for a, b in zip(rgb1, rgb2))
        draw.line(((x0, top+j), (x1, top+j)), fill=col)

def screw(draw, x, y):
    draw.ellipse((x-4, y-4, x+4, y+4), fill=(8, 9, 11), outline=(103, 98, 87), width=1)
    draw.line((x-2, y+1, x+2, y-1), fill=(147, 140, 125), width=1)

def wood(draw, rng):
    # Two slim solid walnut cheeks frame the four-column tactile MPC layout.
    for x0, x1 in ((0, 17), (1263, 1279)):
        for x in range(x0, x1+1):
            t=(x-x0)/(max(1,x1-x0))
            c=(49+round(16*math.sin(t*4)), 28+round(9*math.sin(t*3)), 18+round(5*math.sin(t*3)))
            draw.line((x, 0, x, HEIGHT), fill=c)
        for i in range(48):
            x=rng.randint(x0+2, x1-2)
            y=rng.randrange(HEIGHT)
            dy=rng.randint(10,90)
            draw.line((x,y,x+round(math.sin(i)*1),min(HEIGHT-1,y+dy)),
                      fill=(72, 43, 27) if i%3 else (34, 20, 14), width=1)
    for x in (18,1262):
        draw.line((x,0,x,HEIGHT),fill=(166,111,50),width=2)
        draw.line((x+2 if x==18 else x-2,0,x+2 if x==18 else x-2,HEIGHT),
                  fill=(8,9,10),width=2)

def backgrounds(page, page_no, out_dir):
    rng=random.Random(701 + page_no * 193)
    img=Image.new("RGB",(WIDTH,HEIGHT),PALETTE["back"])
    draw=ImageDraw.Draw(img)
    # Anodised chassis: fine hand-tuned horizontal brushed-metal texture.
    for y in range(HEIGHT):
        v=int(2*math.sin(y*.51)+rng.uniform(-1,1))
        draw.line((19,y,1261,y),fill=(21+v,22+v,24+v))
    for row, start in enumerate(ROW_STARTS):
        end = ROW_STARTS[row+1]-1 if row < 3 else HEIGHT-1
        shade(draw,start,end,(34,35,37),(18,19,21),19,1261)
        draw.line((19,end,1261,end),fill=(101,101,97),width=1)
        draw.line((19,end-2,1261,end-2),fill=(7,8,10),width=1)
        # Burgundy / steel-blue panel bands, with restrained satin shading.
        y1,y2=start+9,start+33
        bar=BARS[row]
        shade(draw,y1,y2,tuple(min(255,k+24) for k in bar),bar,24,1257)
        draw.line((24,y1,1257,y1),fill=tuple(min(255,k+63) for k in bar))
        draw.line((24,y2,1257,y2),fill=(5,8,11),width=2)
        # Each module is independently framed, with tactile breathing room.
        for col,x in enumerate(SLOT_X):
            y=SLOT_Y[row]
            draw.rounded_rectangle((x-4,y-5,x+284,y+112),radius=8,
                                   fill=(18,19,21),outline=(67,67,67),width=1)
            draw.line((x+4,y+107,x+274,y+107),fill=(6,7,8),width=2)
            draw.line((x+6,y-2,x+271,y-2),fill=(58,58,56),width=1)
            draw.rectangle((x+2,y+2,x+5,y+105),fill=(34,35,36))
        # Engraved area heading in band, left-aligned to wood cheek.
        header=("GENESIS // " if page["name"]=="GENESIS" and row==0 else
                page["name"] + " // ")
        sections=section_names(page["name"],row)
        label(draw,31,start+12,header+"    "+sections,12,PALETTE["text"],True,max_width=1060)
        label(draw,1126,start+13,"LIBERTY",11,(252,222,169),True,max_width=100)
        screw(draw,1244,start+21)
        screw(draw,1078,start+21)
    wood(draw,rng)
    # Studio-series identity; unobtrusive since every control must remain editable.
    label(draw,25,HEIGHT-16,"LIBERTY MUSIC SOFTWARE  /  GENESIS ANALOG SERIES",9,(142,141,137),False,max_width=510)
    label(draw,1108,HEIGHT-16,f"PANEL {page_no+1:02d}/20",9,(142,141,137),True,max_width=130)
    out=out_dir/page["background"]
    img.save(out,format="PNG",optimize=True)
    return out

def section_names(name,row):
    if name == "GENESIS":
        return (
            "DCO A    /    DCO B    /    MIXER    /    SUB",
            "VCF CUTOFF    /    RESONANCE    /    FILTER MODE    /    DRIVE",
            "LFO    /    MODULATION    /    CHORUS    /    DELAY",
            "VCA LEVEL    /    DETUNE    /    ENVELOPE    /    ARPEGGIATOR",
        )[row]
    if name.startswith("MATRIX"):
        return ("MOD ROUTING  /  SOURCE  /  DESTINATION  /  AMOUNT  /  POLARITY",
                "MOD ROUTING  /  TOUCH CONTROL  /  Q-LINK",
                "MOD ROUTING  /  SOURCE  /  DESTINATION  /  AMOUNT  /  POLARITY",
                "MOD ROUTING  /  TOUCH CONTROL  /  Q-LINK")[row]
    if name in ("CHORUS","DELAY","REVERB"):
        return ("STUDIO EFFECTS    /    ANALOG CHARACTER",
                "MIX    /    TIME    /    MODULATION    /    STEREO",
                "STUDIO EFFECTS    /    SIGNAL PATH",
                "STUDIO EFFECTS    /    OUTPUT CONTROL")[row]
    if name in ("AMP ENV","MOD ENV"):
        return ("ENVELOPES    /    ATTACK    /    DECAY    /    SUSTAIN    /    RELEASE",
                "ENVELOPE SHAPING    /    TOUCH CONTROL",
                "MODULATION    /    VOICE DYNAMICS",
                "ENVELOPE SHAPING    /    Q-LINK")[row]
    if name in ("ARP","PERFORM"):
        return ("PERFORMANCE    /    LIVE CONTROL",
                "RHYTHM    /    EXPRESSION    /    SYNC",
                "PERFORMANCE    /    VOICE",
                "ARPEGGIATOR    /    CONTROL")[row]
    return ("ANALOG SYNTHESIS    /    SIGNAL PATH",
            "SOUND SHAPING    /    TOUCH CONTROL",
            "STUDIO CONTROL    /    MODULATION",
            "SYNTH ENGINE    /    Q-LINK")[row]

def main():
    if len(sys.argv)!=3:
        raise SystemExit("Usage: render_analog_skin.py <ui.json> <assets-directory>")
    ui=json.loads(pathlib.Path(sys.argv[1]).read_text(encoding="utf-8"))
    out=pathlib.Path(sys.argv[2]);out.mkdir(parents=True,exist_ok=True)
    pages=ui["pages"]
    assert len(pages)==20
    for n,page in enumerate(pages):
        assert page.get("background","").startswith("genesis-panel-")
        backgrounds(page,n,out)
    print("Genesis analog panels: rendered 20 original MPC skin assets at 1280x628.")

if __name__=="__main__":
    main()
