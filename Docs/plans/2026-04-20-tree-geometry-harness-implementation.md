# Tree Geometry Harness Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** 把 slide geometry harness 升级成树结构审计链路，让所有可见元素都能被同层兄弟碰撞和父子 containment 规则统一约束，并优先消费前台浏览器真实测量结果。

**Architecture:** 先在类型层引入 `GeometryEntity` 和兼容映射，再为 metrics 增加树约束与 browser-truth bounds 入口，随后把 blocker、review artifact 和 formal review 输出接到新指标上。整个过程保持旧 `nodes` 输入兼容，并通过 feature flag 或 dual-report 方式降低迁移风险。

**Tech Stack:** TypeScript + React + Remotion + Vitest + slide geometry harness + repo audit scripts

---

### Task 1: Add tree entities and legacy mapping

**Files:**
- Modify: `SlideApp/src/harness/slide-geometry/render/geometry-sketch-types.ts`
- Modify: `SlideApp/src/harness/slide-geometry/review/geometryMetrics.ts`
- Test: `SlideApp/src/harness/slide-geometry/review/geometryMetrics.test.ts`

**Step 1: Write the failing test**

Add coverage that:

- `entities` input preserves explicit `parentId`
- legacy `nodes` input maps into tree entities
- `renderStyle: "textOnly"` legacy nodes become measurable text entities instead of disappearing

**Step 2: Run test to verify it fails**

Run: `npm --silent --prefix SlideApp test -- src/harness/slide-geometry/review/geometryMetrics.test.ts`

Expected: FAIL because tree entity resolution does not exist yet

**Step 3: Write minimal implementation**

- Add `GeometryEntityKind` and `GeometryEntity`
- Extend `GeometrySketchDefinition` with optional `entities`
- Add `resolveGeometryEntities(sketch)` in `geometryMetrics.ts`
- Keep legacy `SketchNode` support intact

**Step 4: Run test to verify it passes**

Run: `npm --silent --prefix SlideApp test -- src/harness/slide-geometry/review/geometryMetrics.test.ts`

Expected: PASS

**Step 5: Commit**

```bash
git add SlideApp/src/harness/slide-geometry/render/geometry-sketch-types.ts SlideApp/src/harness/slide-geometry/review/geometryMetrics.ts SlideApp/src/harness/slide-geometry/review/geometryMetrics.test.ts
git commit -m "Add tree entity mapping for geometry harness"
```

### Task 2: Add browser-truth entity probe coverage

**Files:**
- Modify: `SlideApp/src/harness/slide-geometry/review/browserGeometryTextProbe.ts`
- Modify: `SlideApp/src/harness/slide-geometry/review/browserGeometryTextProbe.test.ts`
- Modify: `SlideApp/src/harness/slide-geometry/review/geometryReviewArtifact.test.ts`

**Step 1: Write the failing test**

Add assertions that:

- a probed `data-geometry-entity-id` returns entity bounds
- browser bounds override formula bounds when both exist
- free text and edge labels can be probed as first-class entities

**Step 2: Run test to verify it fails**

Run: `npm --silent --prefix SlideApp test -- src/harness/slide-geometry/review/browserGeometryTextProbe.test.ts src/harness/slide-geometry/review/geometryReviewArtifact.test.ts`

Expected: FAIL because the probe only reports node text metrics today

**Step 3: Write minimal implementation**

- Introduce an entity-level browser probe type
- Read `data-geometry-entity-id` and compatible legacy attributes from the rendered SVG
- Expose `resolveTruthBounds(entity)` helpers for metrics consumers

**Step 4: Run test to verify it passes**

Run: `npm --silent --prefix SlideApp test -- src/harness/slide-geometry/review/browserGeometryTextProbe.test.ts src/harness/slide-geometry/review/geometryReviewArtifact.test.ts`

Expected: PASS

**Step 5: Commit**

```bash
git add SlideApp/src/harness/slide-geometry/review/browserGeometryTextProbe.ts SlideApp/src/harness/slide-geometry/review/browserGeometryTextProbe.test.ts SlideApp/src/harness/slide-geometry/review/geometryReviewArtifact.test.ts
git commit -m "Add browser truth probing for geometry entities"
```

### Task 3: Add tree metrics for sibling collision and parent containment

**Files:**
- Modify: `SlideApp/src/harness/slide-geometry/review/geometryMetrics.ts`
- Test: `SlideApp/src/harness/slide-geometry/review/geometryMetrics.test.ts`

**Step 1: Write the failing test**

Add cases for:

- sibling overlap count from two children with the same `parentId`
- child out-of-bounds count from a child extending beyond the parent edge
- free text collision count and edge label collision count
- epsilon tolerance that ignores sub-pixel noise

**Step 2: Run test to verify it fails**

Run: `npm --silent --prefix SlideApp test -- src/harness/slide-geometry/review/geometryMetrics.test.ts`

Expected: FAIL because tree metrics are not computed yet

**Step 3: Write minimal implementation**

- Add `siblingOverlapCount`, `maxSiblingOverlapArea`, `minSiblingGap`
- Add `childOutOfBoundsCount`, `maxChildOverflowPx`, `minContainmentPad`
- Add `freeTextCollisionCount` and `edgeLabelCollisionCount`
- Map legacy `overlapCount` to the new sibling overlap semantics

**Step 4: Run test to verify it passes**

Run: `npm --silent --prefix SlideApp test -- src/harness/slide-geometry/review/geometryMetrics.test.ts`

Expected: PASS

**Step 5: Commit**

```bash
git add SlideApp/src/harness/slide-geometry/review/geometryMetrics.ts SlideApp/src/harness/slide-geometry/review/geometryMetrics.test.ts
git commit -m "Add tree collision and containment metrics"
```

