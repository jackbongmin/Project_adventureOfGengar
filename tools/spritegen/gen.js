// 도트 스프라이트 생성기: 도형으로 픽셀 격자를 만들고 외곽선/음영을 자동으로 입힌 뒤 문자 격자로 출력한다.
// 사용: node gen.js preview [이름...]   |   node gen.js cpp <출력경로>
const fs = require('fs');

class G {
  constructor(w, h) { this.w = w; this.h = h; this.p = Array.from({ length: h }, () => Array(w).fill('.')); }
  set(x, y, c) { x = Math.round(x); y = Math.round(y); if (x >= 0 && y >= 0 && x < this.w && y < this.h) this.p[y][x] = c; }
  get(x, y) { return (x >= 0 && y >= 0 && x < this.w && y < this.h) ? this.p[y][x] : '.'; }
  rect(x, y, w, h, c) { for (let j = 0; j < h; j++) for (let i = 0; i < w; i++) this.set(x + i, y + j, c); }
  ellipse(cx, cy, rx, ry, c, clip) {
    for (let y = 0; y < this.h; y++) for (let x = 0; x < this.w; x++) {
      const dx = (x + 0.5 - cx) / rx, dy = (y + 0.5 - cy) / ry;
      if (dx * dx + dy * dy <= 1 && (!clip || clip(x, y))) this.p[y][x] = c;
    }
  }
  poly(pts, c) {
    for (let y = 0; y < this.h; y++) for (let x = 0; x < this.w; x++) {
      const px = x + 0.5, py = y + 0.5; let inside = false;
      for (let i = 0, j = pts.length - 1; i < pts.length; j = i++) {
        const [xi, yi] = pts[i], [xj, yj] = pts[j];
        if (((yi > py) !== (yj > py)) && (px < (xj - xi) * (py - yi) / (yj - yi) + xi)) inside = !inside;
      }
      if (inside) this.p[y][x] = c;
    }
  }
  // 채워진 픽셀 바깥 1px에 외곽선. skip: 외곽선을 만들지 않을 대상 문자들
  outline(c, skip = []) {
    const add = [];
    for (let y = 0; y < this.h; y++) for (let x = 0; x < this.w; x++) {
      if (this.p[y][x] !== '.') continue;
      for (const [dx, dy] of [[1, 0], [-1, 0], [0, 1], [0, -1]]) {
        const n = this.get(x + dx, y + dy);
        if (n !== '.' && n !== c && !skip.includes(n)) { add.push([x, y]); break; }
      }
    }
    add.forEach(([x, y]) => this.p[y][x] = c);
  }
  // 광원(좌상단) 기준 음영: base 픽셀을 light/dark 로 바꾼다
  shade(base, light, dark, cx, cy, rx, ry, lt = 0.55, dt = 0.5) {
    for (let y = 0; y < this.h; y++) for (let x = 0; x < this.w; x++) {
      if (this.p[y][x] !== base) continue;
      const d = ((x + 0.5 - cx) / rx) * 0.6 + ((y + 0.5 - cy) / ry) * 0.7;
      if (d < -lt) this.p[y][x] = light; else if (d > dt) this.p[y][x] = dark;
    }
  }
  mirror(srcX0, srcX1, axis) { // 좌->우 대칭 복사
    for (let y = 0; y < this.h; y++) for (let x = srcX0; x <= srcX1; x++) {
      const c = this.get(x, y); if (c !== '.') this.set(axis - x, y, c);
    }
  }
  rows() { return this.p.map(r => r.join('')); }
}

const S = {};

