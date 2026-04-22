# 第 29_data 页

> 文件类型：`正式动画页`
> Step / Sketch：`page_29_data` / `formal-page29-data`
> 上一页：`page_29`
> 下一页：`page_30`

## Harness 公式快照

- 公式版本：`Harness Registry v1.0`
- 统一总表：`Docs/SkillFramework/higher-level/slide-geometry-harness/references/harness-formula-registry.md`
- 几何硬门槛：`overlap=0`、`crossing=0`、`nodePierce=0`、`textOverflow=0`
- 动画标准：`node_move / edge_grow / fade` + phase `hold_before -> fade_out -> node_motion -> edge_motion -> fade_in -> hold_after`
- 时长判定：`allowedMinSec = requiredSec`，`allowedMaxSec = requiredSec * 1.35`

## 页面评估看板（四项）

| 指标 | 分数 | 说明 |
| --- | --- | --- |
| 布局分数 | `整体布局 Review=4.0/10` | 顶排已经收成“shader 两块 + state 两块 + 双平台表”，但 formal mechanical review 仍提示局部口袋偏紧。 |
| 模块分数 | `模块空间结构 Review=4.3/10` | 上下阅读顺序已经稳定，不过上下卡片之间的舒适间距还没达到 harness 的宽松阈值。 |
| 节点 / 边分数 | `单节点与单边 Review=9.3/10` | 无 overlap / crossing / nodePierce / textOverflow blocker，隐式阅读流继续成立。 |
| 动画分数 | `in_range` | `page_29 -> page_29_data` 与 `page_29_data -> page_30` 继续保持 `71` 帧。 |

## Page goal

- 在 `page_29` 的治理证据页之后，不再讲 cpp 和参数表，而是直接把测试用的同一份 `Vertex Shader / Fragment Shader` 和 `state` 开关摊开。
- 这一页要明确表达：参与测试的 shader 没变，真正变的是 `state` 何时对驱动可见；PSO 的价值就在这里。

## 页面改动表

| 项目 | 内容 |
| --- | --- |
| 页面标题 | `无（移除页内主标题）` |
| 页面副标题 | `无（移除页内副标题）` |
| 当前页核心信息 | 顶排直接由 `Vertex Shader / Fragment Shader` 占住左上和右上，而且标题与代码都比上一版更大并整体上提；shader 代码行现在保留真实缩进，并加了轻量语法高亮；原本右上角那组开关下移成第二排左右各一块，也去掉 `State 开关` 可见组标题、外层框与 API 对照标签；两块 state 现在直接把结论写进顶行，不再保留灰色解释行；下方保留轻表格化的 PC / Android 极值表，列名改成 `VK off / VK on`，并让表格边界直接吃满外层 node 宽度，表头上沿只留一根直线，底部仍是一条脚注。 |
| 与上一页关系 | `page_29` 已解释了“为什么同一个 Material 会分叉出不同 PSO”；这一页继续回答“为什么驱动会在乎这些前置信息”。 |
| 与下一页关系 | 当前页先把“同一份 shader，只换 state”讲清，再把工程阅读交给 `page_30`。 |
| 这一页新增了什么 | `shader-card`、横向分栏后的 `vertex-code / fragment-code`、`state-card`、`state-vk`、`state-gl`、`footer-note`。 |
| 这一页移动了什么 | 顶排维持 `vertex-code` 左上、`fragment-code` 右上，但标题与代码整体上提并放大；`state-vk / state-gl` 维持第二排左右各一块，同时把原第三行灰色说明折叠掉，只保留顶行结论 + 第二行代码；`pc-card / android-card` 里的统计表从居中内框改成直接贴满外层 node 宽度。 |
| 删除了哪些 node / edge | 删除上一版的 `benchmark-card`、`benchmark-code`、`benchmark-env` 语义；此前的环境参数解释也不再单独成块。 |
| 增加 / 替换的 edge | 上半区阅读流改成 `Vertex/Fragment Shader -> State 开关 -> 双平台极值表 -> Footer`。 |
| 被替换的过渡逻辑 | 原先“代码/参数 + state + 表格”淡入，改成“vert/frag + 更大的 state 卡 + 表格”淡入；前后页仍保持淡入淡出链路。 |

## Node inventory

