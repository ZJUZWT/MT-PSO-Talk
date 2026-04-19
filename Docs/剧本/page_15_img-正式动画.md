# 插图过渡页：`15_img`

> 文件类型：`正式动画页`
> Step / Sketch：`15_img` / `-`
> 上一页：`13_img`
> 下一页：`15`

## Harness 公式快照

- 公式版本：`Harness Registry v1.0`
- 动画标准：`overlay_page`
- 当前转场时长：`1.000s`

## 页面评估看板（四项）

| 指标 | 分数 | 说明 |
| --- | --- | --- |
| 布局分数 | `7.3/10` | 插图页结构稳定。 |
| Node 分数 | `7.0/10` | 图片为主。 |
| Edge 分数 | `7.0/10` | 渐变关系为主。 |
| 动画分数 | `待复核` | `2026-04-19` fresh timing audit：`13_img -> 15_img` 为 `in_range`，`15_img -> 15` 为 `too_long`，本次仅修整图显示，未改 step frame。 |

## 动画剧本

| Beat | 时间 | 画面变化 | 屏幕元素 | 备注 |
| --- | --- | --- | --- | --- |
| 1 | `0.00s - 1.00s` | 插图过渡到主图。 | 图片 / 主图背景 | 保持统一背景，不闪回主图。 |

## Current screenshot

![Current screenshot](/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/ignore/slide-stage-captures/20260419-153030/page15img-whole-image-fixed.png)

- 当前 URL：`http://127.0.0.1:4173/?variant=bus-clean&step=page_15_img`
- 捕获方式：`headless-stage`
- 捕获来源记录：`/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/ignore/slide-stage-captures/20260419-153030/page15img-whole-image-fixed.txt`
- 几何客观事实：
  - 原图尺寸：`901 x 305`，宽高比约 `2.954`
  - 旧图片框：`1188 x 648`，宽高比约 `1.833`，配合 `xMidYMid slice` 会必然裁掉左右内容
  - 新图片框：`1188 x 430`，宽高比约 `2.763`，改为 `xMidYMid meet` 后整图完整显示
  - 以满宽显示时，图片实际可见高度约 `402px`，上下各留约 `14px` 内边距

## 三层 Review 总表

| Review 层级 | Art Critic | Geometry Critic | 综合判断 | 综合分数 |
| --- | --- | --- | --- | --- |
| `整体布局 Review` | `7.2/10` | `7.4/10` | 稳定。 | `7.3/10` |
| `模块空间结构 Review` | `7.0/10` | `7.2/10` | 清楚。 | `7.1/10` |
| `单节点与单边 Review` | `6.9/10` | `7.1/10` | 可读。 | `7.0/10` |

## 节点剧本与 Review 表

| 节点 | 中文剧本描述 | 与上一页连续语义 | 当前渲染载体 | 连续语义 Review | 连续语义分数 | 几何 / 美感 Review | 综合分数 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `image-node` | 插图主体。 | 承接 `13_img` | 图片容器 | 成立。 | `7.0/10` | 可读。 | `7.0/10` |

## 边剧本与 Review 表

| 边 | 中文剧本描述 | 与上一页连续语义 | 当前渲染载体 | 连续语义 Review | 连续语义分数 | 几何 / 美感 Review | 综合分数 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `overlay-fade` | 插图与主图的渐变关系。 | 承接 overlay 规则 | 渐变边语义 | 成立。 | `7.0/10` | 平稳。 | `7.0/10` |

## 过渡动画剧本与时长审查

- `13_img -> 15_img`：`1.000s`，`in_range`
- `15_img -> 15`：`1.750s`，`too_long`
- Fresh audit source：`/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/ignore/transition-audits/page15img-20260419/transition-timing-audit-20260419-153119.md`
