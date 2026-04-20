# Page 31 Loop Design

## Goal

把 `page_31` 从“评分链路 + 公式说明”升级成“项目内真实回环页”，直接讲清楚：

`Agent 修改 -> 前台 Edge / 浏览器真实取数 -> geometryReviewArtifact -> geometryMetrics -> geometryScorePolicy -> 回写下一轮修改`

同时保留数学约束与 hook / gate 的存在感，但不再让它们抢掉主循环主角。

## Chosen Approach

采用用户确认的 `A` 方案：

- 主视觉改成完整闭环，突出“改完会自动拿真实数据，再自动计算”的反馈系统。
- 数学公式与硬门槛缩成辅助信息区，说明这不是主观调图。
- 文案明确点名 `hook`、`workflow_gate.py`、前台 Microsoft Edge 真实取数、`geometryReviewArtifact -> geometryMetrics -> geometryScorePolicy` 这一条内部计算链。

## Page Structure

### 左侧主循环

- 顶部节点：`Agent / Codex 修改页面`
- 右侧节点：`前台 Edge 真实取数`
- 底部右侧节点：`geometryReviewArtifact`
- 底部中间节点：`geometryMetrics`
- 底部左侧节点：`geometryScorePolicy`
- 回写节点：`回写修改建议 / 下一轮修改`

这些节点用显式箭头连成一个顺时针闭环，避免继续只靠隐式阅读流。

### 右侧辅助说明

- 一张较窄的辅助卡解释两件事：
  - 触发入口：`hook / workflow_gate.py / review:mechanical`
  - 数学门槛：`overlap=0`、`crossing=0`、`nodePierce=0`、`textOverflow=0`
- 底部补一句收束：我们先拿真实渲染数据，再做约束计算，而不是只靠公式猜。

## Why This Matches The Repo

- 已有仓库真实流程里同时存在：
  - hook / workflow gate
  - 前台 Microsoft Edge 文本几何探针
  - formal sketch / geometry artifact / metrics / score policy
  - automatic review loop
- 所以这页不是概念图，而是对当前仓库工作流的可视化整理。

## Risks To Avoid

- 不要把右侧辅助卡做得比主闭环更重。
- 不要把“真实取数”画成泛泛截图，要明确是前台 Edge / 浏览器取数。
- 不要重新引入大面积色块背景。
- 不要让新增箭头造成 formal review 的 crossing / pierce 回归。

## Validation

- 更新 `page_31` 的 formal review 基准与相关测试。
- 运行 page31 相关 targeted tests。
- 跑 `audit_transition_timings.py`、`audit_storyboard_sync.py`、`review:mechanical -- --from page_19`。
- 最终回答里解释这套回环具体靠什么实现：hook、workflow gate、前台 Edge 真实取数、geometry review 链。
