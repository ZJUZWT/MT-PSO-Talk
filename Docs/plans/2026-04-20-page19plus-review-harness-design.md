# Page 19+ Review Harness Design

**Date:** 2026-04-20  
**Scope:** `SlideApp` 审校型 harness 命令，仅覆盖 `page_19` 及之后页面

## Goal

补一条“只审校、不拦截、不改页面”的报告链路，让 `page_19+` 的正式页面和相邻过渡都能稳定落出当前 harness 对应的审校 artifact，避免再次出现“口头说过 harness，但没有数学产物”的情况。

## Non-Goals

- 不修改 `page_19` 之前的页面
- 不自动修改任何页面布局、节点、边或动画时长
- 不接入 `pre-push`
- 不自动回写 `Docs/剧本/`
- 不重新定义评分规则；必须复用现有 harness

## Existing Harness Contracts To Reuse

几何审校必须直接复用：

- `SlideApp/src/harness/slide-geometry/review/geometryReviewArtifact.ts`
- `SlideApp/src/harness/slide-geometry/review/geometryMetrics.ts`
- `SlideApp/src/harness/slide-geometry/review/geometryScorePolicy.ts`

几何三层 Review 继续沿用现有换算逻辑：

- `整体布局 Review`
- `模块空间结构 Review`
- `单节点与单边 Review`

时长审校必须直接复用：

- `scripts/slide-geometry-harness/probe_transition_timeline.py`
- `Docs/SkillFramework/higher-level/slide-geometry-harness/references/harness-formula-registry.md`
- `Docs/SkillFramework/higher-level/slide-geometry-harness/references/animation-timing-standard.md`

## User-Approved Behavior

第一版是 `report-only` 命令，默认行为等价于：

```bash
npm --prefix SlideApp run review:slides -- --from page_19
```

命令要求：

1. 默认只审校 `page_19+`
2. 终端打印 geometry + transition timing 摘要
3. 结果落盘为可追溯 artifact
4. 始终 `exit 0`
5. 若页面缺 formal sketch，明确记为 `missing_sketch`
6. 若过渡缺 workload，明确记为 `missing_workload`

## Output Layout

自动生成产物全部写入 `ignore/slide-review/page_19_plus/`。

建议结构：

- `summary.json`
- `pages/<stepId>.geometry.json`
- `pages/<stepId>.geometry.md`
- `transitions/<from>__to__<to>.timing.json`
- `transitions/<from>__to__<to>.timing.md`

这样既不污染正式剧本，也保留后续“挑结果回写 docs”的余地。

## Geometry Output Contract

每个 geometry artifact 至少包含：

- `stepId`
- `sketchId`
- `label`
- `整体布局 Review`
- `模块空间结构 Review`
- `单节点与单边 Review`
- `mechanicalScore`
- `verdict`
- `scores`
- `metrics`
- `topFixes`
- `worstNodes`
- `worstEdges`

其中数值全部来自当前 `buildGeometryReviewArtifact(...)` 链路，不允许人工臆造。

## Timing Output Contract

每个 timing artifact 至少包含：

- `fromStep`
- `toStep`
- 当前时长（秒 / 帧）
- `requiredSec`
- `allowedMinSec`
- `allowedMaxSec`
- `verdict`
- 关键时间点：`Start / Q1 / Mid / Q3 / End`
- phase 时间轴
- action 时长表

其中 timing verdict 必须由 `probe_transition_timeline.py` 产出。

## Scope Boundary

`page_19+` 的含义是从 `SlideApp/src/remotion/sceneTimeline.ts` 的 canonical step sequence 中，以 `page_19` 为起点向后取所有正式步骤。

第一版不会为了“让命令能跑”去 retroactively 补 `page_19` 之前的 sketch 或 workload。

## Implementation Shape

实现只增加一条总控命令，负责串两类现有审校：

1. 几何审校：读取 formal sketch -> 构建 geometry artifact -> 输出 json/md
2. 时长审校：读取相邻 transition workload -> 调用 timing probe -> 输出 json/md

为减少重复实现，命令优先抽取现有 `SlideApp/scripts/printGeometryReview.ts` 中已经存在的三层 Review、worst nodes、worst edges 汇总逻辑。

## Verification Expectations

命令本身需要覆盖以下验证点：

- 默认范围从 `page_19` 开始
- 缺 sketch 时不会崩溃，而是落 `missing_sketch`
- 缺 workload 时不会崩溃，而是落 `missing_workload`
- 产物目录会被创建并更新
- 已有 workload 的 transition 会真正生成 timing artifact

## Future Upgrade Path

等规则稳定后，这条 report-only 命令可以自然升级成两条更强的链路：

1. `warn-on-push`
2. `blocking CI gate`

但这两个都不在第一版范围内。
