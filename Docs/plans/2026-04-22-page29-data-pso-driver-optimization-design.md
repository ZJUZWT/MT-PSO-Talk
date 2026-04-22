# Page29 Data PSO Driver Optimization Design

**Goal:** Insert a new data evidence page between `page_29` and `page_30` so the talk can show raw benchmark evidence for "PSO驱动层的激进优化" before transitioning into the standalone PSO reading page.

**Approved Direction:** Add a new inserted step named `page_29_data`. Keep the existing `page_29` governance evidence page and the existing `page_30` reading page intact. The new page should emphasize that PSO is not only a state-packaging mechanism; it also enables more aggressive driver-side optimization on some desktop drivers, while some mobile drivers appear to trim or skip that optimization depth.

## Design Summary

- Use `page_29_data` naming to match the repo's existing inserted-step convention such as `page_04_data`.
- Place the new step in the awareness / governance session directly after `page_29`.
- Keep the page visually consistent with the current late-tail shell:
  - no new background plate
  - fade out previous page, fade in this data page
  - fade out this data page, fade in the current `page_30`
- Show only the two requested loop-count rows:
  - minimum: `loopCount=10`
  - maximum: `loopCount=5000`
- Show both platforms and both APIs:
  - PC: `Vulkan` and `OpenGL`
  - Android: `Vulkan` and `GLES`
- Do not spend space on empty / baseline validation rows.
- Split the page into two information bands:
  - upper band: compact raw-data comparison table
  - lower band: short contrast summary and takeaway

## Content Contract

### Title

- `PSO驱动层的激进优化`

### Upper data region

- Two side-by-side platform cards:
  - `PC（RTX 3080）`
  - `Android（Adreno）`
- Each card contains four measurement columns:
  - `VK mask=0`
  - `VK mask=RGBA`
  - `GL/GLES mask=0`
  - `GL/GLES mask=RGBA`
- Each card contains two rows:
  - `loop=10`
  - `loop=5000`
- This keeps the page rooted in raw evidence rather than ratio-only storytelling.

### Lower summary region

- Three short summary bullets / strips:
  - `PC：Vulkan 在 mask=0 下几乎压平 heavy loop，说明驱动吃到了 PSO 的激进优化。`
  - `Android：Vulkan / GLES 都没有出现同等级的跨 stage 裁剪。`
  - `结论：PSO 的价值不只是状态打包，也是给驱动更多编译期信息；但优化能否兑现，取决于驱动实现。`

## Affected Areas

- `SlideApp` storyboard metadata
- Remotion late-tail page scene and timeline anchors
- Formal review registry and late-tail geometry tests
- Formal page scripts under `Docs/剧本/`
- Transition workload JSONs for the new handoffs

## Acceptance Criteria

- `page_29_data` is inserted between `page_29` and `page_30`.
- The new page title is `PSO驱动层的激进优化`.
- The table only includes `loop=10` and `loop=5000`.
- The table includes PC + Android and Vulkan + OpenGL/GLES together.
- The lower summary communicates:
  - desktop drivers may use PSO information for aggressive optimization
  - mobile drivers may not expose the same optimization depth
- New page insertion does not compress existing later-page durations.
- Required audits pass, including:
  - `python3 scripts/slide-geometry-harness/audit_storyboard_sync.py`