| id | text | semantic role | visual kind | anchor region | emphasis | alignment constraints | 与上一页连续语义 | 同元素承载要求 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `shader-card` | `隐藏分组：测试 Shader` | shader 证据总容器 | open group | 顶排全宽 | 高 | `vertex-code / fragment-code` 直接占住左右两列，不再额外挂一层可见标题。 | 把上一页的治理证据转成“参与测试的 shader 长什么样” | 否 |
| `vertex-code` | `Vertex Shader` | 测试使用的 vert | code box | `shader-card` 内左侧 | 高 | 标题与代码整体上提并放大，代码行保留真实缩进，但仍需给 heavy loop 足够宽度。 | 新增证据 | 否 |
| `fragment-code` | `Fragment Shader` | 测试使用的 frag | code box | `shader-card` 内右侧 | 高 | 与 `vertex-code` 顶底对齐，不再做上下堆叠；代码行保留真实缩进并加轻量语法高亮。 | 新增证据 | 否 |
| `state-card` | `隐藏分组：State 开关 / 驱动可见性` | state 差异总容器 | open group | 顶排下方全宽 | 高 | `state-vk / state-gl` 直接占住左右两列，不再额外挂一层可见标题。 | 回答“为什么同一份 shader 会跑出不同结果” | 否 |
| `state-vk` | `Vulkan PSO：构建时Shader对此已知` + `blendAtt.colorWriteMask = 0;` | Vulkan state | code strip | 第二排左侧 | 高 | 与 `state-gl` 同高同宽；结论直接并入顶行，不再保留第三行灰色解释。 | 新增证据 | 否 |
| `state-gl` | `OpenGL / GLES runtime：构建时Shader对此无感知` + `glColorMask(GL_FALSE, ...)` | OpenGL / GLES state | code strip | 第二排右侧 | 高 | 与 `state-vk` 同高同宽；结论直接并入顶行，不再保留第三行灰色解释。 | 新增证据 | 否 |
| `pc-card` | `PC（RTX 3080）` | 桌面端极值表容器 | rounded card | 主舞台左中 | 高 | 与 `android-card` 同宽同高。 | 承接上一页的平台差异证据 | 否 |
| `pc-row-1` | `loop=10` | PC 小 loop 极值 | table row | `pc-card` 内 | 中 | 与 `pc-row-2` 共列。 | 新增证据 | 否 |
| `pc-row-2` | `loop=5000` | PC 大 loop 极值 | table row | `pc-card` 内 | 高 | `VK off` 列需要继续被强调。 | 新增证据 | 否 |
| `android-card` | `Android（Adreno）` | 移动端极值表容器 | rounded card | 主舞台右中 | 高 | 与 `pc-card` 同宽同高。 | 承接上一页的平台差异证据 | 否 |
| `android-row-1` | `loop=10` | Android 小 loop 极值 | table row | `android-card` 内 | 中 | 与 `android-row-2` 共列。 | 新增证据 | 否 |
| `android-row-2` | `loop=5000` | Android 大 loop 极值 | table row | `android-card` 内 | 高 | 继续不展示 `GLES RGBA`。 | 新增证据 | 否 |
| `footer-note` | `NV 的 Vulkan mask=0 几乎不随 loop 波动...` | 单句脚注总结 | footer bar | 主舞台底部 | 高 | 宽度覆盖双平台卡下方，但不能形成新底板。 | 为 `page_30` 收束 | 否 |

## 边剧本与 Review 表

| id | from | to | meaning | route grammar | max bends | segment label | arrow style | sibling / symmetry note | 与上一页连续语义 | 同元素承载要求 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `shader-to-state` | `shader-card` | `state-card` | 先给 shader，再看 state 差异 | implicit horizontal | 0 | - | none | 顶部形成一眼就能看懂的双入口 | 是 | 否 |
| `top-to-tables` | `shader-card / state-card` | `pc-card / android-card` | 从测试前提落到结果极值 | implicit vertical | 0 | - | none | 由上向下压到双平台表格 | 是 | 否 |
| `tables-to-footer` | `pc-card / android-card` | `footer-note` | 从极值表收束到一句结论 | implicit vertical | 0 | - | none | 双平台一起汇聚到底部脚注 | 是 | 否 |
| `footer-to-page30` | `footer-note` | `page_30` | 数据解释页收束到工程阅读页 | implicit page handoff | 0 | - | none | 先得出一句结论，再给资料入口 | 是 | 否 |

## Spatial constraints

- 顶部不允许回到“参数 / 环境”说明卡；左上和右上都必须直接落到 `vert / frag` 本身。
- 顶排两块 shader 与第二排两块 state 必须和下方 `pc-card / android-card` 共用同一列宽，保证整页形成稳定双栏。
- `shader-card / state-card` 外层框去掉后，内容本身仍要保持同样的阅读起点与列对齐。
- `state-card` 继续维持左右对称宽度，但内部现在只允许“顶行结论 + 第二行代码”两层信息，不再回到第三行灰色解释句。
- 下方极值表继续保持左右对称，只保留 `loop=10 / loop=5000` 两组极值；表格边界直接贴满外层 node 宽度，表头上沿必须是一根直线，不再出现圆角上沿。
- 底部脚注继续保持轻量，不回到旧的大总结卡。

