---
name: fetch-edge-image
description: Use when the target visual is currently open in Microsoft Edge and you need a local image file before review, comparison, or image analysis.
---

# Fetch Edge Image

This skill turns the current Microsoft Edge tab or front window into a local image artifact.

Use the simplest capture path that yields a trustworthy file.

## When to Use

Use this skill when:

- the user asks to review what is currently shown in Edge
- an image analysis step needs a local file first
- you need a stable screenshot of the current Edge tab before visual critique

Do not use this skill when the image file already exists locally or is already attached in the thread.

## Capture Order

1. If the active Edge tab is already a direct image asset URL, save the asset directly.
2. Otherwise capture the front Edge window with the helper script.
3. If the target is only part of the page, crop from the saved capture after you have a verified source image.

## Default Command

From the repo:

```bash
bash Docs/SkillFramework/lower-level/fetch-edge-image/scripts/capture_edge_window.sh
```

From the runtime mirror:

```bash
bash ~/.codex/skills/fetch-edge-image/scripts/capture_edge_window.sh
```

The helper prints the saved image path. It also writes a sidecar metadata file with the tab title, URL, and CSS bounds used for capture.

## Required Output

When using this skill, report:

- the saved image path
- whether the result came from direct download or screenshot capture
- the active tab URL
- whether browser chrome is included in the capture

## Hard Bans

- Do not claim capture succeeded unless a file exists on disk.
- Do not rely on `System Events` UI automation as a hard requirement.
- Do not analyze a browser screenshot as if browser chrome were part of the target design.
- Do not skip provenance; the next skill needs to know what was captured.

## Reference

See [references/capture-ladder.md](references/capture-ladder.md) for the exact fallback order and caveats.
