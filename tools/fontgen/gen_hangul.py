# 한글 글리프 굽기: 소스의 문자열 리터럴에 쓰인 한글 음절만 골라 갈무리7 BDF 에서 7x7 비트맵을 뽑아
# Source/Private/platform/HangulGlyphs.cpp 로 출력한다. 번역(LocaleKo.cpp)에 한글을 추가했으면 다시 돌린다.
#   사용: python tools/fontgen/gen_hangul.py <Galmuri7.bdf 경로>
#   폰트: 갈무리 (https://github.com/quiple/galmuri), SIL Open Font License 1.1 (res/fonts/Galmuri-LICENSE.txt)
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
OUT = ROOT / "Source" / "Private" / "platform" / "HangulGlyphs.cpp"
LITERAL = re.compile(r'"((?:[^"\\\n]|\\.)*)"')


def used_syllables():
    chars = set()
    for path in (ROOT / "Source").rglob("*"):
        if path.suffix not in (".cpp", ".h") or path == OUT:
            continue
        for text in LITERAL.findall(path.read_text(encoding="utf-8")):
            chars.update(c for c in text if 0xAC00 <= ord(c) <= 0xD7A3)
    return sorted(chars)


def load_bdf(path):
    glyphs, cur, rows = {}, None, None
    for line in pathlib.Path(path).read_text(encoding="utf-8", errors="replace").splitlines():
        if line.startswith("ENCODING"):
            cur = {"cp": int(line.split()[1])}
        elif line.startswith("BBX") and cur is not None:
            cur["bbx"] = list(map(int, line.split()[1:]))
        elif line == "BITMAP":
            rows = []
        elif line == "ENDCHAR":
            cur["rows"] = rows
            glyphs[cur["cp"]] = cur
            cur, rows = None, None
        elif rows is not None:
            rows.append(line)
    return glyphs


def to_rows(g):
    # 기준선 위 0~6 줄만 쓴다: 줄 r(0=위)는 기준선에서 6-r 높이
    w, h, ox, oy = g["bbx"]
    out = [0] * 7
    for r, hexrow in enumerate(g["rows"]):
        bits = int(hexrow, 16)
        nbits = len(hexrow) * 4
        y = oy + (h - 1 - r)
        if not 0 <= y <= 6:
            continue
        for c in range(w):
            if bits >> (nbits - 1 - c) & 1 and 0 <= ox + c < 8:
                out[6 - y] |= 0x80 >> (ox + c)
    return out


def main():
    glyphs = load_bdf(sys.argv[1])
    chars = used_syllables()
    missing = [c for c in chars if ord(c) not in glyphs]
    if missing:
        print("missing in font:", "".join(missing))
    lines = [
        "// 자동 생성 파일 (tools/fontgen/gen_hangul.py). 직접 고치지 말 것.",
        "// 글리프: 갈무리7 (Galmuri7) Copyright (c) 2019-2025 Lee Minseo (quiple@quiple.dev), SIL Open Font License 1.1",
        '#include "platform/PixelFont.h"',
        "",
        "#include <algorithm>",
        "#include <iterator>",
        "",
        "namespace gengar {",
        "namespace {",
        "",
        "struct HangulDef {",
        "  uint32_t cp;",
        "  uint8_t rows[7];",
        "};",
        "",
        f"// 게임 문자열에 쓰인 한글 {len(chars) - len(missing)}자 (코드 순으로 정렬: 이진 탐색)",
        "const HangulDef kHangul[] = {",
    ]
    for c in chars:
        if ord(c) not in glyphs:
            continue
        rows = ", ".join(f"0x{v:02x}" for v in to_rows(glyphs[ord(c)]))
        lines.append(f"    {{0x{ord(c):04x}, {{{rows}}}}},  // {c}")
    lines += [
        "};",
        "",
        "}  // namespace",
        "",
        "const uint8_t* HangulGlyph(uint32_t cp) {",
        "  const auto it = std::lower_bound(std::begin(kHangul), std::end(kHangul), cp,",
        "                                  [](const HangulDef& d, uint32_t v) { return d.cp < v; });",
        "  return it != std::end(kHangul) && it->cp == cp ? it->rows : nullptr;",
        "}",
        "",
        "}  // namespace gengar",
        "",
    ]
    OUT.write_text("\n".join(lines), encoding="utf-8")
    print(f"wrote {OUT} ({len(chars) - len(missing)} glyphs)")


if __name__ == "__main__":
    main()