### Task 4: Wire tree metrics into score policy and review artifact

**Files:**
- Modify: `SlideApp/src/harness/slide-geometry/review/geometryScorePolicy.ts`
- Modify: `SlideApp/src/harness/slide-geometry/review/geometryReviewArtifact.ts`
- Modify: `SlideApp/src/harness/slide-geometry/review/geometryReviewArtifact.test.ts`

**Step 1: Write the failing test**

Add assertions that:

- `blockerOpen` is true when `siblingOverlapCount > 0`
- `blockerOpen` is true when `childOutOfBoundsCount > 0`
- review facts include tree metrics
- verdict text prefers sibling collision and containment failures before critic pass

**Step 2: Run test to verify it fails**

Run: `npm --silent --prefix SlideApp test -- src/harness/slide-geometry/review/geometryReviewArtifact.test.ts`

Expected: FAIL because score policy and artifact do not know about the new tree metrics

**Step 3: Write minimal implementation**

- Extend `GEOMETRY_METRIC_META`
- Fold tree metrics into `scoreLayoutDensity`, `scoreCrossingRisk`, and `scorePrimaryLineClarity`
- Add module-level and node-level review wording for sibling collisions and containment overflow

**Step 4: Run test to verify it passes**

Run: `npm --silent --prefix SlideApp test -- src/harness/slide-geometry/review/geometryReviewArtifact.test.ts`

Expected: PASS

**Step 5: Commit**

```bash
git add SlideApp/src/harness/slide-geometry/review/geometryScorePolicy.ts SlideApp/src/harness/slide-geometry/review/geometryReviewArtifact.ts SlideApp/src/harness/slide-geometry/review/geometryReviewArtifact.test.ts
git commit -m "Wire tree metrics into review scoring and artifact output"
```

### Task 5: Lock the page_31 regression with a real case

**Files:**
- Modify: `SlideApp/src/remotion/pages/Page10Scene.tsx`
- Modify: `SlideApp/src/review/formalPageReviewRegistry.ts`
- Modify: `SlideApp/src/review/formalPageReviewRegistry.test.ts`
- Test: `SlideApp/src/harness/slide-geometry/review/geometryMetrics.test.ts`

**Step 1: Write the failing test**

Add coverage that the `page_31` free text sentence:

- is represented as a measurable entity
- collides with sibling entities under the same parent when its real bounds overlap
- opens a blocker instead of reporting `Overlaps: 0`

**Step 2: Run test to verify it fails**

Run: `npm --silent --prefix SlideApp test -- src/harness/slide-geometry/review/geometryMetrics.test.ts src/review/formalPageReviewRegistry.test.ts`

Expected: FAIL because `page_31` still treats the sentence as a blind spot

**Step 3: Write minimal implementation**

- Add stable render attributes for page31 measurable entities
- Ensure the formal review surface emits matching entity ids and parent ids
- Update the regression fixtures to use the new tree-aware metrics

**Step 4: Run test to verify it passes**

Run: `npm --silent --prefix SlideApp test -- src/harness/slide-geometry/review/geometryMetrics.test.ts src/review/formalPageReviewRegistry.test.ts`

Expected: PASS

**Step 5: Commit**

```bash
git add SlideApp/src/remotion/pages/Page10Scene.tsx SlideApp/src/review/formalPageReviewRegistry.ts SlideApp/src/review/formalPageReviewRegistry.test.ts SlideApp/src/harness/slide-geometry/review/geometryMetrics.test.ts
git commit -m "Lock page31 tree geometry regression"
```

### Task 6: Run full verification and sync docs

**Files:**
- Modify: `Docs/剧本/00-剧本与页面对齐审计.md`
- Modify: `Docs/剧本/31-第三十一页-正式动画.md`

**Step 1: Update docs to match the new harness behavior**

- Record that geometry review now treats the page as a tree
- Record that browser truth is compressed into metrics and policy before rewrite

**Step 2: Run targeted tests**

Run: `npm --silent --prefix SlideApp test -- src/harness/slide-geometry/review/browserGeometryTextProbe.test.ts src/harness/slide-geometry/review/geometryMetrics.test.ts src/harness/slide-geometry/review/geometryReviewArtifact.test.ts src/review/formalPageReviewRegistry.test.ts`

Expected: PASS

**Step 3: Run repo-level verification**

Run:

- `python3 scripts/slide-geometry-harness/audit_transition_timings.py`
- `python3 scripts/slide-geometry-harness/audit_storyboard_sync.py`
- `npm --silent --prefix SlideApp run review:mechanical -- --from page_19`

Expected:

- transition audit passes
- storyboard sync audit passes
- mechanical review reports no blocker regressions from `page_19`

**Step 4: Commit**

```bash
git add Docs/剧本/00-剧本与页面对齐审计.md Docs/剧本/31-第三十一页-正式动画.md
git commit -m "Document tree geometry harness rollout"
```

### Task 7: Optional cleanup after rollout stabilizes

**Files:**
- Modify: `SlideApp/src/harness/slide-geometry/review/geometryMetrics.ts`
- Modify: `SlideApp/src/harness/slide-geometry/review/geometryReviewArtifact.ts`

**Step 1: Evaluate dual-report noise**

- Compare legacy flat metrics and tree metrics on a representative late-tail subset
- Record remaining false positives and false negatives

**Step 2: Remove temporary compatibility branches only if safe**

Run: `npm --silent --prefix SlideApp test -- src/harness/slide-geometry/review/*.test.ts`

Expected: PASS with no behavior regressions

**Step 3: Commit**

```bash
git add SlideApp/src/harness/slide-geometry/review/geometryMetrics.ts SlideApp/src/harness/slide-geometry/review/geometryReviewArtifact.ts
git commit -m "Trim temporary tree geometry compatibility paths"
```
