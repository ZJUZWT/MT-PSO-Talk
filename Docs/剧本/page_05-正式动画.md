# 第 06 页正式动画：`5`

> 文件类型：`正式动画页`
> Step / Sketch：`5` / `-`
> 上一页：`4_data`
> 下一页：`6`

## Harness 公式快照

- 公式版本：`Harness Registry v1.0`
- 统一总表：`Docs/SkillFramework/higher-level/slide-geometry-harness/references/harness-formula-registry.md`
- 几何硬门槛：`overlap=0`、`crossing=0`、`nodePierce=0`、`textOverflow=0`
- 动画标准：`node_move / edge_grow / fade`

## 页面评估看板（四项）

| 指标 | 分数 | 说明 |
| --- | --- | --- |
| 布局分数 | `8.0/10` | UE Cook 主线稳定，右上角新增缩略结构后信息闭环更完整。 |
| Node 分数 | `7.9/10` | 右上角改成与前面同构的 `UE PSO = Shaders+States+？` 卡后，标题改为水平居中，中间恢复成两个小框 `UsageMask` / `BindCount`，其中 `UsageMask` 更醒目。 |
| Edge 分数 | `7.8/10` | 主链保持不变，只新增一条说明性虚线，不打断原阅读路径。 |
| 动画分数 | `8.5/10` | `5 -> 6` 为 `in_range`。 |

## 动画剧本

| Beat | 时间 | 画面变化 | 屏幕元素 | 备注 |
| --- | --- | --- | --- | --- |
| 1 | `0.00s - 1.50s` | 从数据页回放进入 UE Cook 主线终态，并在右上角补出一张与前面同构的 `UE PSO = Shaders+States+？` 卡，标题水平居中，中间恢复成 `UsageMask` / `BindCount` 两个小框。 | `Material/Cooked/Binary/GPU/UE PSO card` | 主舞台骨架不变，只新增一张说明性结构卡与一条虚线；顶部用 ShaderHash，中间保持结构字段且突出 `UsageMask`，下面尽量用 Enum / Struct 名字。 |

## Current screenshot

![Current screenshot](/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/ignore/browser-api-captures/page05-ue-pso-card-headless-v4.png)

- 当前 URL：`http://127.0.0.1:4173/?step=page_05`
- 捕获方式：`Edge headless screenshot`

## 三层 Review 总表

| Review 层级 | Art Critic | Geometry Critic | 综合判断 | 综合分数 |
| --- | --- | --- | --- | --- |
| `整体布局 Review` | `7.9/10` | `8.1/10` | 主链清楚，右上补充结构不压主轴。 | `8.0/10` |
| `模块空间结构 Review` | `7.8/10` | `8.0/10` | 主链与注释卡分层成立。 | `7.9/10` |
| `单节点与单边 Review` | `7.7/10` | `7.9/10` | 新增节点与虚线关系可读。 | `7.8/10` |

## 节点剧本与 Review 表

| 节点 | 中文剧本描述 | 与上一页连续语义 | 当前渲染载体 | 连续语义 Review | 连续语义分数 | 几何 / 美感 Review | 综合分数 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `material-node` | Cook 输入资产节点。 | 承接 `4_data` 回放后显性落位 | 主节点 | 连续成立。 | `7.6/10` | 可读性稳定。 | `7.6/10` |
| `cooked-node` | Cook 输出节点。 | 连续承接 | 主节点 | 连续成立。 | `7.5/10` | 可读性稳定。 | `7.5/10` |
| `ue-pso-structure-card` | 右上角与前面同构的 `UE PSO = Shaders+States+？` 结构卡，标题水平居中，顶部是 `VertexShader Hash / PixelShader Hash`，中间恢复成两个与下方两列对齐的小框 `UsageMask` / `BindCount`，下面再放几类 Enum / Struct 名字。 | 新增说明节点 | 结构卡 | 与上一页无连续要求。 | `10/10` | 与主链保持足够间距，未压住 `CookedShaderCode`。 | `8.1/10` |

## 边剧本与 Review 表

| 边 | 中文剧本描述 | 与上一页连续语义 | 当前渲染载体 | 连续语义 Review | 连续语义分数 | 几何 / 美感 Review | 综合分数 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `material-to-cooked` | 资产到 Cook 输出。 | 承接 `4_data` 回放 | 主链边 | 连续成立。 | `7.4/10` | 路径清晰。 | `7.4/10` |
| `cooked-to-binary` | Cook 到 Binary。 | 承接主链 | 主链边 | 连续成立。 | `7.3/10` | 线条规整。 | `7.3/10` |
| `cooked-to-ue-pso-card` | 从 `CookedShaderCode` 指向右上 `UE PSO` 结构卡的说明性虚线。 | 新增说明边 | 虚线注释边 | 与上一页无连续要求。 | `10/10` | 水平虚线清楚，不切入主链。 | `8.0/10` |

## 过渡动画剧本与时长审查

- `4_data -> 5`：`2.400s`，`too_long`
- `5 -> 6`：`1.500s`，`in_range`
- timing probe workload：`Docs/剧本/workloads/page_04_data_to_page_05.json`
- 本次仅在既有 `page_05` 终态内保留右上角同构的 `UE PSO = Shaders+States+？` 说明卡，把标题改为水平居中，并将中间层恢复成两个小框 `UsageMask` / `BindCount`，其中 `UsageMask` 使用更强的强调色；`UE PSO 以 ShaderHash 为索引定位对应的 ShaderCode` 继续留在左侧 `objectiveFacts`，没有调整 step frame 锚点；两段转场 verdict 保持不变。
