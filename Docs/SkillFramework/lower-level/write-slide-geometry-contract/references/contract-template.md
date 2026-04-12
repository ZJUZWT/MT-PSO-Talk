# Contract Template

Recommended file path:

- formal page ledger: `Docs/剧本/NN-第N页-正式动画.md`
- sketch mirror ledger: `Docs/剧本/NN-第N页-草图镜像-<sketch-id>.md`

Both formal pages and sketch mirrors must use the same ledger shape.

## 页面元信息

| 项目 | 内容 |
| --- | --- |
| 文件类型 | `正式动画页` / `草图镜像页` |
| 页面 / Step id | `page_xx` |
| 草图 id | `pagexx-r1` / `无` |
| 上一页 | `page_(x-1)` |
| 当前 URL | `http://127.0.0.1:4173/?step=page_xx` 或 sketch URL |
| 当前截图路径 | `/absolute/path/to/current-screenshot.png` |

## Page goal

- One sentence describing what this page must explain.
- One sentence describing which actors are primary and which are secondary.

## Node inventory

| id | text | semantic role | visual kind | anchor region | emphasis | alignment constraints | 与上一页连续语义 | 同元素承载要求 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `computer` | `Computer` | cook machine | device box | lower-left third | primary | shares baseline with `phone` and `.scl.csv` | `无` | `新元素，可淡入` |
| `bytecode` | `.ushaderbytecode` | first cooked artifact | artifact box | lower middle band | primary | centered between devices | `承接上一页 ShaderLibrary` | `必须复用同一 shared element carrier` |

## Edge inventory

| id | from | to | meaning | route grammar | max bends | segment label | arrow style | sibling / symmetry note | 与上一页连续语义 | 同元素承载要求 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `e_cook_to_a` | `computer` | `junction_a` | cook flow | launch 45 down-right, then horizontal right | `1` | `cook` on horizontal segment | single arrowhead | parent branch of shader outputs | `无` | `新元素，可淡入` |
| `e_runtime_left` | `vertex_old` | `gpu_old` | old main-axis left link remapped into phone runtime | rounded handoff path | `2` | `无` | single arrowhead | sibling of `e_runtime_right` | `承接上一页 VertexData -> GPU` | `必须复用同一 shared edge carrier或明确记录断裂原因` |

Allowed route grammar language:

- `horizontal`
- `vertical`
- `45 up-right`
- `45 down-right`
- `horizontal then 45 up-right`
- `45 down-right then horizontal`
- `rounded handoff path`

## Spatial constraints

- `computer` sits in the lower-left third.
- `phone` sits in the lower-right third.
- `.scl.csv`, `computer`, and `phone` share one horizontal baseline.
- `stablepc.csv` and `stable.upipelinecache` share one horizontal baseline.
- Junction `A` sits to the right of `computer` and below the shared baseline.

## Element styles

- Nodes: clean rectangular outlines, no jitter.
- Junctions: small solid points.
- Edges: declared angle family only; if curves are allowed, state which edges may curve.
- Arrowheads: compact, proportional to stroke width.
- Line labels: centered on the owning segment and never promoted into standalone nodes.
- Hard bans: no crossings, no clipped text, no fake continuity by duplicating a semantically continuous actor.

## Assembly order

1. Place all major nodes with baselines and bands locked first.
2. Mark which nodes and edges inherit continuous semantics from the previous page.
3. Reserve the shared render carriers for continuous nodes and edges before adding new elements.
4. Place junction points.
5. Route primary branches from shared sources.
6. Add secondary return loop.
7. Add segment labels.
8. Run the acceptance checklist item by item.

## Acceptance checklist

1. `Computer` is in the left lower-third and `Phone` is in the right lower-third.
2. The `cook` branch exits from `Computer` lower-right, launches at `45°`, then runs horizontal to junction `A`.
3. All branches from `A` keep the specified symmetry and route grammar.
4. Shared baselines are preserved exactly where required.
5. No edge violates the declared angle family or label ownership.
6. Every node or edge with previous-page continuity explicitly records whether it must reuse the same render carrier.
7. If a node or edge is semantically continuous, it is not re-created as an unrelated duplicate without a documented reason.

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

| 节点 | 中文剧本描述 | 与上一页连续语义 | 当前渲染载体 | 连续语义 Review | 连续语义分数 | 几何 / 美感 Review | 综合分数 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `Computer` | 左下主锚点，代表 cook / build 侧。 | `无` | `Page10Scene/ComputerDevice` | `无连续义务，可作为新舞台淡入。` | `N/A` | 位置正确，但如果周边线条过低，会让它显得过于下沉。 | `7.5/10` |
| `.ushaderbytecode` | 中央主产物，承接上一页回答出来的 `ShaderLibrary`。 | `承接上一页 ShaderLibrary` | `shared-element: shader-library-bytecode-carrier` | `语义连续，应由同一 carrier 完成形变，而不是新起一层淡入。` | `9.5/10` | 位置稳定，但需要检查字框和母版的水平居中。 | `8.0/10` |

## 边剧本与 Review 表

| 边 | 中文剧本描述 | 与上一页连续语义 | 当前渲染载体 | 连续语义 Review | 连续语义分数 | 几何 / 美感 Review | 综合分数 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `computer-to-a` | 从 `Computer` 右下出发，先 `45°` 向右下，再水平向右到分叉点；水平段上写 `cook`。 | `无` | `Page10Scene/e_cook_to_a` | `无连续义务，可新画。` | `N/A` | 路由语法正确，但如果分叉点过低，会拖垮下半区的气质。 | `6.5/10` |
| `runtime-left-handoff` | 把上一页 `VertexData -> GPU` 的左半主线接入手机内部 runtime。 | `承接上一页主轴左链路` | `shared-edge: runtime-left-carrier` | `如果语义连续，就不能只是新画一条像它的线。` | `9.0/10` | 过渡弧线必须干净，不可出现多余拐点或飘移。 | `7.8/10` |
