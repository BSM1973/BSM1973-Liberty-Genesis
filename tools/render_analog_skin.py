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

def layout_for(page):
    n=len([c for c in page["controls"] if "param" in c])
    if page["name"]=="GENESIS":
        assert n==16
        return [(SLOT_X[i%4],SLOT_Y[i//4],278,107) for i in range(16)]
    out=[]
    for row,cnt in enumerate((min(4,n),max(0,n-4))):
        if not cnt:continue
        x0={1:503,2:343,3:183,4:23}[cnt]
        y=174 if n<=4 else (65 if row==0 else 351)
        out += [(x0+i*320,y,278,228) for i in range(cnt)]
    return out

def backgrounds(page, page_no, out_dir):
    main=page["name"]=="GENESIS"
    rng=random.Random(701+page_no*193)
    im=Image.new("RGB",(WIDTH,HEIGHT),PALETTE["back"])
    d=ImageDraw.Draw(im)
    for y in range(HEIGHT):
        jitter=int(2*math.sin(y*.48)+rng.uniform(-1,1))
        d.line((19,y,1261,y),fill=(22+jitter,23+jitter,26+jitter))
    sections=("GENESIS / ANALOG UI 03","VCF / FILTER","MODULATION / EFFECTS","VCA / PERFORMANCE")
    for row in range(4 if main else (1 if len(page["controls"])<=4 else 2)):
        y0=(ROW_STARTS[row]+6 if main else
            (105 if len(page["controls"])<=4 else (15 if row==0 else 302)))
        accent=BARS[row%4] if main else (BARS[0] if row==0 else BARS[1])
        shade(d,y0,y0+32,tuple(min(255,k+21) for k in accent),accent,23,1257)
        d.line((23,y0,1257,y0),fill=(205,148,67),width=1)
        text=sections[row] if main else page["name"]+(" / SOUND ENGINE" if row==0 else " / MODULATION & Q-LINK")
        label(d,32,y0+9,text,17,(248,231,204),True,max_width=1000)
        label(d,1136,y0+9,"LIBERTY",11,(252,215,151),True,max_width=107)
    for i,(x,y,w,h) in enumerate(layout_for(page)):
        d.rounded_rectangle((x-5,y-5,x+w+5,y+h+5),radius=8,fill=(23,25,27),
                            outline=(104,99,89),width=2)
        d.rounded_rectangle((x+1,y+1,x+w-1,y+h-1),radius=6,fill=(28,30,33),
                            outline=(69,69,68),width=1)
        d.line((x+10,y+8,x+w-10,y+8),fill=(107,100,85),width=1)
        d.rounded_rectangle((x+7,y+h-9,x+w-7,y+h-6),radius=1,
                            fill=(219,162,60) if i%4==0 else ((151,58,59) if i%2 else (48,101,140)))
    if not main and len(page["controls"])<=4:
        label(d,70,467,"GENESIS / ANALOG SERIES",17,(106,103,95),True,max_width=750)
        d.line((70,504,1208,504),fill=(65,64,61),width=1)
    wood(d,rng)
    label(d,25,HEIGHT-16,"LIBERTY MUSIC SOFTWARE / GENESIS / ANALOG UI 03",
          10,(168,162,149),False,max_width=600)
    label(d,1110,HEIGHT-16,f"PANEL {page_no+1:02d}/20",10,
          (168,162,149),True,max_width=135)
    dest=out_dir/page["background"]
    im.save(dest,format="PNG",optimize=True)
    return dest

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
