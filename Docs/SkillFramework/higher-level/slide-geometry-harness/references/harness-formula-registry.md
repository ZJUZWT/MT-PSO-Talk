# Harness Formula Registry (Single Source)

版本：`v1.0`  
适用范围：`slide-geometry-harness` 全链路（几何评测 + 文本排版 + 动画时长）

本文件是 harness 公式总表。后续阈值或公式调整，优先更新本文件，并同步实现代码。

---

## 1) 几何文本排版公式（Typography / Overflow）

来源：

- `SlideApp/src/harness/slide-geometry/render/geometryText.ts`

核心常量：

- `PPT_MIN_READABLE_FONT_PT = 18`
- `PX_PER_PT = 96 / 72`
- `PPT_MIN_READABLE_FONT_PX = round(18 * 96 / 72) = 24`
- `PPT_COMFORT_BODY_FONT_PX = 26`
- `PPT_COMFORT_RECEIVER_FONT_PX = 30`
- `PPT_COMFORT_RECEIVER_TITLE_FONT_PX = 32`

可用区域：

- 叶子节点可用宽度：`availableWidth = max(0, node.width - 20)`
- 容器节点可用宽度：`availableWidth = max(0, node.width - 44)`
- 叶子节点可用高度：`availableHeight = max(0, node.height - 8)`
- 容器节点可用高度：`availableHeight = 60`

字体缩放：

- `desiredFont = baseFont * min(widthScale, heightScale, 1)`
- `fontSize = forcedFontSize ?? max(minFont(=24), desiredFont)`
- 行高因子：容器 `1.06`，叶子 `1.12`

溢出：

- `overflowPx = max(estimatedWidthPx - availableWidthPx, blockHeightPx - availableHeightPx, 0)`

内部 padding（用于节点内边距评分）：

- 容器：
  - `left = 28`
  - `right = node.width - 28 - blockWidth`
  - `top = 38 - blockHeight/2`
  - `bottom = node.height - (38 + blockHeight/2)`
- 非容器：
  - `left = right = (node.width - blockWidth)/2`
  - `top = bottom = (node.height - blockHeight)/2`
- `tightest = min(top,right,bottom,left)`（并裁剪到 `>=0`）

---

## 2) 几何指标提取公式（Metrics）

来源：

- `SlideApp/src/harness/slide-geometry/review/geometryMetrics.ts`

基础判定：

- `overlapCount`：非容器亲缘节点对，`rectGap == 0` 计重叠
- `rectGap`：`overlap ? 0 : hypot(dx, dy)`
- `crossingCount = sum(edgeRoute.crossingCount) / 2`
- `edgeOverlapCount = sum(edgeRoute.overlapCount) / 2`

路径质量：

- `primaryLineBendCount = sum(bendCount(edge))`（仅 primary tone）
- `avoidableBendCount = count(edge.waypoints && (from.x==to.x || from.y==to.y))`
- `shortSegmentCount`：段长 `< 36px` 的段数量（按 edge route 聚合）
- `hookTurnCount`：`A-B-C` 三段满足 `A||C 同方向 且 B 异方向 且 len(B)<48`
- `detourRatio = max(0, (routeLength - idealLength)/idealLength)`
  - orthogonal route 的 `idealLength = |dx| + |dy|`
  - 含非正交段 route 的 `idealLength = hypot(dx,dy)`

锚点质量：

- `offCenterAnchorCount`：端点锚点 `centerRatio > 0.35`（非 junction-center / unattached）
- `cornerAnchorCount`：锚点落在 corner 时计数

舞台与空间：

- `minMargin = min(node 到四边最小外边距)`
- `top/right/bottom/leftMargin`：由节点+边所有点包围盒计算
- `minSideClearance`：节点四向最近障碍距离最小值
- `crampedNodeCount`：`tightestSideClearance < 48` 的节点数

文本相关：

- `textOverflowCount = overflow labels 数`
- `maxTextOverflowPx = max(overflowPx)`
- `minRenderedFontPx = min(参与审计节点 fontSize)`
- `minInternalPadding = min(nodePadding.tightest)`
- `crampedInternalNodeCount`：`tightestPadding < 10` 的节点数

重心平衡：

- `leftRightMassDelta = |leftMass - rightMass| / (leftMass + rightMass)`
- `mass = node.width * node.height`（优先 top-level nodes）

---

## 3) 几何评分公式（Scoring）

来源：

- `SlideApp/src/harness/slide-geometry/review/geometryScorePolicy.ts`
- `SlideApp/src/harness/slide-geometry/review/geometryReviewArtifact.ts`

公共：

- `clampScore(v) = clamp(round(v), 0, 10)`

### 3.1 Layout Density

初始分 `8`，若 `overlapCount>0 || textOverflowCount>0` 直接 `2`。  
其余扣分：

- `minNodeGap <24 => -4`，`<36 => -1`
- `minMargin <24 => -2`，`<32 => -1`
- `minSideClearance <24 => -2`，`<40 => -1`
- `crampedNodeCount >=3 => -1`
- `minInternalPadding <6 => -2`，`<10 => -1`
- `crampedInternalNodeCount >=2 => -1`

### 3.2 Stage Layout

初始分 `10`。  
中间量：

- `occupiedWidthRatio = (1280 - leftMargin - rightMargin)/1280`
- `occupiedHeightRatio = (720 - topMargin - bottomMargin)/720`
- `horizontalAsymmetry = |left-right|/1280`
- `verticalAsymmetry = |top-bottom|/720`

