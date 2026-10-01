"""Validate proposed map geometry and generate a local SVG/HTML review (stdlib only)."""
from pathlib import Path
import html
import json

ROOT = Path(__file__).resolve().parents[2]
OUT = Path(__file__).resolve().parent
cards = []
proposals = []

for index in range(3):
    map_id = 100000000 + index
    source = ROOT / f"LL2_Client_Win/Data/Maps/{map_id}.json"
    data = json.loads(source.read_text(encoding="utf-8"))
    physics = data["physics"]
    platforms = physics["platforms"]
    ladders = physics["climbables"]
    by_id = {p["id"]: p for p in platforms}
    assert len(by_id) == len(platforms)
    assert len({c["id"] for c in ladders}) == len(ladders)
    assert physics["minX"] == 0 and physics["maxX"] == 1536
    safe = physics["safeFeet"]
    assert physics["minX"] <= safe["x"] <= physics["maxX"]
    assert 0 <= safe["y"] < physics["killY"]
    assert any(p["left"] <= safe["x"] <= p["right"] and p["y"] == safe["y"] for p in platforms)
    for p in platforms:
        assert p["id"] > 0 and 0 <= p["left"] < p["right"] <= 1536
        assert 0 <= p["y"] <= 1024
    for c in ladders:
        assert c["id"] > 0 and c["kind"] in ("rope", "ladder")
        assert c["grabRange"] > 0 and 0 <= c["top"] < c["bottom"] <= 1024
        top = by_id[c["topPlatformId"]]
        assert top["left"] <= c["x"] <= top["right"] and abs(top["y"] - c["top"]) <= .05

    image_path = f"../../LL2_Client_Win/Resources_Woodland/Background/forest/forest_ground_{index+1}.png"
    layers = [f'<image href="{image_path}" width="1536" height="1024"/>']
    layers.append('<g class="platforms" stroke="#4dff77" stroke-width="3">')
    rows = []
    for p in platforms:
        title = f'P{p["id"]}: x={p["left"]}..{p["right"]}, y={p["y"]}'
        layers.append(f'<g><title>{title}</title><path d="M{p["left"]} {p["y"]}H{p["right"]}"/>'
                      f'<text x="{p["left"]+6}" y="{p["y"]-9}" class="label">P{p["id"]} · y={p["y"]}</text></g>')
        rows.append(f'<tr><td>P{p["id"]}</td><td>{p["left"]}–{p["right"]}</td><td>{p["y"]}</td></tr>')
    layers.append('</g><g class="ladders" stroke="#ffcf40" stroke-width="3">')
    ladder_rows = []
    for c in ladders:
        title = f'L{c["id"]}: x={c["x"]}, y={c["top"]}..{c["bottom"]}, exit=P{c["topPlatformId"]}'
        layers.append(f'<g><title>{title}</title><rect x="{c["x"]-c["grabRange"]}" y="{c["top"]}" '
                      f'width="{c["grabRange"]*2}" height="{c["bottom"]-c["top"]}" fill="#ffcf40" fill-opacity=".12" stroke-width="1"/>'
                      f'<path d="M{c["x"]} {c["top"]}V{c["bottom"]}"/>'
                      f'<text x="{c["x"]+22}" y="{c["top"]+30}" class="label">L{c["id"]}</text></g>')
        ladder_rows.append(f'<tr><td>L{c["id"]}</td><td>{c["x"]}</td><td>{c["top"]}–{c["bottom"]}</td><td>P{c["topPlatformId"]}</td></tr>')
    layers.append('</g>')
    layers.append(f'<g class="safe"><circle cx="{safe["x"]}" cy="{safe["y"]}" r="9" fill="#55deff" stroke="#03171f" stroke-width="3"/>'
                  f'<text class="label" x="{safe["x"]+13}" y="{safe["y"]-14}">SAFE ({safe["x"]}, {safe["y"]})</text></g>')
    if index == 2:
        layers.append('<g class="bridge"><path d="M890 525L1035 631L1075 631" fill="none" stroke="#ff658c" stroke-width="5" stroke-dasharray="10 7"/>'
                      '<text class="label" x="946" y="554">BRIDGE: NO COLLISION</text></g>')
    svg = ('<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1536 1024" role="img" '
           f'aria-label="Forest {index+1} proposed geometry">'
           '<style>.label{font: bold 17px monospace;fill:white;stroke:#101820;stroke-width:4px;paint-order:stroke;stroke-linejoin:round}</style>'
           + ''.join(layers) + '</svg>')
    (OUT / f"forest_{index+1}.svg").write_text(svg, encoding="utf-8")
    note = ('대각선 다리(분홍 점선)는 현재 수평 발판/수직 줄 형식으로 표현할 수 없어 충돌을 추가하지 않았습니다.'
            if index == 2 else '사다리 아래 끝은 그림 길이를 유지했습니다. 일부는 점프해서 잡는 형태입니다.')
    cards.append(f'''<section id="map-{index+1}"><h2>Forest {index+1} <small>{map_id}</small></h2>
      <p>발판 {len(platforms)}개 · 사다리 {len(ladders)}개 · 로프 0개 · 1536 × 1024</p>
      <div class="canvas">{svg}</div><p class="coords">이미지 위에 마우스를 올리면 월드 좌표가 표시됩니다.</p>
      <p>{note}</p><details><summary>좌표 목록</summary><div class="tables">
      <table><tr><th>발판</th><th>X 구간</th><th>Y</th></tr>{''.join(rows)}</table>
      <table><tr><th>사다리</th><th>X</th><th>위–아래 Y</th><th>상단 발판</th></tr>{''.join(ladder_rows)}</table>
      </div></details></section>''')
    proposals.append({"mapId": map_id, "physics": physics})
    print(f"PASS {map_id}: {len(platforms)} platforms, {len(ladders)} ladders; bounds, IDs, exits, safeFeet")

