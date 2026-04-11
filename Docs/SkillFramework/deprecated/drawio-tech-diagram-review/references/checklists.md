# Review Checklists

Use these checklists after reading the page docs and again after rendering the updated preview.

## Blocker Scan

- Are any two nodes overlapping?
- Is any label clipped, cropped, or visually cramped inside its box?
- Is any node unintentionally touching another node, line, or frame edge?
- Does any line stab through a box body instead of connecting at its intended side?
- Did a local fix create a new collision somewhere else on the page?

If any answer is yes, stop. Fix blockers before evaluating macro composition.

## Page Contract Check

- Can I name the previous-page skeleton in one sentence?
- Can I name this page's new structural idea in one sentence?
- Can I name the next-page idea that must not appear early?
- Do I know which node is the receiver plane?
- Do I know which line is the primary line?
- Do I know which edges should stay neutral and which should be emphasized?
- Is the page doc explicit about node add/move/delete/keep decisions and the key routing reasons?
- Did the page doc account for every important previous-page node, including any intentionally hidden or removed ones?
- Is the rationale written in a table that lets me scan subject-by-subject decisions quickly?

If any answer is no, stop and read the docs again.

## Macro Composition Check

- Does the page feel balanced around the receiver plane?
- Is the main upper composition centered by meaningful mass, not by the widest stray node?
- If two groups compete horizontally, did I try vertical layering before pushing them far apart?
- Is the receiver plane wide enough to accept inputs cleanly?
- Did I preserve small optical gaps instead of literal touching?
- If this page feels vertically cramped, did I test whether the whole sequence band should move instead of only this page?
- If this page hides a runtime receiver, did I still preserve the empty band that later pages need for continuity?

## Routing Check

- Does the primary line have the cleanest channel on the page?
- Are same-family lines using one routing language?
- Did I avoid extra bends that exist only because of poor node placement?
- Did I avoid accidental total-bus semantics when the page should still feel fragmented?
- Did I avoid lines cutting through the primary reading area?
- Can I explain why each important line enters from that side and why its bend count is necessary?
- If there was an obvious alternative route, did I explain why this route wins instead of merely asserting it?

## Narrative Check

- Does the page still read as an evolution from the previous page?
- Does it still read as an evolution when compared to both adjacent pages at once, not just one side?
- Did I accidentally borrow the next page's solution too early?
- Is the current page's emphasized action visually clear within 2 seconds?
- Are inherited skeleton edges visually quieter than new highlighted action?
- If an earlier page points here with a dashed future target, does that ghost still plausibly land in the same region after my changes?
- If a ghost target foreshadows a later receiver plane, does it preserve the same receiver-plane type and vertical band instead of drifting from an inserted layer into a right-end terminal box?
- If a node is input-like, did I verify whether it is a runtime payload or an upper-band descriptor before placing it?
- Did I keep the draft schematic enough that it still reads as a reasoning surface rather than a fake finished stage?
- If I inserted a new layer like `PSO`, did I keep older stable skeleton lines identifiable instead of pretending the new layer now owns them?

## Mandatory Self-Review Output

Before reporting back, write these four scores explicitly:

- `空间美感`: Does the frame feel balanced, centered, and intentionally spaced?
- `线条美感`: Do the lines feel clean, disciplined, and visually elegant?
- `表意强度`: Can the audience understand the point from the diagram itself?
- `演讲适配度`: Will this still read clearly on a PPT screen during a talk?

Use `0-10` for each. For every score, add:

- one short reason
- one likely next fix

If any score is below `7`, do not describe the page as polished. If any score is below `5`, revise before presenting unless the human explicitly asked to inspect the bad version.

## Rationalizations To Reject

- "The macro got better, so this overlap is acceptable for now."
- "This one comment is satisfied, so the page is good now."
- "The coordinates are cleaner even if the rendered page feels worse."
- "贴着 means no gap."
- "I can keep the macro imbalance if one local relationship became correct."
- "I can use one shared bus here because it looks cleaner."

If you catch yourself using one of those lines, back up and re-check the page contract.