## Element styles

- 顶部 shader / state 区不再画外层盒，也不再显示组标题；四个内部块继续沿用晚段浅底、轻边框、橙色强调的统一语气。
- `vertex-code / fragment-code / state-vk / state-gl` 的代码行继续使用等宽字体，明确这是测试代码而不是抽象示意；shader 区保留真实缩进，并用轻量语法高亮区分关键字、类型、数字和注释；顶行结论沿用正文强调字重。
- 不再保留 `API 对照` 小标签位。
- 重点提示并入标题区，不再额外长出独立的 focus strip。

## Assembly order

1. `page_29` 整体淡出。
2. `shader-card`、`state-card`、`pc-card`、`android-card` 同步进入，其中 `Vertex Shader / Fragment Shader` 以放大后的标题与代码先稳定读点。
3. `footer-note` 在后段补齐，把整页收束成一句话。

## Acceptance checklist

1. 观众能立刻读出：参与测试的是同一份 `vert / frag`。
2. 观众能立刻读出：真正被切换的是 `blendAtt.colorWriteMask = 0` 与 `glColorMask(...)` 这种 state。
3. 观众能看见：在 `PC（RTX 3080）` 上，`VK off` 从 `loop=10` 到 `loop=5000` 几乎不波动。
4. 观众能看见：在 `Android（Adreno）` 上，大 loop 时 `mask=0` 没有兑现同级别收益。
5. 页面结尾仍然只有一条脚注式总结句。

## 动画剧本

| Beat | 时间 | 画面变化 | 屏幕元素 | 讲述词 / 备注 |
| --- | --- | --- | --- | --- |
| 1 | 前段 | `page_29` 淡出 | 治理证据合页 | 从输入布局根因转入“参与测试的 shader 到底是什么”。 |
| 2 | 中段 | 顶部 shader/state 卡与双平台极值表淡入 | `shader-card`、`state-card`、`pc-card`、`android-card` | 先看 shader，再看 state，再看结果。 |
| 3 | 后段 | 脚注条补齐 | `footer-note` | 用一句话收住“PSO 给驱动的到底是什么”。 |

## Remotion 实现备注

| 项目 | 内容 |
| --- | --- |
| 新增的节点 / 元素 | `Page29DataShaderCard`、更大的 `Page29DataStateCard`、横向分栏的 `vertex-code / fragment-code` |
| 移动的节点 | `vertex-code / fragment-code` 维持左右上角两块，但标题与代码整体上移，且代码文本改成保留缩进的轻量高亮渲染；`state-vk / state-gl` 维持第二排左右各一块，并把灰色说明折叠进顶行结论；`pc-card / android-card` 内的统计表改成横向吃满外层 node 宽度、顶部直线分隔的轻表格 |
| 删除的节点 | `benchmark-card`、`benchmark-code`、`benchmark-env` |
| 删除 / 替换的 edge | 删除旧的“参数 / 环境”阅读入口，改成 `vert / frag -> state -> 极值表 -> 脚注` |
| 需要复用的元素 | `PlaceholderBoardShell`、`StageBox`、晚段统一浅底风格、淡入淡出链路 |
| 是否需要镜头推进 / 缩放 | 否 |
| 是否需要逐步显隐 | 是，脚注条后补，其余模块同步进入 |
| 风险点 | 顶部 shader 区和第二排 state 顶行都已经靠近当前宽度预算；若继续明显增大字号，长行会优先撞宽度预算。 |

## Current screenshot

- Latest screenshot：`ignore/browser-api-captures/20260422-122038/review-capture.png`
- Capture provenance：`browser-api / scope=page / http://127.0.0.1:4173/?step=page_29_data`
- Capture size：`4096 x 2404`
- 复核结论：最新 capture 已更新到“shader 代码保留缩进并加轻量高亮”的当前版本；第二排两块 state 已改成“顶行结论 + 第二行代码”的两层结构，灰色解释行已移除；transition probe 仍是 `71 frames / in_range`。

## 三层 Review 总表

