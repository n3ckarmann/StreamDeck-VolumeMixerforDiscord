import os
DEFS='''<defs>
<linearGradient id="tile" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#2b2e35"/><stop offset="1" stop-color="#101114"/></linearGradient>
<linearGradient id="gloss" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#fff" stop-opacity="0.10"/><stop offset="1" stop-color="#fff" stop-opacity="0"/></linearGradient>
<linearGradient id="white" gradientUnits="userSpaceOnUse" x1="0" y1="110" x2="0" y2="400"><stop offset="0" stop-color="#ffffff"/><stop offset="1" stop-color="#d9dce6"/></linearGradient>
<linearGradient id="grey" gradientUnits="userSpaceOnUse" x1="0" y1="110" x2="0" y2="400"><stop offset="0" stop-color="#9aa0ad"/><stop offset="1" stop-color="#6b7180"/></linearGradient>
<linearGradient id="green" gradientUnits="userSpaceOnUse" x1="0" y1="110" x2="0" y2="400"><stop offset="0" stop-color="#b9f6d0"/><stop offset="0.5" stop-color="#43d17f"/><stop offset="1" stop-color="#1f9d58"/></linearGradient>
<linearGradient id="red" gradientUnits="userSpaceOnUse" x1="0" y1="110" x2="0" y2="400"><stop offset="0" stop-color="#ff8a8c"/><stop offset="1" stop-color="#e03b3e"/></linearGradient>
<linearGradient id="blurple" gradientUnits="userSpaceOnUse" x1="0" y1="110" x2="0" y2="400"><stop offset="0" stop-color="#9aa3ff"/><stop offset="1" stop-color="#5865F2"/></linearGradient>
<filter id="glow" filterUnits="userSpaceOnUse" x="0" y="0" width="512" height="512"><feGaussianBlur stdDeviation="9" result="b"/><feMerge><feMergeNode in="b"/><feMergeNode in="SourceGraphic"/></feMerge></filter>
<filter id="shadow" filterUnits="userSpaceOnUse" x="0" y="0" width="512" height="512"><feDropShadow dx="0" dy="6" stdDeviation="8" flood-color="#000" flood-opacity="0.55"/></filter>
<clipPath id="tc"><rect x="6" y="6" width="500" height="500" rx="112"/></clipPath>
</defs>'''
TILE='''<rect x="6" y="6" width="500" height="500" rx="112" fill="url(#tile)"/>
<g clip-path="url(#tc)"><ellipse cx="256" cy="20" rx="340" ry="200" fill="url(#gloss)"/></g>
<rect x="7.5" y="7.5" width="497" height="497" rx="110.5" fill="none" stroke="#fff" stroke-opacity="0.13" stroke-width="3"/>'''
def svg(body,tile=True): return f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 512 512" width="512" height="512">{DEFS}{TILE if tile else ""}{body}</svg>'
SLASH='<g filter="url(#shadow)"><line x1="150" y1="390" x2="362" y2="122" stroke="#101114" stroke-width="44" stroke-linecap="round"/><line x1="150" y1="390" x2="362" y2="122" stroke="url(#red)" stroke-width="26" stroke-linecap="round"/></g>'
def mic(fill,glow):
    cap=f'<rect x="206" y="110" width="100" height="180" rx="50" fill="{fill}"/>'
    if glow: cap=f'<g filter="url(#glow)">{cap}</g>'
    return f'''<g filter="url(#shadow)">{cap}
<path d="M166 250 a90 90 0 0 0 180 0" fill="none" stroke="url(#white)" stroke-width="22" stroke-linecap="round"/>
<line x1="256" y1="340" x2="256" y2="392" stroke="url(#white)" stroke-width="22" stroke-linecap="round"/>
<line x1="206" y1="398" x2="306" y2="398" stroke="url(#white)" stroke-width="22" stroke-linecap="round"/></g>'''
def speaker(body,waves):
    s=f'<g filter="url(#shadow)"><path d="M130 210 h56 l78 -66 v224 l-78 -66 h-56 a14 14 0 0 1 -14 -14 v-64 a14 14 0 0 1 14 -14z" fill="{body}" stroke="{body}" stroke-width="10" stroke-linejoin="round"/></g>'
    if waves: s+='''<g filter="url(#glow)" fill="none" stroke="url(#green)" stroke-width="22" stroke-linecap="round">
<path d="M306 206 a70 70 0 0 1 0 100"/><path d="M348 166 a126 126 0 0 1 0 180"/></g>'''
    return s
def headphones(col,cups):
    return f'''<g filter="url(#shadow)"><path d="M132 300 v-30 a124 124 0 0 1 248 0 v30" fill="none" stroke="{col}" stroke-width="26" stroke-linecap="round"/>
<rect x="112" y="276" width="70" height="120" rx="30" fill="{col}"/><rect x="330" y="276" width="70" height="120" rx="30" fill="{col}"/></g>
<g filter="url(#glow)"><rect x="160" y="300" width="10" height="72" rx="5" fill="{cups}"/><rect x="342" y="300" width="10" height="72" rx="5" fill="{cups}"/></g>'''
def chevron(d): return f'<g filter="url(#shadow)"><path d="{d}" fill="none" stroke="url(#white)" stroke-width="40" stroke-linecap="round" stroke-linejoin="round"/></g>'
icons={
 'mic_on': mic('url(#green)',True),
 'mic_off': mic('url(#grey)',False)+SLASH,
 'audio_on': headphones('url(#white)','url(#green)'),
 'audio_off': headphones('url(#grey)','#3a3d45')+SLASH,
 'volume_up': '<g filter="url(#glow)"><path d="M256 156 v200 M156 256 h200" stroke="url(#green)" stroke-width="46" stroke-linecap="round"/></g>',
 'volume_down': '<g filter="url(#glow)"><path d="M156 256 h200" stroke="url(#blurple)" stroke-width="46" stroke-linecap="round"/></g>',
 'page_prev': chevron('M296 156 L196 256 L296 356'),
 'page_next': chevron('M216 156 L316 256 L216 356'),
 'back': chevron('M256 366 V166 M166 246 L256 156 L346 246'),
 'user': '<g filter="url(#shadow)" fill="url(#white)"><circle cx="256" cy="200" r="70"/><path d="M126 392 a130 110 0 0 1 260 0 z"/></g>',
 'empty': '',
}
for n,b in icons.items(): open(f'{n}.svg','w').write(svg(b))
# speaking overlay: transparent, green glowing ring along tile edge
open('speaking_deco.svg','w').write(svg('<g filter="url(#glow)"><rect x="16" y="16" width="480" height="480" rx="104" fill="none" stroke="url(#green)" stroke-width="18"/></g>',tile=False))
print(" ".join(sorted(os.listdir('.'))))
