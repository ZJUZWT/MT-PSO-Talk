# 第 14 页草图镜像：`page14-contract-r1`

## Page goal

This page explains the PSO collection/build/use loop with only the essential actors.
`Computer` owns the cook/build side.
`Phone` owns the runtime side.
The page should read as one clockwise loop with a cook split `A`, a stable merge `B`, and a runtime return path.

## Spatial structure

- `Computer` sits in the lower-left third.
- `Phone` sits in the lower-right third.
- `Computer`, `.scl.csv`, and `Phone` share one horizontal baseline.
- `stablepc.csv`, `B`, and `stable.upipelinecache` sit on the upper stable band.
- `B` is a circled plus merge junction between `stablepc.csv` and `stable.upipelinecache`.
- `.ushaderbytecode` sits below `.scl.csv`.
- `rec.upipelinecache` sits above the main body and closes the top return loop.

## Style rules

- The audited routing skeleton stays in the `0 / 45 / 90` family.
- Rendered turns may be rounded, but they must not drift away from the audited skeleton.
- No shaky or noisy routing.
- Arrowheads stay compact.
- `cook` and `expand` stay as line labels, never standalone nodes.
- `A` stays visually minor but still readable as a split point.
- `B` is a circled plus, not a file box.
- 非 junction 的出点和落点尽量命中对应 side center；同侧多条入线/出线要尽量对称平均，不从角点硬刺出去。

## Visual priority

- First read: the clockwise loop from `Computer` to `Phone` and back.
- Second read: the `cook` split and the `expand` stable band.
- Third read: the individual artifact filenames and the merge semantics at `B`.

## Acceptance checklist

1. `Computer` is in the left lower-third and `Phone` is in the right lower-third.
2. `cook` is an edge label on the `Computer -> A` route, not a node.
3. `expand` is an edge label on the `Computer -> stablepc.csv` route, not a node.
4. `A` is a junction, not a file node.
5. `B` is a circled plus merge junction, not a file node.
6. `Computer`, `.scl.csv`, and `Phone` share one horizontal baseline.
7. `stablepc.csv`, `B`, and `stable.upipelinecache` share one upper stable band.
8. All visible routes keep an auditable `0 / 45 / 90` skeleton, even when the rendered corner is rounded.
9. `stablepc.csv -> B -> stable.upipelinecache` exists as the stable merge path.
10. `.scl.csv -> B` merges upward before `stable.upipelinecache`.
11. `stable.upipelinecache -> Phone` uses horizontal first, then `45 down-right`.
12. The return loop `Phone -> rec.upipelinecache -> Computer` exists.
13. `.ushaderbytecode -> Phone` uses horizontal first, then `45 up-right`.
14. Major non-junction anchors land on side centers or on visibly averaged symmetric offsets.

## Current screenshot

Current browser-exported sketch:

![Page14 contract sketch](../../ignore/page14-contract-r1-capture/page14-contract-r1.png)

Capture source:
`http://127.0.0.1:4177/?mode=sketch&sketch=page14-contract-r1`

Capture provenance:
`browser-api` page export

## Latest fact layer

- `overlapCount`: `0`
- `crossingCount`: `0`
- `nodePierceCount`: `0`
- `badEndpointCount`: `0`
- `textOverflowCount`: `0`
- `primaryLineBendCount`: `2`
- `avoidableBendCount`: `0`
- `Rendered corner style`: `rounded corner smoothing on audited 0/45/90 skeleton`
- `minRenderedFontPx`: `28`
- `minNodeGap`: `12`
- `minMargin`: `40`
- `directionalMargins`: `T 185 / R 40 / B 153 / L 132 px`
- `minSideClearance`: `12 px`
- `crampedNodeCount`: `5`
- `minInternalPadding`: `10.3 px`
- `internalPaddingBySide`: `T 10.3 / R 13.8 / B 10.3 / L 13.8 px`
- `crampedInternalNodeCount`: `0`
- `leftRightMassDelta`: `50%`
- `Node fonts`: `Computer 32px`, `Phone 32px`, `.scl.csv 28px`, `.ushaderbytecode 28px`, `stablepc.csv 28px`, `stable.upipelinecache 28px (2 lines)`, `rec.upipelinecache 28px`, `B 20px symbol`, `A explicit circular split junction`
- `Worst anchor offsets`: `computer-to-a: from Computer right +11px`, `computer-to-stablepc: from Computer right -11px`, `bytecode-to-phone: to Phone left +10px`
- Mechanical verdict: `Clean the primary route before critic pass`

## Browser text probe

This section records the browser-side source of truth for text padding after the latest page14 geometry resize. It is measured from real rendered SVG `getBBox()` output, not from CLI fallback estimation.