| Review 层级 | Art Critic | Geometry Critic | 综合判断 | 综合分数 |
| --- | --- | --- | --- | --- |
| `整体布局 Review` | `formal harness` | `4.0/10` | 顶排与底排语义已经收顺，但局部留白仍偏紧。 | `通过` |
| `模块空间结构 Review` | `formal harness` | `4.3/10` | `shader -> state -> table -> footer` 的层级成立，不过 harness 仍提示若干 tight pocket。 | `通过` |
| `单节点与单边 Review` | `formal harness` | `9.3/10` | 无 overlap / crossing / nodePierce / textOverflow blocker。 | `通过` |

## 节点剧本与 Review 表

| 节点 | 中文剧本描述 | 与上一页连续语义 | 当前渲染载体 | 连续语义 Review | 连续语义分数 | 几何 / 美感 Review | 综合分数 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `shader-card` | 把参与测试的 shader 直接摊开。 | 从治理证据切到测试前提 | open group | 成立。 | `通过` | 顶排直接进入两个 shader，阅读起点更直接。 | `通过` |
| `state-card` | 把 `Vulkan / OpenGL` 的 state 可见性差异摊开。 | 回答“为什么同一份 shader 会跑出不同结果” | open group | 成立。 | `通过` | 第二排左右各一块后，state 信息不再挤在右上角。 | `通过` |
| `pc-card` | 桌面端极值表，重点压 `VK off` 稳定性。 | 从前提落到桌面端结果 | rounded card | 成立。 | `通过` | 去掉 API badge 后，焦点更集中在数值本身。 | `通过` |
| `android-card` | 移动端极值表，对照说明优化没有兑现。 | 从前提落到移动端结果 | rounded card | 成立。 | `通过` | 与左卡同宽同高，且轻表格不再有碎块感。 | `通过` |
| `footer-note` | 单句脚注收束整页结论。 | 为下一页阅读页收口 | footer bar | 成立。 | `通过` | 继续保持轻量。 | `通过` |

## 过渡动画剧本与时长审查

## 过渡动画剧本（page_29 -> page_29_data）

- 帧窗口：`2983 -> 3054`（`71` frames @ 60fps）
- 总时长：`1.183s`
- 时长评估：`in_range`（目标区间 `0.90s ~ 1.80s`）
- 调整建议：目标帧长 `71`，相对当前 `Δ+0` frames
- 标准时序评估：`in_range`（标准下限 `1.071s`，上限 `1.446s`）
- 标准调整建议：目标帧长 `71`，相对当前 `Δ+0` frames

### Harness 动画标准（v1.0）

- 标准相位顺序：`hold_before -> fade_out -> node_motion -> edge_motion -> fade_in -> hold_after`
- 标准推荐最短时长：`1.071s`
- 标准允许上限：`1.446s`

| Phase | 开始(ms) | 结束(ms) | 时长(ms) | 动作 |
|---|---:|---:|---:|---|
| hold_before | 0.0 | 100.0 | 100.0 | - |
| fade_out | 100.0 | 320.0 | 220.0 | fade_out_governance_source |
| node_motion | 320.0 | 671.0 | 351.0 | move_platform_tables, move_footer_note |
| fade_in | 671.0 | 891.0 | 220.0 | fade_in_driver_surface |
| hold_after | 891.0 | 1071.0 | 180.0 | - |

| 动作 id | 类型 | 相位 | 指标 | 计算时长(ms) | 来源 |
|---|---|---|---|---:|---|
| fade_out_governance_source | fade_out | fade_out | - | 220.0 | formula |
| move_platform_tables | node_move | node_motion | distancePx=180.0 | 351.0 | formula |
| move_footer_note | node_move | node_motion | distancePx=120.0 | 294.0 | formula |
| fade_in_driver_surface | fade_in | fade_in | - | 220.0 | formula |

| 时间点 | 帧 | 相对时间(s) | Node 行为 | Edge 行为 |
|---|---:|---:|---|---|
| Start | 2983 | 0.000 | hold_before 静置 | hold_before 静置 |
| Q1 | 3001 | 0.300 | fade_out_governance_source(fade_out) | 无显著边变化 |
| Mid | 3018 | 0.583 | move_platform_tables(node_move)、move_footer_note(node_move) | 无显著边变化 |
| Q3 | 3036 | 0.883 | fade_in_driver_surface(fade_in) | 无显著边变化 |
| End | 3054 | 1.183 | 无显著节点变化 | 无显著边变化 |

## Animation review verdict

- `page_29 -> page_29_data` 与 `page_29_data -> page_30` 继续保持 `71` 帧，当前不需要改 anchor。
- 当前页仍遵循“淡出上一页 -> 淡入解释页 -> 淡出解释页 -> 淡入下一页”的链路，没有插入无关主图闪回。

## 审批状态

`通过。可继续作为 page_29 与 page_30 之间的 shader/state 解释页进入正式讲述。`
