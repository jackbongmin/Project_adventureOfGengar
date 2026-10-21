const fs=require('fs');
const src=fs.readFileSync('shadow-phantom.html','utf8');
const js=src.slice(src.indexOf('<script>')+8,src.lastIndexOf('</script>'));
const page=`<title>Shadow Phantom</title>
<meta name="claude:viewport-fit-sides" content="cover">
<style>
/* layout: one centered game stage with a key legend underneath; a single dark look by choice */
:root{--bg:#0b0714;--panel:#160b2a;--fg:#e6d6ff;--muted:#9a82c0;--accent:#c58bff;--key:#2a1650;--keyline:#4a2a7a;color-scheme:dark}
html,body{margin:0}
body{background:var(--bg);color:var(--fg);font:14px/1.5 "Malgun Gothic","Apple SD Gothic Neo","Noto Sans KR",system-ui,sans-serif;
  padding-block:16px;padding-left:max(16px,env(safe-area-inset-left,0px));padding-right:max(16px,env(safe-area-inset-right,0px))}
main{max-width:960px;margin:0 auto;display:flex;flex-direction:column;gap:12px}
canvas{display:block;width:100%;height:auto;aspect-ratio:960/544;background:#000;border-radius:6px;outline:1px solid var(--keyline);cursor:pointer}
canvas:focus-visible{outline:2px solid var(--accent)}
.legend{display:flex;flex-wrap:wrap;gap:8px 18px;align-items:center;justify-content:center;color:var(--muted)}
.legend span{display:inline-flex;gap:6px;align-items:center}
kbd{font:600 12px/1 ui-monospace,Consolas,monospace;background:var(--key);color:var(--fg);border:1px solid var(--keyline);border-bottom-width:3px;border-radius:4px;padding:5px 8px}
.note{text-align:center;color:var(--muted);margin:0}
</style>
<main>
<canvas id="c" width="960" height="544" tabindex="0" aria-label="Shadow Phantom 게임 화면"></canvas>
<div class="legend">
<span><kbd>←</kbd><kbd>→</kbd> 이동</span>
<span><kbd>C</kbd> 점프</span>
<span><kbd>Z</kbd> 섀도우볼</span>
<span><kbd>X</kbd> 필살기</span>
<span><kbd>Esc</kbd> 일시정지</span>
<span><kbd>Enter</kbd> 시작 · 선택</span>
</div>
<p class="note">게임 화면을 한 번 클릭한 뒤 키보드로 조작하세요. Space와 ↑도 점프로 쓸 수 있습니다. 적을 처치하면 필살기 게이지가 차오릅니다. PC 키보드 전용입니다.</p>
</main>
<script>
${js.replace("addEventListener('keydown'","document.getElementById('c').addEventListener('pointerdown',()=>{document.getElementById('c').focus();initAudio();});\naddEventListener('keydown'")}
</script>
`;
fs.writeFileSync('shadow-phantom-artifact.html',page);