- Probe source: `python3 scripts/slide-geometry-harness/probe_geometry_text.py --url 'http://127.0.0.1:4177/?mode=sketch&sketch=page14-contract-r1' --out ignore/page14-contract-r1-capture/page14-contract-r1-browser-text-probe-all.json`
- Probe artifact: `ignore/page14-contract-r1-capture/page14-contract-r1-browser-text-probe-all.json`
- Browser-measured minimum internal padding across audited labeled nodes: `8.0 px`
- Browser-measured directional minima: `T 8.0 / R 8.2 / B 11.6 / L 8.2 px`
- `stable.upipelinecache` browser padding: `T 11.3 / R 8.2 / B 15.4 / L 8.2 px`
- `Phone` browser padding: `T 8.4 / R 11.0 / B 11.6 / L 11.0 px`
- `Computer` browser padding: `T 8.4 / R 13.0 / B 11.6 / L 13.2 px`
- Important audit note: the earlier CLI-only internal padding facts were optimistic because jsdom had no real canvas context. Those numbers remain useful as a rough local preflight, but browser probe data is the truth source for final typography fit.

## 本轮全局修改逻辑

| 全局问题 | 目标 | 通过哪些局部修改来牵引 | 当前结果 |
| --- | --- | --- | --- |
| 字号仍不够 PPT | 把所有语义节点抬到真正可讲的投影字号 | 把 `Computer/Phone` 拉到 `32px`，其余语义节点拉到 `28px`，并同步放宽对应盒子尺寸 | 当前 `minRenderedFontPx = 28`，所有语义节点 `overflowPx = 0` |
| 45 度折线过硬 | 让折点更圆润，但不能失去逻辑骨架 | 渲染层改为 rounded SVG path，合同层与 metrics 仍继续审计 `0/45/90` skeleton | 当前圆角已经生效，但 critic 认为部分转折仍略偏软，需要继续克制半径 |
| 放大字号后几何被挤坏 | 把 `scl / B / stable / phone` 一带重新打开，先把 blocker 清掉 | 重新布置 `Phone`、`.scl.csv`、`B`、`stable.upipelinecache` 的相对位置，修复 turn 反向折返 | `nodePierceCount`、`badEndpointCount` 已回到 `0`，但 merge zone 仍是全页最紧区域 |
| 合同、截图、评分容易分裂 | 让后续迭代都能在同一份脚本里对账 | 最新截图、事实层、双盲 critic 结论和节点/边表都回写到本文件 | 当前此文件已经成为 page14 的单一审计来源 |

## 三层 Review 总表

| Review 层级 | Art Critic | Geometry Critic | 综合判断 | 综合分数 |
| --- | --- | --- | --- | --- |
| `整体布局 Review` | `7.0/10` | `7.0/10` | 左右角色与闭环叙事成立，字体已过线，但端点更像“中下部”而不是明确压到下三分之一；整体仍带一点 web card 的软感。 | `7.0/10` |
| `模块空间结构 Review` | `6.0/10` | `6.5/10` | `cook / stable / runtime return` 三个模块关系成立，但 `B -> stable.upipelinecache -> Phone` 仍偏挤，stable band 的刚性不够强。 | `6.3/10` |
| `单节点与单边 Review` | `6.5/10` | `6.0/10` | 所有节点都已达到 PPT 字号且无超框，圆角转折也已落地；但 `A` 仍偏弱、`B` 仍偏小，局部方向性被圆角磨软了一点。 | `6.3/10` |

## 节点剧本与 Review 表

以下分数为双盲 critic 结论与事实层的合并整理，不是 builder 自评。

| 节点 | 中文剧本描述 | 内部字体大小 | Review 观点 | Review 分数 |
| --- | --- | --- | --- | --- |
| `Computer` | 左下主锚点，代表 cook / build 侧，是整页左侧出发点。 | `32px / 24pt` | 可读性和识别度都够，作为左侧角色已经成立；但在整页重心里还不够“压低”，更像中下部角色。 | `7.2/10` |
| `Phone` | 右下主锚点，代表 runtime / 包体运行侧，是右侧汇聚点。 | `32px / 24pt` | 终点语义成立，左侧双入线仍保持了平均分布；但右侧 merge zone 逼近它，局部呼吸感不足。 | `7.2/10` |
| `A` | `Computer` 输出后的分叉点，只承担 cook split junction 语义。 | `—` | 逻辑上已经分叉成功，但视觉上仍偏像“线自然分开”而不是一个被明确读到的 split junction。 | `6.0/10` |
| `B` | stable 路线的 merge junction，用一个带圆圈的 `+` 表示汇合。 | `20px symbol` | 语义成立，但体量仍偏小，且右侧合流区太近，作为关键 merge 点还不够干净醒目。 | `6.1/10` |
| `.scl.csv` | 中层 material 相关产物，是 lower baseline 上的中间节点。 | `28px / 21pt` | 字号已经达标，和下层主线的关系清楚；但向 `B` 汇入的那一段仍显得有点紧。 | `6.8/10` |
| `.ushaderbytecode` | 下方 cook 产物，偏向 runtime 消费。 | `28px / 21pt` | 路径清楚、字号稳定，是当前最容易一眼读通的 artifact 之一。 | `7.1/10` |
| `stablepc.csv` | 上方 expand 路径产物，是 stable band 的左输入。 | `28px / 21pt` | 节点本身清楚，但 `expand` 标注与它的邻近关系还可以再松一点。 | `6.7/10` |
| `stable.upipelinecache` | stable cache 的最终语义节点，承接 `B` 后再落到 runtime。 | `28px / 21pt` `2 lines` | 两行排版已经能讲，但它和 `B`、`Phone` 一起构成全页最紧区域，也削弱了 stable band 的稳定感。 | `6.4/10` |
| `rec.upipelinecache` | 顶部 return artifact，表示 runtime 收集后的返回记录。 | `28px / 21pt` | 顶部回环读法清楚，是当前闭环叙事里最稳定的一段。 | `7.2/10` |

