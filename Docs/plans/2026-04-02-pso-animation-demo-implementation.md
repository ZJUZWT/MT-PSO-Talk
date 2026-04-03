# PSO Animation Demo Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Generate a standalone 4-slide animated PowerPoint demo that shows the `A -> f(x) -> B` model evolving into inline and shared-code PSO concepts.

**Architecture:** Keep the storyboard as structured Python data in one generator script under `Docs/scripts`, then render slides from reusable shape helpers so all slide positions remain aligned. Export the deck, per-slide preview PNGs, and a stitched preview asset from the same script.

**Tech Stack:** Python 3.11, `unittest`, `aspose-slides`, `aspose.pydrawing`, Pillow.

---

### Task 1: Lock the storyboard contract in a failing test

**Files:**
- Create: `Docs/scripts/test_generate_pso_animation_demo.py`
- Test: `Docs/scripts/test_generate_pso_animation_demo.py`

**Step 1: Write the failing storyboard test**

```python
from generate_pso_animation_demo import build_storyboard


def test_storyboard():
    storyboard = build_storyboard()
    assert len(storyboard) == 4
```

**Step 2: Run the test to verify it fails**

Run: `python3 -m unittest discover -s Docs/scripts -p 'test_*.py' -v`
Expected: FAIL because the generator module does not exist yet.

**Step 3: Commit**

```bash
git add Docs/scripts/test_generate_pso_animation_demo.py
git commit -m "test: lock PSO animation demo storyboard"
```

### Task 2: Implement the generator and exports

**Files:**
- Create: `Docs/scripts/generate_pso_animation_demo.py`
- Modify: `Docs/scripts/test_generate_pso_animation_demo.py`
- Test: `Docs/scripts/test_generate_pso_animation_demo.py`

**Step 1: Implement the structured storyboard**

```python
def build_storyboard():
    return [
        SlideSpec(step_id="base_formula", ...),
        SlideSpec(step_id="decompose_fx", ...),
        SlideSpec(step_id="inline_material", ...),
        SlideSpec(step_id="share_shader_code", ...),
    ]
```

**Step 2: Implement slide rendering helpers**

```python
def render_slide(slide, spec):
    draw_base_chain(slide)
    if spec.show_breakdown:
        draw_breakdown(slide)
```

**Step 3: Export the deck and preview assets**

Run: `DYLD_LIBRARY_PATH=/opt/homebrew/lib python3 Docs/scripts/generate_pso_animation_demo.py`
Expected: A `.pptx`, preview PNGs, and a preview asset appear under `Docs/`.

**Step 4: Run the test again to verify it passes**

Run: `python3 -m unittest discover -s Docs/scripts -p 'test_*.py' -v`
Expected: PASS with the storyboard order and output paths validated.

### Task 3: Verify the generated demo assets

**Files:**
- Output: `Docs/PSO-animation-demo.pptx`
- Output: `Docs/PSO-animation-demo-previews/*.png`
- Output: `Docs/PSO-animation-demo-preview.gif`

**Step 1: Re-run the generator from a clean command**

Run: `DYLD_LIBRARY_PATH=/opt/homebrew/lib python3 Docs/scripts/generate_pso_animation_demo.py`
Expected: Deck and preview assets regenerate without errors.

**Step 2: Inspect the output paths**

Run: `ls -la Docs/PSO-animation-demo.pptx Docs/PSO-animation-demo-preview.gif Docs/PSO-animation-demo-previews`
Expected: All output artifacts exist.