// ---------- 주인공 (보라색 유령, 24x24, 오른쪽을 본다) ----------
function phantom(frame) {
  const g = new G(24, 24);
  // 등 가시(왼쪽): 귀처럼 밑동이 두꺼운 삼각형 3개 (어두운 보라색으로 몸통과 구분)
  g.poly([[6.5, 4], [0.8, 6.6], [6.5, 10]], 'd'); g.poly([[5.5, 10.6], [0.8, 13.4], [5.5, 16.4]], 'd'); g.poly([[6.5, 17], [1.5, 20], [8, 21.8]], 'd');
  // 귀
  g.poly([[4.5, 8], [5.5, 0.5], [11, 6]], 'p'); g.poly([[13, 6], [18.5, 0.5], [19.5, 8]], 'p');
  // 몸통
  g.ellipse(12, 13, 9.5, 8.5, 'p');
  // 아랫단(일렁임)
  const tails = frame === 0 ? [[6, 10, 8, 23], [10, 14, 12, 21.5], [14, 19, 16.5, 23]] : [[6, 10, 8, 21.5], [10, 14, 12, 23], [14, 19, 16.5, 21.5]];
  tails.forEach(([a, b, tx, ty]) => g.poly([[a, 19], [b, 19], [tx, ty]], 'p'));
  // 팔
  g.ellipse(19.5, 16, 2.4, 3, 'p'); g.ellipse(4.5, 16, 2.4, 3, 'p');
  g.shade('p', 'l', 'd', 12, 12, 11, 11);
  // 눈: 바깥쪽 위 -> 안쪽 아래로 기울어진 반달 (윗변은 직선, 아랫변은 둥글게)
  const eye = (flip) => {
    const px = [[5, 8], [6, 8], [5, 9], [6, 9], [7, 9], [8, 9], [6, 10], [7, 10], [8, 10], [9, 10], [10, 10], [7, 11], [8, 11], [9, 11]];
    px.forEach(([x, y]) => g.set(flip ? 24 - x : x + 1, y, 'r'));
    g.set(flip ? 24 - 8 : 8 + 1, 10, 'w');
  };
  eye(false); eye(true);
  // 큰 미소 (초승달 모양 + 이빨 선)
  g.set(5, 13, 'k'); g.set(19, 13, 'k'); g.rect(6, 14, 13, 1, 'k');
  g.rect(6, 15, 1, 1, 'k'); g.rect(18, 15, 1, 1, 'k'); g.rect(7, 15, 11, 1, 'w');
  g.rect(7, 16, 1, 1, 'k'); g.rect(17, 16, 1, 1, 'k'); g.rect(8, 16, 9, 1, 'w');
  g.rect(8, 17, 1, 1, 'k'); g.rect(16, 17, 1, 1, 'k'); g.rect(9, 17, 7, 1, 'w');
  g.rect(9, 18, 7, 1, 'k');
  [9, 12, 15].forEach(x => { g.set(x, 15, 'k'); g.set(x, 16, 'k'); });
  g.outline('o');
  return g.rows();
}
S.phantom0 = phantom(0); S.phantom1 = phantom(1);

// ---------- 슬라임 (16x12) ----------
function slime(f) {
  const g = new G(16, 12);
  const ry = f === 0 ? 7.5 : 6.5, rx = f === 0 ? 7.5 : 8;
  g.ellipse(8, 11 - 0, rx, ry + 0.5, 'g', (x, y) => y <= 10);
  g.shade('g', 'h', 'd', 8, 6, 8, 7);
  const ey = f === 0 ? 5 : 6;
  g.rect(4, ey, 3, 3, 'w'); g.rect(9, ey, 3, 3, 'w'); g.rect(5, ey + 1, 2, 2, 'k'); g.rect(10, ey + 1, 2, 2, 'k');
  g.set(4, ey - 2, 'W'); g.set(5, ey - 2, 'W');
  g.outline('o');
  return g.rows();
}
S.slime0 = slime(0); S.slime1 = slime(1);

// ---------- 박쥐 (20x14) ----------
function bat(f) {
  const g = new G(20, 14);
  const wl = f === 0
    ? [[7, 7], [2, 1], [1, 6], [3, 5], [4, 9.5], [6, 8], [7, 10]]
    : [[7, 7], [1, 9], [2, 13.5], [4, 10.5], [5, 13.5], [7, 11]];
  g.poly(wl, 'w'); g.poly(wl.map(([x, y]) => [20 - x, y]), 'w');
  g.ellipse(10, 8, 3.5, 4.2, 'b');
  g.poly([[6.6, 6], [7, 2.5], [9, 5]], 'b'); g.poly([[13.4, 6], [13, 2.5], [11, 5]], 'b');
  g.shade('w', 'W', 'v', 10, 7, 11, 8, 0.5, 0.3);
  g.set(8, 7, 'y'); g.set(11, 7, 'y'); g.set(9, 10, 'f'); g.set(11, 10, 'f');
  g.outline('o');
  return g.rows();
}
S.bat0 = bat(0); S.bat1 = bat(1);