扣分：

- `occupiedWidthRatio <0.65 || >0.9 => -1`
- `occupiedHeightRatio <0.5 || >0.64 => -2`，`<0.56 => -1`
- `horizontalAsymmetry >0.18 => -2`，`>0.1 => -1`
- `|left-right| >120 => -2`，`>72 => -1`
- `verticalAsymmetry >0.22 => -4`，`>0.14 => -2`，`>0.08 => -1`
- `|top-bottom| >120 => -2`，`>40 => -1`
- `minMargin <40 => -1`，`<56 => -0.5`

### 3.3 Balance

- `leftRightMassDelta <=0.08 => 9`
- `<=0.18 => 8`
- `<=0.30 => 6`
- `>0.30 => 4`

### 3.4 Line Straightness

初始：

- `10 - 2*primaryLineBendCount - 3*avoidableBendCount - 2*badEndpointCount - 2*hookTurnCount - 2*edgeOverlapCount`

附加扣分：

- `shortSegmentCount >=3 => -2`，`>0 => -1`
- `detourEdgeCount >=2 => -2`，`>0 => -1`
- `maxDetourRatio >0.35 => -2`，`>0.18 => -1`
- `cornerAnchorCount >=2 => -2`，`>0 => -1`
- `offCenterAnchorCount >=4 => -2`，`>=2 => -1`

### 3.5 Crossing Risk

- `overlapCount >0 => 2`
- else if `crossingCount>0 || nodePierceCount>0 => 4`
- else if `edgeOverlapCount>0 => 6`
- else `9`

### 3.6 Primary Line Clarity

- `score = min(lineStraightness, crossingRisk)`
- `textOverflowCount>0 => score=min(score,4)`
- `primaryLineBendCount>1 => -1`
- `hookTurnCount>0 => -1`
- `maxDetourRatio>0.18 => -1`
- `offCenterAnchorCount>=3 => -1`

### 3.7 Blocker / Mechanical / Verdict

- `blockerOpen = overlapCount>0 || crossingCount>0 || nodePierceCount>0 || textOverflowCount>0`
- `mechanicalScore = mean(stageLayout, layoutDensity, balance, lineStraightness, crossingRisk, primaryLineClarity)`（保留 1 位小数）

verdict 优先级：

1. `blockerOpen`：按 overflow / overlap / pierce / crossing 返回 blocker 文案
2. `lineStraightness<6 || primaryLineClarity<6`
3. `stageLayout<7`
4. `layoutDensity<7`
5. `balance<7`
6. 否则 `Ready for blind critic pass`

---

## 4) 动画时长公式（Timing）

来源：

- `scripts/slide-geometry-harness/probe_transition_timeline.py`

### 4.1 传统区间 profile

- `strict`: `1.2s ~ 2.2s`
- `wide`: `1.0s ~ 2.8s`
- `auto`:
  - 若 step 含 `_img` 或 `_data`：`0.9s ~ 1.8s`
  - 否则：`1.2s ~ 2.6s`

### 4.2 Timing Standard（Harness v1.0）

动作公式：

- `node_move: clamp(180 + distancePx*0.95, 180, 960)`
- `edge_grow: clamp(140 + lengthPx*0.75, 140, 840)`
- `fade_in: 220ms`（夹紧 `180~420`）
- `fade_out: 220ms`（夹紧 `180~420`）

默认相位：

- `hold_before -> fade_out -> node_motion -> edge_motion -> fade_in -> hold_after`
- `hold_before = 120ms`
- `hold_after = 180ms`
- 每个 phase 时长 = 该 phase 动作时长最大值（hold phase 取 `max(hold, actionMax)`）

标准 verdict：

- `requiredSec = phase 串行总时长`
- `allowedMinSec = requiredSec`
- `allowedMaxSec = requiredSec * 1.35`
- `< allowedMinSec => too_short`
- `> allowedMaxSec => too_long`
- 否则 `in_range`

建议帧长：

- `suggestedFrameSpan = round(targetSec * fps)`
- `targetSec`：
  - `too_short => requiredSec`
  - `too_long => requiredSec * 1.2`
  - `in_range => currentDuration`

基础动画保护：

- 若传入 `base_seconds` 与 `insert_seconds`：
  - `currentDuration >= base + insert => pass`
  - 否则 `fail`

---

## 5) 过程硬门槛（Contract / Skill Gate）

来源：

- `Docs/SkillFramework/higher-level/slide-geometry-harness/SKILL.md`
- `Docs/SkillFramework/higher-level/slide-geometry-harness/references/loop-contract.md`

默认硬禁：

- `node-node overlap`（除非声明 container membership）
- `edge-edge crossing`（除非声明 junction）
- `edge penetration`（穿透非目标节点/标签）
- `long co-lane edge overlay`（除非声明 split/merge handoff）

门槛失败时：

- 页面标记为 `not approved`
- `单节点与单边 Review` 上限 `4.0`

---

## 6) 标准命令（统一入口）

```bash
python3 scripts/slide-geometry-harness/probe_transition_timeline.py \
  --from-step <prev_step> \
  --to-step <current_step> \
  --workload-json <workload-json> \
  --emit-markdown
```

---

## 7) 关联文档

- 动画细化说明：`references/animation-timing-standard.md`
- 过渡动作模板：`references/workload-template.json`
- 控制循环：`references/loop-contract.md`
