# Page 19 Strong Parent Tree Design

**Goal:** Rebuild `page_19` so `stable.upipelinecache` becomes the parent container of the UE PSO children, freeing horizontal space for the right-side memory and disk groups while preserving the existing reading logic.

**Context**

- Current page structure uses two left columns: `stable.upipelinecache` and a separate `UE PSO` column.
- Harness facts show the page is not blocked by overlap, crossing, or text overflow.
- The real issue is spatial pressure on the right side, especially the `memory -> disk` gap, which is currently the tightest horizontal breathing slot.
- The user wants the algorithm to follow tree semantics rather than page-specific visual patching.

**Approved Direction**

- Use the `strong parent` version.
- `stable.upipelinecache` becomes the parent container.
- `PSO 1 / PSO 2 / PSO ...` become child nodes inside that parent container.
- Remove the standalone `UE PSO` column from the main left-to-right chain.
- Keep the external reading line as:
  `stable(with UE PSO children) -> GPU -> 内存中 PSO -> 硬盘中的 PSO`

**Why This Direction**

- It changes the page from a flat lane layout into a real tree on the left, which matches the intended semantics better.
- It releases roughly one intermediate column of horizontal budget.
- That reclaimed width can be used to move `GPU`, `内存中 PSO`, and `硬盘中的 PSO` leftward and to reopen the breathing slot between the two right groups.
- It improves the algorithmic model as well as the page composition, because containment and sibling rules become explicit in the harness data model.

**Geometry Targets**

- Convert the left structure into a parent-child tree:
  - parent: `stable`
  - children: `ue-1`, `ue-2`, `ue-3`
- Keep hard gates at zero:
  - `overlapCount = 0`
  - `crossingCount = 0`
  - `nodePierceCount = 0`
  - `textOverflow = 0`
  - `siblingOverlapCount = 0`
  - `childOutOfBoundsCount = 0`
- Increase the visual breathing room on the right:
  - reopen `memory -> disk` horizontal gap from the current tight state to a clearly safer margin
  - reduce the perceived right-heavy mass by shifting the full right block left
- Preserve the main axis clarity:
  - keep the external chain straight
  - keep GPU as the center pivot
  - keep memory and disk groups aligned as parallel containers

**Planned Visual Changes**

- Replace the old `UE PSO` group title with a stronger `stable.upipelinecache` parent card that visually owns the three PSO children.
- Re-stack the three child pills inside the parent with explicit vertical spacing.
- Move the GPU stack left.
- Move the memory and disk groups left and slightly rebalance their widths if needed.
- Keep the API mapping rows and dashed cache edges horizontally aligned.

**Files Expected To Change**

- `SlideApp/src/remotion/pages/Page10Scene.tsx`
- `SlideApp/src/harness/slide-geometry/contracts/page19-r1.ts`
- `Docs/剧本/19-第十九页-正式动画.md`
- related review tests if page geometry assertions need updating

**Verification Plan**

- Run page-scoped tests for the updated geometry.
- Run the formal mechanical review from `page_19`.
- Run `audit_transition_timings.py`.
- Run `audit_storyboard_sync.py`.
- Use real browser-backed review output as the source of truth for the final geometry judgment.