// ---------- 그림자 늑대 (24x16, 오른쪽을 본다) ----------
function hound(f) {
  const g = new G(24, 16);
  const off = f === 0 ? [[0, 1], [1, -1], [-1, 1], [1, 0]] : [[1, 0], [-1, 1], [1, -1], [0, 1]];
  [4, 8, 14, 18].forEach((lx, i) => g.rect(lx + off[i][0], 10, 2, 5 + (off[i][1] > 0 ? 0 : -1), 'p'));
  g.poly([[3, 7], [0, 2], [5, 6]], 'p');                       // 꼬리
  g.ellipse(10, 8.5, 8, 4.3, 'p');                              // 몸통
  g.ellipse(18, 5, 4.6, 3.6, 'p'); g.rect(19, 5, 5, 3, 'p');     // 머리/주둥이
  g.poly([[16, 3], [17, -0.5], [19.5, 3]], 'p');                // 귀
  for (let i = 0; i < 3; i++) g.poly([[5 + i * 3.6, 5.5], [6.6 + i * 3.6, 2.5], [8.2 + i * 3.6, 5.5]], 'v');
  g.shade('p', 'l', 'd', 12, 7, 12, 8);
  g.set(19, 4, 'e'); g.set(20, 4, 'e'); g.set(22, 9, 'w'); g.set(21, 9, 'w');
  g.outline('o');
  return g.rows();
}
S.hound0 = hound(0); S.hound1 = hound(1);

// ---------- 후드 마법사 (16x20, 오른쪽을 본다) ----------
function caster(f) {
  const g = new G(16, 20);
  g.rect(13, 3, 1, 17, 'b');                                     // 지팡이
  g.poly([[2, 20], [13, 20], [8.5, 6]], 'p');                    // 로브
  g.ellipse(8, 7.5, 4.6, 4.6, 'p');                              // 후드
  g.shade('p', 'l', 'd', 8, 10, 7, 10);
  g.rect(2, 18, 11, 2, 'd');                                     // 단
  g.ellipse(9, 8, 3, 3.2, 'k');                                  // 후드 속 어둠
  g.set(8, 8, 'r'); g.set(11, 8, 'r');
  const orb = f === 0 ? 'm' : 'w';
  g.ellipse(13.5, 2, 2.4, 2.4, 'm'); g.set(13, 1, orb); g.set(14, 1, orb);
  g.outline('o');
  return g.rows();
}
S.caster0 = caster(0); S.caster1 = caster(1);

// ---------- 코인 (8x8, 4프레임 회전) ----------
function coin(w) {
  const g = new G(8, 8);
  g.ellipse(4, 4, w / 2, 3.9, 'y');
  g.shade('y', 'h', 'd', 4, 4, 4, 4, 0.4, 0.45);
  g.outline('o');
  return g.rows();
}
S.coin0 = coin(7); S.coin1 = coin(5); S.coin2 = coin(2.5); S.coin3 = coin(5);

// ---------- 하트 (9x8) ----------
function heart(full) {
  const rows = [
    '.oo...oo.', 'oRRo.oRRo', 'oRWRoRRRo', 'oRRRRRRRo', '.oRRRRRo.', '..oRRRo..', '...oRo...', '....o....'];
  const g = new G(9, 8);
  rows.forEach((r, y) => [...r].forEach((ch, x) => { if (ch !== '.') g.set(x, y, full ? ch : (ch === 'o' ? 'o' : 'E')); }));
  return g.rows();
}
S.heartFull = heart(true); S.heartEmpty = heart(false);

// ---------- 섀도우볼 (8x8, 2프레임) ----------
function ball(f) {
  const g = new G(8, 8);
  g.ellipse(4, 4, 4, 4, 'r');
  g.ellipse(4, 4, 3, 3, 'm');
  g.ellipse(4, 4, 2, 2, 'd');
  g.set(2, 2, 'h'); g.set(3, 2, 'h');
  if (f === 0) { g.set(5, 3, 'l'); g.set(5, 4, 'l'); g.set(4, 5, 'l'); } else { g.set(3, 5, 'l'); g.set(4, 5, 'l'); g.set(5, 4, 'l'); }
  return g.rows();
}
S.ball0 = ball(0); S.ball1 = ball(1);

// ---------- 적 마법구 (8x8) ----------
(function () {
  const g = new G(8, 8);
  g.ellipse(4, 4, 4, 4, 'o'); g.ellipse(4, 4, 3, 3, 'p'); g.ellipse(4, 4, 1.6, 1.6, 'w');
  S.orb0 = g.rows();
})();

// ---------- 충격파 불꽃 (12x12, 2프레임) ----------
function wave(f) {
  const g = new G(12, 12);
  const tip = f === 0 ? 0 : 1.5;
  g.poly([[0, 12], [4, 5], [6, tip], [8, 5], [12, 12]], 'p');
  g.poly([[3, 12], [5.5, 7], [6, 4 + tip], [7, 7], [9, 12]], 'w');
  g.outline('o');
  return g.rows();
}
S.wave0 = wave(0); S.wave1 = wave(1);

