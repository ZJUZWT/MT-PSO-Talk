# Contract Template

Recommended file path:

- final page script: `Docs/剧本/NN-第N页.md`
- sketch mirror ledger: `Docs/剧本/NN-第N页-草图镜像-<sketch-id>.md`

## Page goal

- One sentence describing what this page must explain.
- One sentence describing which actors are primary and which are secondary.

## Node inventory

| id | text | semantic role | visual kind | anchor region | emphasis | alignment constraints |
| --- | --- | --- | --- | --- | --- | --- |
| `computer` | `Computer` | cook machine | device box | lower-left third | primary | shares baseline with `phone` and `.scl.csv` |

## Edge inventory

| id | from | to | meaning | route grammar | max bends | segment label | arrow style | sibling / symmetry note |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `e_cook_to_a` | `computer` | `junction_a` | cook flow | launch 45 down-right, then horizontal right | `1` | `cook` on horizontal segment | single arrowhead | parent branch of shader outputs |

Allowed route grammar language:

- `horizontal`
- `vertical`
- `45 up-right`
- `45 down-right`
- `horizontal then 45 up-right`
- `45 down-right then horizontal`

## Spatial constraints

- `computer` sits in the lower-left third.
- `phone` sits in the lower-right third.
- `.scl.csv`, `computer`, and `phone` share one horizontal baseline.
- `stablepc.csv` and `stable.upipelinecache` share one horizontal baseline.
- Junction `A` sits to the right of `computer` and below the shared baseline.

## Element styles

- Nodes: clean rectangular outlines, no jitter.
- Junctions: small solid points.
- Edges: `0/45/90` only.
- Arrowheads: compact, proportional to stroke width.
- Line labels: centered on the owning segment and never promoted into standalone nodes.
- Hard bans: no crossings, no freeform curves, no clipped text.

## Assembly order

1. Place all major nodes with baselines and bands locked first.
2. Place junction points.
3. Route primary branches from shared sources.
4. Add secondary return loop.
5. Add segment labels.
6. Run the acceptance checklist item by item.

## Acceptance checklist

1. `Computer` is in the left lower-third and `Phone` is in the right lower-third.
2. The `cook` branch exits from `Computer` lower-right, launches at `45°`, then runs horizontal to junction `A`.
3. All branches from `A` keep the specified symmetry and route grammar.
4. Shared baselines are preserved exactly where required.
5. No edge violates the declared angle family or label ownership.

## Current screenshot

![Current page screenshot](/absolute/path/to/current-screenshot.png)

Capture source:
`http://127.0.0.1:4173/?mode=sketch&sketch=example-sketch-id`

## 三层 Review 总表

| Review 层级 | Art Critic | Geometry Critic | 综合判断 | 综合分数 |
| --- | --- | --- | --- | --- |
| `整体布局 Review` | `7.0/10` | `7.2/10` | 主图位置、占比、留白和重心的全局判断。 | `7.1/10` |
| `模块空间结构 Review` | `7.4/10` | `7.8/10` | source / middle system / upper band / receive zone / return loop 的分带与呼吸感判断。 | `7.6/10` |
| `单节点与单边 Review` | `6.8/10` | `7.5/10` | 单点、单边和局部着陆质量的综合判断。 | `7.2/10` |

This table is page-local. Do not inherit numeric scores from a different page.

## 节点剧本与 Review 表

| 节点 | 中文剧本描述 | Review 观点 | Review 分数 |
| --- | --- | --- | --- |
| `Computer` | 左下主锚点，代表 cook / build 侧。 | 位置正确，但如果周边线条过低，会让它显得过于下沉。 | `7.5/10` |

## 边剧本与 Review 表

| 边 | 中文剧本描述 | Review 观点 | Review 分数 |
| --- | --- | --- | --- |
| `computer-to-a` | 从 `Computer` 右下出发，先 `45°` 向右下，再水平向右到分叉点；水平段上写 `cook`。 | 路由语法正确，但如果分叉点过低，会拖垮下半区的气质。 | `6.5/10` |
