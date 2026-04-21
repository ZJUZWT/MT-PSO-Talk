#!/usr/bin/env python3
from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont


WIDTH = 1280
HEIGHT = 720

BG = (252, 251, 247, 255)
PANEL = (255, 251, 246, 245)
PANEL_ACCENT = (248, 236, 226, 250)
STROKE = (52, 67, 82, 190)
TEXT = (34, 48, 61, 255)
TEXT_SOFT = (93, 109, 124, 255)
ACCENT = (208, 107, 68, 255)
GOOD = (88, 156, 117, 255)


@dataclass(frozen=True)
class FontPack:
  title: ImageFont.FreeTypeFont
  section: ImageFont.FreeTypeFont
  body: ImageFont.FreeTypeFont
  value: ImageFont.FreeTypeFont
  small: ImageFont.FreeTypeFont


def _load_font(size: int) -> ImageFont.FreeTypeFont:
  candidates = [
    "/System/Library/Fonts/PingFang.ttc",
    "/System/Library/Fonts/Supplemental/Arial Unicode.ttf",
    "/System/Library/Fonts/Supplemental/Arial.ttf",
  ]
  for candidate in candidates:
    path = Path(candidate)
    if path.exists():
      return ImageFont.truetype(str(path), size=size)
  return ImageFont.load_default()


def load_fonts() -> FontPack:
  return FontPack(
    title=_load_font(34),
    section=_load_font(24),
    body=_load_font(21),
    value=_load_font(34),
    small=_load_font(18),
  )


def panel(
  draw: ImageDraw.ImageDraw,
  box: tuple[int, int, int, int],
  radius: int,
  fill: tuple[int, int, int, int] = PANEL,
  outline: tuple[int, int, int, int] = STROKE,
  width: int = 3,
) -> None:
  draw.rounded_rectangle(box, radius=radius, fill=fill, outline=outline, width=width)


def text(
  draw: ImageDraw.ImageDraw,
  xy: tuple[int, int],
  value: str,
  font: ImageFont.FreeTypeFont,
  fill: tuple[int, int, int, int] = TEXT,
  anchor: str = "la",
) -> None:
  draw.text(xy, value, font=font, fill=fill, anchor=anchor)


def human_kib(size: int) -> str:
  return f"{round(size / 1024):d}K"


