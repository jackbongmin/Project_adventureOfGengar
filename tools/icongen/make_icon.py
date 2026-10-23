# exe 아이콘 생성기: 게임 속 보물 모자 도트(SpriteData.cpp 의 TreasureHat + Art.cpp 의 kTreasureHat 팔레트)를
# 그대로 읽어서 res/windows/gengar.ico 를 만든다. 크기마다 정수 배율로 키워서 도트가 번지지 않는다.
#   사용법: python tools/icongen/make_icon.py   (Pillow 필요)
import re
from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
SPRITES = (ROOT / "Source/Private/art/SpriteData.cpp").read_text(encoding="utf-8")
ART = (ROOT / "Source/Private/art/Art.cpp").read_text(encoding="utf-8")
OUT = ROOT / "res/windows/gengar.ico"
SIZES = [16, 24, 32, 48, 64, 128, 256]


def read_rows():
    body = re.search(r"TreasureHat\(\)\s*\{.*?rows\s*=\s*\{(.*?)\};", SPRITES, re.S).group(1)
    return re.findall(r'"([^"]+)"', body)


def read_palette():
    body = re.search(r"kTreasureHat\s*=\s*\{(.*?)\};", ART, re.S).group(1)
    return {k: tuple(int(h[i:i + 2], 16) for i in (0, 2, 4)) for k, h in re.findall(r"\{'(.)',\s*Hex\(0x([0-9a-fA-F]{6})\)\}", body)}


def render(rows, pal, size):
    w, h = len(rows[0]), len(rows)
    scale = max(1, size // max(w, h))
    img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    ox, oy = (size - w * scale) // 2, (size - h * scale) // 2
    for y, row in enumerate(rows):
        for x, ch in enumerate(row):
            if ch == ".":
                continue
            r, g, b = pal[ch]
            for dy in range(scale):
                for dx in range(scale):
                    img.putpixel((ox + x * scale + dx, oy + y * scale + dy), (r, g, b, 255))
    return img


def main():
    rows, pal = read_rows(), read_palette()
    images = [render(rows, pal, s) for s in SIZES]
    OUT.parent.mkdir(parents=True, exist_ok=True)
    images[-1].save(OUT, format="ICO", sizes=[(s, s) for s in SIZES], append_images=images[:-1])
    images[-1].save(OUT.with_suffix(".png"))  # 미리보기용
    print(f"OK -> {OUT}")


if __name__ == "__main__":
    main()
