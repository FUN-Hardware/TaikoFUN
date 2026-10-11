"""Render captured production draw commands with Pillow for layout review.

This is an offscreen preview with approximate font metrics, not a DxLib screenshot.
Usage: python tests/render_song_select_captures.py <captures directory>
"""
from pathlib import Path
import json
import sys
from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[1]
FONT_PATHS = [Path('C:/Windows/Fonts/meiryob.ttc'), Path('C:/Windows/Fonts/meiryo.ttc'),
              Path('/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc')]
FONT = next((p for p in FONT_PATHS if p.exists()), None)
if FONT is None:
    raise SystemExit('Japanese font is required for visual verification')
SCALE = 2

def render(path):
    canvas = Image.new('RGBA', (1280*SCALE, 720*SCALE))
    def point(x, y): return (x*SCALE, y*SCALE)
    def box(x, y, r, b): return (x*SCALE, y*SCALE, r*SCALE, b*SCALE)
    def color(c, alpha=255): return (c>>16 & 255, c>>8 & 255, c & 255, alpha)
    for raw in path.read_text(encoding='utf-8').splitlines():
        op, *a = json.loads(raw)
        layer = Image.new('RGBA', canvas.size)
        draw = ImageDraw.Draw(layer)
        if op == 'background':
            x,y,alpha=a
            bg=Image.open(ROOT/'TaikoFUN/Resource/Image/SkinColor/songselectbg.png').convert('RGBA')
            bg=bg.resize((1280*SCALE,720*SCALE),Image.Resampling.LANCZOS)
            bg.putalpha(alpha)
            layer.paste(bg,point(x,y))
        elif op == 'rect':
            x,y,r,b,c,alpha=a; draw.rectangle(box(x,y,r,b),fill=color(c,alpha))
        elif op == 'round':
            x,y,r,b,radius,c,alpha=a
            draw.rounded_rectangle(box(x,y,r,b),radius*SCALE,fill=color(c,alpha))
        elif op == 'circle':
            x,y,radius,c,alpha=a
            draw.ellipse(box(x-radius,y-radius,x+radius,y+radius),fill=color(c,alpha))
        elif op == 'triangle':
            x,y,r,b,u,v,c,alpha=a
            draw.polygon([point(x,y),point(r,b),point(u,v)],fill=color(c,alpha))
        elif op == 'line':
            x,y,r,b,c,width,alpha=a
            draw.line([point(x,y),point(r,b)],fill=color(c,alpha),width=width*SCALE)
        elif op == 'text':
            x,y,size,c,alpha,text=a
            font=ImageFont.truetype(str(FONT),size*SCALE)
            # Align glyphs by top, like the game's top-left text coordinates.
            draw.text(point(x,y),text,font=font,fill=color(c,alpha),anchor='lt')
        else:
            raise ValueError(op)
        canvas=Image.alpha_composite(canvas,layer)
    canvas.resize((1280,720),Image.Resampling.LANCZOS).convert('RGB').save(path.with_suffix('.png'))

for capture in sorted(Path(sys.argv[1]).glob('*.jsonl')):
    render(capture)
    print(capture.with_suffix('.png'))