template = '''<!doctype html><html lang="ko"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>숲 맵 좌표 검토 · 클라이언트 확정</title><style>
*{box-sizing:border-box}body{margin:0;background:#10191c;color:#edf5f2;font:16px/1.6 system-ui,sans-serif}
main{max-width:1576px;margin:auto;padding:24px}h1{margin-bottom:8px}h2{margin-bottom:0}small{font-size:16px;color:#a6c4bb}
p{color:#c2d6cf}.toolbar{position:sticky;top:0;z-index:2;background:#1b2e32;padding:12px 20px;display:flex;gap:20px;flex-wrap:wrap;border-bottom:1px solid #406063}
a{color:#9deac7}.badge{color:#ffcf40}section{margin:32px 0 52px}.canvas{border:1px solid #42605c}svg{display:block;width:100%;height:auto}
table{border-collapse:collapse;min-width:260px}td,th{padding:6px 20px;border-bottom:1px solid #36524d;text-align:left}.tables{display:flex;gap:36px;flex-wrap:wrap;padding:15px 0}
.coords{font-family:monospace;color:#80def3;margin:4px 0}summary{cursor:pointer}label{cursor:pointer}
</style><div class="toolbar"><a href="#map-1">Forest 1</a><a href="#map-2">Forest 2</a><a href="#map-3">Forest 3</a>
<label><input type="checkbox" data-layer="platforms" checked> 초록: 발판</label>
<label><input type="checkbox" data-layer="ladders" checked> 노랑: 사다리·탑승 범위</label>
<label><input type="checkbox" data-layer="safe" checked> 하늘색: 복귀 발점</label></div><main>
<h1>숲 맵 좌표 검토 <small class="badge">2026-10-01 · 클라이언트 확정</small></h1>
<p>실제 맵 JSON을 원본 이미지 위에 표시했습니다. 이 클라이언트 작업에서 서버 파일은 수정하지 않았습니다. 이미지 위의 숫자는 월드 좌표입니다.</p>
<p>표면의 잔디 굴곡은 수평선으로 근사했습니다. 세 이미지에서 확인되는 사다리를 지정했으며 장식 덩굴은 탑승 대상으로 추가하지 않았습니다.
기존 포탈·스폰 값은 이번 수정 범위에서 유지했습니다. 새 지면과 맞추는 후속 작업이 필요합니다.</p>
__CARDS__
</main><script>
document.querySelectorAll('[data-layer]').forEach(c=>c.addEventListener('change',()=>{
document.querySelectorAll('.'+c.dataset.layer).forEach(e=>e.style.display=c.checked?'':'none');}));
document.querySelectorAll('section svg').forEach(svg=>svg.addEventListener('pointermove',e=>{
const p=new DOMPoint(e.clientX,e.clientY).matrixTransform(svg.getScreenCTM().inverse());
svg.closest('section').querySelector('.coords').textContent=`월드 X=${p.x.toFixed(0)}   Y=${p.y.toFixed(0)}`;}));
</script></html>'''
(OUT / "index.html").write_text(template.replace('__CARDS__', ''.join(cards)), encoding="utf-8")
(OUT / "server_physics_proposal.json").write_text(json.dumps({
    "description": "2026-10-01 approved client art-aligned geometry; server deployment unverified; NOT a server export; replace physics only",
    "maps": proposals,
}, ensure_ascii=False, indent=2) + '\n', encoding="utf-8")
