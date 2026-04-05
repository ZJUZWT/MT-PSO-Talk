#!/usr/bin/env python3

from __future__ import annotations

import shutil
import subprocess
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
DOCS_PREVIEW_DIR = REPO_ROOT / ".generated" / "storyboard-previews" / "docs"
ANIMATION_DIR = REPO_ROOT / ".generated" / "storyboard-previews" / "animations"

SCENES = [
    "02-A-fx-B.png",
    "03-输入到GPU.png",
    "04-OpenGL.png",
    "05-Vulkan-PSO.png",
    "06-UE-映射.png",
    "07-Material-ShaderCode.png",
    "08-ShaderCodeLib.png",
    "09-UE-PSO.png",
]

TRANSITIONS = [
    "fade",
    "wipeup",
    "fade",
    "slideleft",
    "wipedown",
    "slideleft",
    "wipeup",
]

SCENE_SECONDS = 0.9
TRANSITION_SECONDS = 0.42
FPS = 60
WIDTH = 1280
HEIGHT = 720


def require_binary(name: str) -> str:
    resolved = shutil.which(name)
    if resolved is None:
        raise SystemExit(f"Missing required binary: {name}")
    return resolved


def main() -> None:
    ffmpeg = require_binary("ffmpeg")
    ANIMATION_DIR.mkdir(parents=True, exist_ok=True)

    missing = [name for name in SCENES if not (DOCS_PREVIEW_DIR / name).exists()]
    if missing:
        raise SystemExit(
            "Missing storyboard preview PNGs:\n" + "\n".join(f"- {name}" for name in missing)
        )

    mp4_path = ANIMATION_DIR / "storyboard-transition-v1.mp4"
    gif_path = ANIMATION_DIR / "storyboard-transition-v1.gif"

    command = [ffmpeg, "-y"]
    for name in SCENES:
        command.extend(
            [
                "-loop",
                "1",
                "-t",
                f"{SCENE_SECONDS + TRANSITION_SECONDS:.2f}",
                "-i",
                str(DOCS_PREVIEW_DIR / name),
            ]
        )

    filters: list[str] = []
    for index in range(len(SCENES)):
        filters.append(
            f"[{index}:v]scale={WIDTH}:{HEIGHT}:force_original_aspect_ratio=decrease,"
            f"pad={WIDTH}:{HEIGHT}:(ow-iw)/2:(oh-ih)/2:color=#fcfbf7,"
            f"format=rgba,setsar=1[v{index}]"
        )

    current_label = "v0"
    offset = SCENE_SECONDS
    for index, transition in enumerate(TRANSITIONS, start=1):
        output_label = f"x{index}"
        filters.append(
            f"[{current_label}][v{index}]xfade=transition={transition}:"
            f"duration={TRANSITION_SECONDS:.2f}:offset={offset:.2f}[{output_label}]"
        )
        current_label = output_label
        offset += SCENE_SECONDS

    command.extend(
        [
            "-filter_complex",
            ";".join(filters),
            "-map",
            f"[{current_label}]",
            "-r",
            str(FPS),
            "-pix_fmt",
            "yuv420p",
            str(mp4_path),
        ]
    )

    subprocess.run(command, check=True)
    subprocess.run(
        [
            ffmpeg,
            "-y",
            "-i",
            str(mp4_path),
            "-vf",
            "fps=24,scale=960:-1:flags=lanczos,split[s0][s1];[s0]palettegen[p];[s1][p]paletteuse",
            str(gif_path),
        ],
        check=True,
    )

    print(mp4_path)
    print(gif_path)


if __name__ == "__main__":
    main()