## 边剧本与 Review 表

以下分数为双盲 critic 结论与事实层的合并整理，不是 builder 自评。

| 边 | 中文剧本描述 | Review 观点 | Review 分数 |
| --- | --- | --- | --- |
| `computer-to-a` | 从 `Computer` 右侧下半中点出发，沿 audited skeleton 先下后右，渲染时用圆角顺过去；线段上写 `cook`。 | 父路径存在且能读，但左侧圆角让分叉方向比以前更软，`A` 的结点感也还不够强。 | `6.8/10` |
| `a-to-bytecode` | 从 `A` 分出下支，沿 audited skeleton 落到 `.ushaderbytecode`。 | 这条线仍然清楚，圆角没有伤到它的读法。 | `7.0/10` |
| `a-to-scl` | 从 `A` 分出上支，沿 audited skeleton 汇到 `.scl.csv`。 | 分叉语义存在，但因为 `A` 本身偏弱，这条边的“从 junction 出发”感没有完全站稳。 | `6.8/10` |
| `bytecode-to-phone` | 从 `.ushaderbytecode` 水平向右，再上拐进入 `Phone`。 | 方向和归宿都清楚，收口也比较整齐。 | `7.0/10` |
| `computer-to-stablepc` | 从 `Computer` 右侧上半中点出发，沿 audited skeleton 上行到 `stablepc.csv`；线段上写 `expand`。 | 主意图成立，但 `expand` 一带仍然偏近，线标注和节点之间还不够松。 | `6.4/10` |
| `stablepc-to-b` | 从 `stablepc.csv` 水平向右进入 merge junction `B`。 | 路径清楚，但抵达 `B` 后马上进入拥挤区，所以终点气口不够宽。 | `6.5/10` |
| `scl-to-b` | 从 `.scl.csv` 向右再上拐，汇入 `B`。 | 逻辑正确，但这是 critic 一致指出的局部高密度区域之一。 | `6.2/10` |
| `b-to-stableupipe` | 从 `B` 水平向右进入 `stable.upipelinecache`。 | merge 语义成立，但 `B` 太小、两侧太近，导致这一小段仍显紧。 | `6.0/10` |
| `stableupipe-to-phone` | 从 `stable.upipelinecache` 向右再下拐进入 `Phone`。 | 去向正确，但右侧局部仍偏挤，且圆角让合流方向稍微偏软。 | `6.3/10` |
| `phone-to-rec` | 从 `Phone` 顶边正中出发，向上再向左连到 `rec.upipelinecache`。 | 顶部回环起笔稳定，是整页最顺的一条边之一。 | `7.2/10` |
| `rec-to-computer` | 从 `rec.upipelinecache` 向左再向下回到 `Computer` 顶边正中，闭合整页回环。 | 闭环叙事很完整，当前是整页最稳的一段收尾。 | `7.3/10` |

## Main blockers

- 字号已经不再是 blocker：当前 `minRenderedFontPx = 28`，所有语义节点 `overflowPx = 0`。
- 几何硬错误已经不再是 blocker：当前 `crossing / pierce / badEndpoint / overlap` 全部为 `0`。
- 当前真正的 blocker 是结构审美层，而不是逻辑正确性：
  - `B -> stable.upipelinecache -> Phone` 的 merge zone 仍偏挤，`minNodeGap = 12px`
  - 节点内部 padding 已经从 `4.6px` 拉到 `10.3px`，这一项不再是 blocker，但右侧合流区的外部气口仍然不够松
  - 全局边距仍不对称，当前是 `L 132 / R 40`
  - 圆角已经成立，`A` 也已经变成显式圆 junction，但 stable band 与右侧合流的方向感仍可继续加硬

## Blind critic verdicts

- Art Critic:
  - `整体布局 Review`: `7.0/10`
  - `模块空间结构 Review`: `6.0/10`
  - `单节点与单边 Review`: `6.5/10`
  - 短评：字体已经过线，但整页仍偏 web-card 语言，不够像投影页；右侧 stable merge zone 仍显拥挤。
  - Top fixes：回拉 PPT 感、打开右侧 stable 区、统一线条与节点的圆角语言。
- Geometry Critic:
  - `整体布局 Review`: `7.0/10`
  - `模块空间结构 Review`: `6.5/10`
  - `单节点与单边 Review`: `6.0/10`
  - 短评：合同大体成立，但 `A` 还不够像明确 junction，stable band 的“铁直”感和下方基线的“尺子感”还需要加强。
  - Top fixes：把 `A` 做实、把 stable band 与 baseline 拉直、疏解最紧区域并恢复方向性。