def main() -> None:
  repo_root = Path(__file__).resolve().parents[1]
  output_generated = repo_root / "generated" / "inline-vs-shared-cook-evidence-slide-v3.png"
  output_public = (
    repo_root / "SlideApp" / "public" / "supplement" / "inline-vs-shared-cook-evidence-v3.png"
  )
  output_generated.parent.mkdir(parents=True, exist_ok=True)
  output_public.parent.mkdir(parents=True, exist_ok=True)

  inline_dir = repo_root / "Supplement" / "Cook结果对比" / "InlineShaderCode"
  shared_dir = repo_root / "Supplement" / "Cook结果对比" / "SharedShaderCode" / "Material"
  files = ["MyMaterial.uexp", "MyMaterial_Inst.uexp", "MyMaterial_SameInst.uexp"]

  rows: list[tuple[str, int, int, int, float]] = []
  for name in files:
    inline_size = (inline_dir / name).stat().st_size
    shared_size = (shared_dir / name).stat().st_size
    delta = inline_size - shared_size
    reduce_pct = (delta / inline_size) * 100 if inline_size else 0
    rows.append((name, inline_size, shared_size, delta, reduce_pct))

  gl_archive = (
    repo_root
    / "Supplement"
    / "Cook结果对比"
    / "SharedShaderCode"
    / "ShaderCode"
    / "ShaderArchive-PSO-GLSL_ES3_1_ANDROID-OPENGL_ES3_1_ANDROID.ushaderbytecode"
  ).stat().st_size
  vk_archive = (
    repo_root
    / "Supplement"
    / "Cook结果对比"
    / "SharedShaderCode"
    / "ShaderCode"
    / "ShaderArchive-PSO-SF_VULKAN_ES31_ANDROID-VULKAN_ES3_1_ANDROID.ushaderbytecode"
  ).stat().st_size

  img = Image.new("RGBA", (WIDTH, HEIGHT), BG)
  draw = ImageDraw.Draw(img)
  fonts = load_fonts()

  left = (42, 40, 360, 450)
  center = (388, 40, 898, 450)
  right = (926, 40, WIDTH - 42, 450)
  bottom = (42, 480, WIDTH - 42, HEIGHT - 36)

  panel(draw, left, radius=24)
  panel(draw, center, radius=24)
  panel(draw, right, radius=24)
  panel(draw, bottom, radius=24)

  text(draw, (64, 86), "InlineShaderCode", fonts.section, fill=ACCENT)
  text(draw, (64, 116), "每个材质都带一份 code", fonts.small, fill=TEXT_SOFT)

  text(draw, (410, 86), ".uexp 体积对比", fonts.section)
  text(draw, (410, 116), "Shared 后 .uexp 明显变小", fonts.small, fill=TEXT_SOFT)

  text(draw, (948, 86), "SharedShaderCode", fonts.section, fill=GOOD)
  text(draw, (948, 116), "材质瘦身 + ShaderArchive 承载", fonts.small, fill=TEXT_SOFT)

  row_top = 148
  row_h = 82
  for index, (name, inline_size, shared_size, _delta, reduce_pct) in enumerate(rows[:2]):
    y0 = row_top + index * (row_h + 18)
    box = (410, y0, 878, y0 + row_h)
    panel(draw, box, radius=16, fill=(250, 247, 241, 255), width=2)
    label = "MyMaterial" if index == 0 else "MyMaterial_Inst"
    text(draw, (430, y0 + 24), label, fonts.body)
    text(draw, (430, y0 + 72), human_kib(inline_size), fonts.value, fill=ACCENT, anchor="lb")
    text(draw, (575, y0 + 62), "→", fonts.value, fill=TEXT_SOFT, anchor="lm")
    text(draw, (640, y0 + 72), human_kib(shared_size), fonts.value, fill=GOOD, anchor="lb")
    text(draw, (780, y0 + 62), f"-{reduce_pct:.0f}%", fonts.body, fill=GOOD, anchor="lm")

  y_inline = 148
  for name, inline_size, _, _, _ in rows:
    panel(draw, (64, y_inline, 340, y_inline + 66), radius=14, fill=(250, 247, 241, 255), width=2)
    short_name = name.replace(".uexp", "")
    text(draw, (84, y_inline + 24), short_name, fonts.small)
    text(draw, (320, y_inline + 44), human_kib(inline_size), fonts.small, anchor="rm")
    y_inline += 78

  y_shared = 148
  for name, _, shared_size, _, _ in rows:
    panel(draw, (948, y_shared, 1230, y_shared + 66), radius=14, fill=(245, 251, 246, 255), width=2)
    short_name = name.replace(".uexp", "")
    text(draw, (968, y_shared + 24), short_name, fonts.small)
    text(draw, (1214, y_shared + 44), human_kib(shared_size), fonts.small, anchor="rm")
    y_shared += 78

  panel(draw, (948, 386, 1230, 420), radius=12, fill=(237, 248, 240, 255), width=2, outline=GOOD)
  text(draw, (968, 407), "GL PSO Archive", fonts.small, fill=TEXT, anchor="lm")
  text(draw, (1214, 407), f"{gl_archive / (1024 * 1024):.1f}M", fonts.small, fill=GOOD, anchor="rm")

  panel(draw, (948, 426, 1230, 460), radius=12, fill=(237, 248, 240, 255), width=2, outline=GOOD)
  text(draw, (968, 447), "Vulkan PSO Archive", fonts.small, fill=TEXT, anchor="lm")
  text(draw, (1214, 447), f"{vk_archive / (1024 * 1024):.1f}M", fonts.small, fill=GOOD, anchor="rm")

  text(draw, (64, 522), "补充证据：Hash 复用", fonts.section, fill=ACCENT)
  text(draw, (64, 550), "同参数实例（Inst / SameInst）共享同一套 ShaderMapHash。", fonts.small, fill=TEXT_SOFT)
  panel(draw, (64, 570, 618, 676), radius=14, fill=(250, 247, 241, 255), width=2)
  text(draw, (84, 596), "OPENGL_ES3_1_ANDROID", fonts.small, fill=GOOD)
  text(draw, (84, 640), "BC10CB48...B4A6DB57", fonts.small, fill=TEXT)

  panel(draw, (660, 570, 1230, 676), radius=14, fill=(250, 247, 241, 255), width=2)
  text(draw, (680, 596), "VULKAN_ES3_1_ANDROID", fonts.small, fill=GOOD)
  text(draw, (680, 640), "8DD283A7...E60A34B5", fonts.small, fill=TEXT)

  img.save(output_generated)
  img.save(output_public)
  print(output_generated)
  print(output_public)


if __name__ == "__main__":
  main()
