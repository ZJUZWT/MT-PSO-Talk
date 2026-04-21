# 统一存放ShaderCode减少重复消耗

> 文件类型：`正式动画页`
> Step / Sketch：`9_img` / `-`
> 上一页：`9`
> 下一页：`10`

## Harness 公式快照

- 公式版本：`Harness Registry v1.0`
- 动画标准：`overlay_page`
- 当前转场时长：`4.200s`

## 页面评估看板（四项）

| 指标 | 分数 | 说明 |
| --- | --- | --- |
| 布局分数 | `7.3/10` | 插图主体、顶部外置说明与右上 prompt overlay 分区清楚。 |
| Node 分数 | `7.1/10` | 图片容器与两层说明节点数量少、识别直接。 |
| Edge 分数 | `7.0/10` | 本页主要是 overlay 渐变关系。 |
| 动画分数 | `5.0/10` | `9 -> 9_img` 与 `9_img -> 10` 当前仍有时长异常。 |

## 页面目标

- 在 `page_09` 的 SharedCode 方案图之后，先给一张补充插图页。
- 这页不改主论证，只补一组真实样例说明：母材质、两个相同实例，以及为了保留 ShaderCode 而改的 Static Bool。

## 页面改动表

| 项目 | 内容 |
| --- | --- |
| 新增页面/元素 | 补图本体 + 顶部外置样例说明；右上 prompt overlay 保持 |
| 移动节点 | 无 |
| 新增 / 删除 node / edge | 无主画布拓扑改动；只新增顶部说明文字 |
| 行为替换 | 维持图片页 overlay 淡入 / 淡出逻辑，不夹杂主图闪回；中间淡说明移出比较卡 |

## 2026-04-21 顶部外置样例说明补丁

- 新增元素：把原来中间的淡说明外提到三栏上方，改成顶部外置 legend。
- 新增文字：
  `M = 母材质`
  `M-I1 / M-I2 = 两个相同的 Material Instance`
  `注：M-I1 / M-I2 都额外改了同一个 Static Bool，否则 UE 可能会优化掉，不保存对应 ShaderCode。`
- 移动节点：无；图片主体与五块证据卡保持原位。
- 新增 / 删除 node / edge：无主画布拓扑改动，只替换说明文字位置。
- 行为替换：把 `.uexp 对比（M 系列）` 卡内的淡说明移到卡外顶部，不修改 `page_09 -> page_09_img` 与 `page_09_img -> page_10` 的原有插图页逻辑。

## 2026-04-21 可读性微调补丁

- 新增页面/元素：无。
- 移动节点：无主拓扑迁移；顶部 legend 改成“两行主说明 + 一行小字注释”，并把上排三卡与下排两块 hash 卡一起收回到更平衡的竖向留白。
- 新增 / 删除 node / edge：无。
- 行为替换：无，仍沿用插图页 overlay 淡入 / 淡出逻辑。
- 布局调整：
  `top-legend-note` 主说明拆成 `M` / `M-I*` 两行，`注` 改成一行小字；
  三行字重整体下调，不再用过黑的标题感；
  上排三卡与下排 hash 卡一并回调位置，让顶部说明和卡组之间的上下留白更接近；
  两块 `Hash 复用证据` 卡片继续保持“缩盒不缩字”，并保留卡内上下留白对齐约束；
  `OpenGL / Vulkan` 两块 hash 证据卡新增独立测试锚点，方便后续持续锁定“缩盒不缩字”的约束。

## 动画剧本

| Beat | 时间 | 画面变化 | 屏幕元素 | 备注 |
| --- | --- | --- | --- | --- |
| 1 | `page_09 -> page_09_img` | 方案图淡出，插图页淡入。 | 插图 / 顶部说明 / prompt overlay | 不引入新背景色块。 |
| 2 | `page_09_img -> page_10` | 插图页淡出，回到原主图世界。 | 插图 / 顶部说明 / prompt overlay | 不允许中途闪回无关主图。 |

## Current screenshot

- 最新截图：`/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/ignore/browser-api-captures/20260421-110917/page09img-two-line-note.png`
- 当前 URL：`http://127.0.0.1:4175/?step=page_09_img`
- 捕获方式：`browser-api page`
- 捕获说明：本轮从本地 Vite 预览服务抓取，未包含浏览器 chrome。

![Current screenshot](/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/ignore/browser-api-captures/20260421-110917/page09img-two-line-note.png)

## 三层 Review 总表

| Review 层级 | Art Critic | Geometry Critic | 综合判断 | 综合分数 |
| --- | --- | --- | --- | --- |
| `整体布局 Review` | `7.2/10` | `7.4/10` | 插图主体、顶部外置 legend 与右上 prompt overlay 可以并存，不压主图。 | `7.3/10` |
| `模块空间结构 Review` | `7.0/10` | `7.2/10` | 图片页主要靠 overlay 叙事，结构简单。 | `7.1/10` |
| `单节点与单边 Review` | `6.9/10` | `7.1/10` | 只需保持图片容器与顶部说明显隐稳定。 | `7.0/10` |
 
## 节点剧本与 Review 表

| 节点 | 中文剧本描述 | 与上一页连续语义 | 当前渲染载体 | 连续语义 Review | 连续语义分数 | 几何 / 美感 Review | 综合分数 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `image-node` | 插图主体。 | 承接 `page_09` 的补充解释节奏 | 图片容器 | 成立。 | `7.0/10` | 主要承担证据展示。 | `7.0/10` |
| `top-legend-note` | 顶部外置样例说明，交代母材质 / 实例 / Static Bool 的真实关系。 | 承接前页关于 SharedCode 证据来源的追问 | 顶层 overlay 文字 | 成立。 | `7.2/10` | 不压主图，把说明从中间移出后阅读更直接。 | `7.2/10` |
| `overlay-answer-card` | 右上 prompt overlay，继续保留“统一存放ShaderCode减少重复消耗”的回答锚点。 | 承接前页关于 SharedCode 的问题 | 顶层 overlay 卡片 | 成立。 | `7.1/10` | 与顶部说明分工清楚，没有压住主图主证据。 | `7.1/10` |

## 边剧本与 Review 表

| 边 | 中文剧本描述 | 与上一页连续语义 | 当前渲染载体 | 连续语义 Review | 连续语义分数 | 几何 / 美感 Review | 综合分数 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `overlay-fade-in` | 从 `page_09` 淡入图片、顶部说明与 prompt overlay。 | 承接 overlay 规则 | 渐变关系 | 成立。 | `7.0/10` | 平稳。 | `7.0/10` |
| `overlay-fade-out` | 向 `page_10` 淡出图片、顶部说明与 prompt overlay。 | 承接 overlay 规则 | 渐变关系 | 成立。 | `7.0/10` | 平稳。 | `7.0/10` |

## 过渡动画剧本与时长审查

- `9 -> 9_img`：`too_short`
- `9_img -> 10`：`too_long`
- 说明：这两个时长问题为当前仓库已有问题，本次只补齐正式动画文档与顶部外置样例说明，不在这一轮调整 step 锚点。
