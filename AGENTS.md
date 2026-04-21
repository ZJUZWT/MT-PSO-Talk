# Local Agent Rules (High Priority)

更新时间：2026-04-17

本文件用于约束本仓库中的 AI 执行行为。以下规则为高优先级，执行时必须遵守。

## 1) 先数学约束，再图像复核

- 每次执行视觉/页面相关修改时，先按现有 skill 的数学方法计算约束值（如边距、对齐、覆盖/交叉、超框等）。
- 先基于计算结果做判断与调整。
- 若计算结果达标但用户仍不满意，再使用读图/截图能力做二次复核与调整。

## 2) 图片/表格解释页的过渡逻辑

- 当新增“图片页”或“解释页（含表格页）”时，统一采用：
  1. `淡入图片/表格，同时淡出之前页面`
  2. 若该图片/表格链条结束：`淡出图片/表格，淡入之前页面`
  3. 然后再播放原有页面间的转场动画
- 不允许在图片页与图片页之间夹杂无关主图闪回。

## 3) 背景一致性与色块限制

- 不新增大色块背景，不引入破坏全局背景一致性的底板。
- 遇到第 2 条场景时，必须通过淡入淡出完成视觉切换，而不是切背景色块。

## 4) 修改说明要求

- 每次改动后必须向用户明确说明：
  - 新增了哪些页面/元素
  - 移动了哪些节点
  - 增加/删除了哪些 node / edge
  - 哪些行为被替换为新的过渡逻辑

## 5) 新增动画的时长预算规则

- 当在一段既有动画中插入“前置动画”或“中间补充动画”时，不能压缩原始动画时长。
- 原则：`新总时长 = 原始动画时长 + 新增动画时长`。
- 示例：若原动画为 `2s`，新增一个前置动画，则应变为 `前置动画(合理时长) + 原始动画2s`，而不是把两者硬塞进 `2s`。
- 只有在用户明确要求“整体时长不变”时，才允许做压缩重分配。

## 6) 几何评分逻辑（强制沿用现有 Skill）

- 评分逻辑仍在，且必须复用 `slide-geometry-harness` 这套链路，不允许人工臆造分数。
- 函数链路（源码）：
  - `SlideApp/src/harness/slide-geometry/review/geometryReviewArtifact.ts` -> `buildGeometryReviewArtifact(...)`
  - `SlideApp/src/harness/slide-geometry/review/geometryMetrics.ts` -> `collectGeometryMetrics(...)`
  - `SlideApp/src/harness/slide-geometry/review/geometryScorePolicy.ts` -> `scoreGeometryMetrics(...)`
- 三层级输出必须同时给出：
  - `整体布局 Review`
  - `模块空间结构 Review`
  - `单节点与单边 Review`

## 7) 过渡动画解释与时长审查（每页必做）

- harness 公式总表唯一入口：`Docs/SkillFramework/higher-level/slide-geometry-harness/references/harness-formula-registry.md`
- 动画时长标准版本：`Harness v1.0`（详见 `Docs/SkillFramework/higher-level/slide-geometry-harness/references/animation-timing-standard.md`）
- 动作公式（默认）：
  - `node_move`: `duration_ms = clamp(180 + distancePx*0.95, 180, 960)`
  - `edge_grow`: `duration_ms = clamp(140 + lengthPx*0.75, 140, 840)`
  - `fade_in`: `220ms`（夹紧到 `180~420`）
  - `fade_out`: `220ms`（夹紧到 `180~420`）
- 标准 phase 顺序（默认）：
  - `hold_before -> fade_out -> node_motion -> edge_motion -> fade_in -> hold_after`
- 标准 verdict：
  - `requiredSec = 标准时间轴总时长`
  - `allowedMinSec = requiredSec`
  - `allowedMaxSec = requiredSec * 1.35`
  - 当前时长低于下限记为 `too_short`，高于上限记为 `too_long`
- 对每个页面的“上一页 -> 当前页”过渡，都要生成动画解释：
  - 总时长（秒/帧）
  - 关键时间点（至少 Start/Q1/Mid/Q3/End）
  - 每个时间点的 `Node 行为` 与 `Edge 行为`
  - 动画标准相位时间轴（phase start/end/duration）
  - 动画标准动作时间表（每个 action 的 kind/metric/duration）
- 使用脚本：
  - `python3 scripts/slide-geometry-harness/probe_transition_timeline.py --from-step <prev_step> --to-step <current_step> --workload-json <workload-json> --emit-markdown`
  - 可选：`--base-seconds <原始动画时长> --insert-seconds <新增动画时长>` 做“原动画不被挤占”检查
- 时长策略：
  - 若 verdict=`too_short`，需要加长
  - 若 verdict=`too_long`，需要缩短
  - 通过调整 `SlideApp/src/remotion/embed.ts` 的 step frame 锚点完成，并同步更新相关 tests
- 页面改完后，必须做一次动画 review，并把结果写回该页剧本（`Docs/剧本/`）。

## 8) 流程层 Harness Gate（建议作为 hook 入口）

- 对视觉 / 页面 / 动画相关任务，优先通过仓库内 gate 先做流程分流，而不是只靠模型自己判断：
  - 分类：
    `python3 scripts/slide-geometry-harness/workflow_gate.py classify --prompt "<用户任务>" --files <可能改动的文件...> --write-state`
  - 收尾：
    `python3 scripts/slide-geometry-harness/workflow_gate.py stop --state-path .git/slide-geometry-harness-workflow-state.json`
- gate 会把任务分成三档：
  - `skip`：不是 slide/harness 相关任务
  - `lite`：文案、notes、storyboard 数据等轻量页面任务；至少跑 `audit_storyboard_sync.py`
  - `full`：几何、动画、时长、剧本 workload 等任务；至少跑 `audit_transition_timings.py` + `audit_storyboard_sync.py`
- Codex 侧当前仓库入口放在 `.codex/hooks.json`：
  - `UserPromptSubmit` -> `workflow_gate.py codex-user-prompt-submit`
  - `Stop` -> `workflow_gate.py codex-stop`
- 不要在不同宿主里复制一份判定逻辑；宿主层只做 hook stdin/stdout 适配，真正分类与审计计划统一留在 `workflow_gate.py`。