// ---------- 깃발 천 (20x12, 3프레임) ----------
function pennant(ph) {
  const g = new G(20, 12);
  for (let x = 0; x < 20; x++) {
    const top = Math.round(1 + Math.sin(x * 0.55 + ph) * 1.4), h = Math.round(10 - x * 0.35);
    for (let y = top; y < top + h; y++) g.set(x, y, y < top + 2 ? 'l' : 'p');
  }
  g.outline('o');
  return g.rows();
}
S.pennant0 = pennant(0); S.pennant1 = pennant(2.1); S.pennant2 = pennant(4.2);

// ---------- 달 (22x22) ----------
(function () {
  const g = new G(22, 22);
  g.ellipse(11, 11, 11, 11, 'm');
  g.shade('m', 'M', 'd', 11, 11, 11, 11, 0.7, 0.55);
  g.ellipse(14.5, 7, 2.2, 2.2, 'c'); g.ellipse(7, 14, 3, 3, 'c'); g.ellipse(15, 15, 1.6, 1.6, 'c'); g.ellipse(6.5, 6, 1.3, 1.3, 'c');
  S.moon = g.rows();
})();

// ---------- 타일 16x16 ----------
function hash(x, y) { let h = (x * 374761393 + y * 668265263) ^ 0x5bd1e995; h = (h ^ (h >>> 13)) * 1274126177; return ((h ^ (h >>> 16)) >>> 0) / 4294967296; }
S.tileGroundTop = (function () {
  const g = new G(16, 16);
  g.rect(0, 0, 16, 16, 'D');
  for (let x = 0; x < 16; x++) { g.set(x, 0, 'T'); g.set(x, 1, 'T'); g.set(x, 2, 't'); g.set(x, 3, hash(x, 1) < 0.5 ? 't' : 'D'); }
  for (let x = 0; x < 16; x++) if (hash(x, 9) < 0.35) { g.set(x, 4, 't'); }
  for (let i = 0; i < 6; i++) g.set(Math.floor(hash(i, 3) * 16), 5 + Math.floor(hash(i, 4) * 11), 'S');
  for (let i = 0; i < 3; i++) g.set(Math.floor(hash(i, 7) * 15), 6 + Math.floor(hash(i, 8) * 9), 'L');
  return g.rows();
})();
S.tileDirt = (function () {
  const g = new G(16, 16);
  g.rect(0, 0, 16, 16, 'D');
  for (let i = 0; i < 9; i++) g.set(Math.floor(hash(i, 13) * 16), Math.floor(hash(i, 14) * 16), 'S');
  for (let i = 0; i < 4; i++) { const x = Math.floor(hash(i, 21) * 14), y = Math.floor(hash(i, 22) * 14); g.set(x, y, 'L'); g.set(x + 1, y, 'L'); }
  return g.rows();
})();
S.tileBrick = (function () {
  const g = new G(16, 16);
  g.rect(0, 0, 16, 16, 'B');
  for (let course = 0; course < 2; course++) {
    const y0 = course * 8;
    g.rect(0, y0, 16, 1, 'H'); g.rect(0, y0 + 7, 16, 1, 'M');
    const vx = course === 0 ? 5 : 13; g.rect(vx, y0 + 1, 1, 6, 'M'); if (course === 1) g.rect(5, y0 + 1, 1, 6, 'M');
    g.rect(0, y0 + 6, 16, 1, 'S');
  }
  g.rect(15, 0, 1, 16, 'S'); g.rect(0, 0, 1, 16, 'H');
  return g.rows();
})();
S.tileCrumble = (function () {
  const g = new G(16, 16);
  g.rect(0, 0, 16, 16, 'B'); g.rect(0, 0, 16, 2, 'H'); g.rect(0, 14, 16, 2, 'S');
  const cr = [[3, 2], [4, 3], [4, 4], [5, 5], [5, 6], [4, 7], [4, 8], [3, 9], [3, 10], [2, 11], [2, 12], [11, 2], [10, 3], [10, 4], [9, 5], [9, 6], [10, 7], [10, 8], [11, 9], [12, 10], [12, 11]];
  cr.forEach(([x, y]) => g.set(x, y, 'K'));
  for (let i = 0; i < 10; i++) g.set(Math.floor(hash(i, 31) * 16), 2 + Math.floor(hash(i, 32) * 12), 'S');
  return g.rows();
})();
S.tileSpike = (function () {
  const g = new G(16, 16);
  [[0, 7], [8, 15]].forEach(([a, b]) => {
    const mid = (a + b + 1) / 2;
    g.poly([[a, 16], [mid, 6], [b + 1, 16]], 'L');
    g.poly([[mid, 6], [b + 1, 16], [mid, 16]], 'S');
  });
  g.outline('o');
  return g.rows();
})();

