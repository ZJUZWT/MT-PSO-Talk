# PSO Video Demo Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Build a short single-canvas animation video that shows the PSO model evolving from `A -> f(x) -> B` into an inline-material path and then a shared-code path, while also exporting per-beat clips that can autoplay inside PowerPoint slides.

**Architecture:** Use one Python generator script under `Docs/scripts` that renders each frame with Pillow from structured scene state. Model each concept node as a rounded-box object with interpolated position, size, opacity, and label style inside a right-side animation stage on a white 1920x1080 canvas. Export PNG frames, an animated GIF preview, a master MP4, and segmented MP4 clips using lightweight Python-side encoders.

**Tech Stack:** Python 3.11, `unittest`, Pillow, NumPy, `imageio`, `imageio-ffmpeg`.

---

### Task 1: Lock the refreshed animation contract in a failing test

**Files:**
- Create: `Docs/scripts/test_generate_pso_video_demo.py`
- Test: `Docs/scripts/test_generate_pso_video_demo.py`

**Step 1: Write the failing storyboard contract test**

```python
from generate_pso_video_demo import build_storyboard


def test_storyboard():
    beats = build_storyboard()
    assert [beat.beat_id for beat in beats] == [
        "base_formula",
        "open_fx",
        "inline_material",
        "shared_code",
    ]
    assert FPS == 60
```

**Step 2: Run test to verify it fails**

Run: `python3 -m unittest discover -s Docs/scripts -p 'test_*.py' -v`
Expected: FAIL because the old generator does not yet expose the refreshed
segment-aware output contract.

### Task 2: Implement the generator and asset outputs

**Files:**
- Create: `Docs/scripts/generate_pso_video_demo.py`
- Modify: `Docs/scripts/test_generate_pso_video_demo.py`
- Test: `Docs/scripts/test_generate_pso_video_demo.py`

**Step 1: Implement structured beat, segment, and output-path helpers**

```python
def build_storyboard():
    return [
        BeatSpec(beat_id="base_formula", ...),
        BeatSpec(beat_id="open_fx", ...),
        BeatSpec(beat_id="inline_material", ...),
        BeatSpec(beat_id="shared_code", ...),
    ]
```

**Step 2: Implement frame rendering**

```python
def render_frame(t):
    scene = scene_state_at(t)
    draw_nodes(scene)
    draw_arrows(scene)
```

**Step 3: Export assets**

Run: `python3 Docs/scripts/generate_pso_video_demo.py`
Expected: A master GIF, master MP4, poster PNG, frame directory, and segmented
MP4 clips are generated under `Docs/`.

**Step 4: Run the tests again to verify they pass**

Run: `python3 -m unittest discover -s Docs/scripts -p 'test_*.py' -v`
Expected: PASS with storyboard order and output-path assertions succeeding.

### Task 3: Verify the generated demo outputs

**Files:**
- Output: `Docs/PSO-video-demo.mp4`
- Output: `Docs/PSO-video-demo.gif`
- Output: `Docs/PSO-video-demo-poster.png`
- Output: `Docs/PSO-video-demo-frames/`
- Output: `Docs/PSO-video-demo-segments/`

**Step 1: Re-run the generator from a clean command**

Run: `python3 Docs/scripts/generate_pso_video_demo.py`
Expected: All output files regenerate without errors.

**Step 2: Inspect output existence**

Run: `ls -la Docs/PSO-video-demo.mp4 Docs/PSO-video-demo.gif Docs/PSO-video-demo-poster.png Docs/PSO-video-demo-frames Docs/PSO-video-demo-segments`
Expected: All artifacts exist.
