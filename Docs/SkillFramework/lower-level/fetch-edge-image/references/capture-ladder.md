# Capture Ladder

## Preferred Order

### 1. Direct Asset Download

If the active Edge tab URL already points to an image file such as `.png`, `.jpg`, `.webp`, `.gif`, or `.svg`, save that asset directly.

This is preferable because it removes browser chrome and avoids Retina scaling surprises.

### 2. Front Window Screenshot

If the active tab is a normal webpage or app route, capture the front Edge window using:

- tab title from AppleScript
- active tab URL from AppleScript
- window bounds from AppleScript
- `screencapture` for the actual PNG

This project intentionally avoids depending on `System Events`, because that permission may not be available.

### 3. Crop After Capture

If the review target is only one region of the page, first keep the full verified source capture. Then crop from that file instead of gambling on a second ad hoc capture.

## Output Convention

Default output location:

- `<cwd>/ignore/edge-captures/<timestamp>/`

Artifacts:

- `edge-capture.png` or `edge-capture-asset.<ext>`
- `edge-capture.txt`

The metadata file records:

- capture mode
- active tab title
- active tab URL
- CSS bounds
- final image path

## Caveats

- `screencapture` may emit device-pixel output that is larger than CSS bounds on Retina displays.
- Window screenshots may include browser chrome. Call that out before passing the file into aesthetic analysis.
- A screenshot is evidence of what was visible, not proof of the underlying DOM structure.