// ---------- 보스 VOIDMAW (64x64) + 손 (14x18) ----------
S.boss = (function () {
  const g = new G(64, 64);
  // 망토: 둥근 어깨(위) + 종 모양 몸통(아래) + 너덜너덜한 아랫단
  g.ellipse(32, 34, 29, 25, 'c', (x, y) => y <= 36);
  g.poly([[3, 34], [61, 34], [57, 54], [7, 54]], 'c');
  for (let i = 0; i < 7; i++) {
    const x0 = 6 + i * 7.4, tip = 60 + (i % 2 ? 3 : -1);
    g.poly([[x0, 53], [x0 + 7.6, 53], [x0 + 3.8, tip]], 'c');
  }
  g.ellipse(32, 27, 21, 13, 'm', (x, y) => y <= 30);
  g.shade('c', 'm', 'c', 32, 34, 30, 28, 0.85, 9);
  // 뿔 왕관 (어깨 위로 솟는다)
  [[32, 0, 5], [23, 4, 4.5], [41, 4, 4.5], [14, 9, 4], [50, 9, 4]].forEach(([cx, top, hw]) => g.poly([[cx - hw, 15], [cx, top], [cx + hw, 15]], 'a'));
  // 얼굴: 어둠 속의 거대한 외눈
  g.ellipse(32, 33, 19, 16, 'v');
  g.ellipse(32, 31, 11, 11, 'x'); g.ellipse(32, 31, 9.5, 9.5, 'e'); g.ellipse(32, 31, 7, 7, 'f');
  g.ellipse(30.2, 29, 2.6, 2.6, 'w');
  // 입과 이빨
  g.rect(19, 46, 26, 3, 'k');
  for (let k = 0; k < 7; k++) {
    const x = 20.5 + k * 3.6; g.poly([[x, 46], [x + 3.4, 46], [x + 1.7, 50 + (k % 2 ? 0 : 3)]], 'w');
  }
  g.outline('x');
  return g.rows();
})();
S.bossHand = (function () {
  const g = new G(14, 18);
  g.ellipse(7, 12, 6, 6, 'c');
  for (let j = 0; j < 3; j++) g.poly([[1.5 + j * 4, 8], [3.2 + j * 4, 0.5], [4.8 + j * 4, 8]], 'a');
  g.shade('c', 'm', 'c', 7, 12, 7, 7, 0.4, 9);
  g.outline('x');
  return g.rows();
})();

module.exports = { S };

if (require.main === module) {
  const mode = process.argv[2];
  if (mode === 'preview') {
    const names = process.argv.slice(3).length ? process.argv.slice(3) : Object.keys(S);
    names.forEach(n => { console.log('--- ' + n + ' (' + S[n][0].length + 'x' + S[n].length + ')'); console.log(S[n].join('\n')); });
  } else if (mode === 'cpp') {
    let out = '// 이 파일은 도트 스프라이트 격자 데이터다. 문자 하나 = 픽셀 하나, \'.\' = 투명.\n// 색은 Art.cpp 의 팔레트 표에서 문자별로 지정한다. 손으로 직접 고쳐도 된다.\n#include "art/SpriteData.h"\n\nnamespace gengar::art::data {\n\n';
    for (const [name, rows] of Object.entries(S)) {
      out += 'const std::vector<std::string>& ' + name[0].toUpperCase() + name.slice(1) + '() {\n  static const std::vector<std::string> rows = {\n';
      rows.forEach(r => out += '      "' + r + '",\n');
      out += '  };\n  return rows;\n}\n\n';
    }
    out += '}  // namespace gengar::art::data\n';
    fs.writeFileSync(process.argv[3], out);
    // 헤더
    let h = '#pragma once\n// 도트 스프라이트 격자 데이터 (SpriteData.cpp). 팔레트는 Art.cpp.\n#include <string>\n#include <vector>\n\nnamespace gengar::art::data {\n\n';
    for (const name of Object.keys(S)) h += 'const std::vector<std::string>& ' + name[0].toUpperCase() + name.slice(1) + '();\n';
    h += '\n}  // namespace gengar::art::data\n';
    fs.writeFileSync(process.argv[4], h);
    console.log('written', Object.keys(S).length, 'sprites');
  }
}
