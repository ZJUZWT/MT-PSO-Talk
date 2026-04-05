# SlideApp Correspondence-First Transition Design

## Summary

这一轮不是改剧情，而是修正 `page_04 -> page_05` 的过渡方法。

当前页面语义已经对了：

- `page_04` 是 Vulkan / `PSO`
- `page_05` 是 UE asset cook bridge

但过渡方式仍然偏“上一页淡出、下一页淡入”，没有把跨页仍然成立的结构当成同一个对象来承接。这会直接损伤页间稳定性，也会让观众感觉像在看两张图切换，而不是同一套骨架被扩展和改写。

## Problem

用户明确指出了几个根问题：

| 问题 | 现象 | 本质 |
| --- | --- | --- |
| 纯切换感过强 | `page_05` 像是整块淡入，不像从 `page_04` 演化 | 缺少 node / edge correspondence |
| 稳定骨架被打断 | `GPU`、主轴、箭头宽度、进线侧别容易漂 | 没有把继承骨架当作一等约束 |
| 资产层表达不纯 | `Mesh` / `Material` 虽然出现了，但更像“新贴上去的盒子” | 缺少从 Vulkan 产物视角到 UE 资产视角的 handoff |
| 资产样式过脏 | 绿色框里还有装饰条纹 | 资产节点风格没有收敛到“技术分享”的克制表达 |

## Design Goal

`page_04 -> page_05` 需要实现下面这件事：

> 不改变故事主轴的前提下，把“Vulkan 的 shader / PSO 包装视角”平滑交棒成“UE 的 Mesh / Material 资产视角”。

观众应该感受到的是：

1. `GPU` 和 `Pixels` 这条运行时骨架一直都在。
2. `VertexData` 仍然是左侧运行时输入，不应该退化成纯文字替代。
3. `SPIR-V ShaderCode` 和 `Cooked ShaderCode` 不是无关两物，而是“技术产物视角”向“引擎资产产物视角”的重命名与改写。
4. `Mesh` 与 `Material` 不是凭空新开一套布局，而是挂接到既有运行时骨架上的资产来源。

## Core Rule

### Node / Edge Correspondence First

每次跨页过渡都先回答：

| 类型 | 问题 |
| --- | --- |
| Node | 上一页哪些节点在下一页仍然是“同一个东西”，只是换了角色名、强调层级或附加来源？ |
| Edge | 上一页哪些边在下一页仍然成立，只是颜色、粗细、终点或强调层发生变化？ |
| Skeleton | 哪些主轴位置必须完全稳定，否则观众会失去空间记忆？ |

只要语义连续，就优先做下面这些动作：

- 平移
- 尺寸变化
- 标签重写
- 颜色权重切换
- 上下游来源的补充显现

而不是先做整组淡出淡入。

## Page 04 -> Page 05 Mapping

### Stable skeleton

| Subject | Type | Rule |
| --- | --- | --- |
| `GPU` 主盒 | Node | 保持在稳定中轴，不重新找版心 |
| `Pixels` 输出盒 | Node | 保持右侧主轴关系 |
| `VertexData -> GPU` 主线 | Edge | 继续作为运行时主骨架存在 |
| 箭头语言 | Edge | 继续使用横平竖直、固定箭头宽度、固定光学缝隙 |

### Re-authored layer

| From page 04 | To page 05 | 方式 |
| --- | --- | --- |
| `SPIR-V ShaderCode` | `Cooked ShaderCode` | 保持上方中轴关系，改标签与强调，不做整块换页 |
| `Description` / `PSO` | off-stage | 退场，不与 UE 资产层抢空间 |
| Vulkan packaging 视角 | UE asset origin 视角 | 通过 `Material -> Cooked ShaderCode` 交棒 |

### New asset attachments

| New node | Contract |
| --- | --- |
| `Mesh` | 在左侧主轴外侧，水平接入 `VertexData` |
| `Material` | 在上方资产带中，水平接入 `Cooked ShaderCode` |

## Motion Contract

### What must stay stable

- `GPU` 盒子的主位置
- 主轴左右两条运行时箭头的宽度与路由语言
- `VertexData` 的图形身份
- 上方 shader 产物到 `GPU` 的垂直通道

### What may evolve

- `SPIR-V ShaderCode` 的标签改写为 `Cooked ShaderCode`
- `Mesh`、`Material` 作为来源节点插入
- `Description` / `PSO` 退场
- 本页高亮从 Vulkan 的 `PSO` 绑定动作，切到 UE cook 资产链

### What is banned

- 仍然语义连续的对象只靠 opacity 完成切换
- 为了塞新节点而改变 `GPU` 或主轴的中心关系
- 资产节点内部继续使用装饰性横线
- 新页箭头宽度、贴边方式、进线侧别与旧页不一致

## Page 05 Layout Contract

| Subject | Type | Rationale |
| --- | --- | --- |
| `Mesh` | Node | 位于 `VertexData` 左侧，并与其共水平线，强调资产到运行时输入的映射 |
| `VertexData` | Node | 仍然保留图形表达，不只是文字标签 |
| `Material` | Node | 位于上方资产带，与 `Cooked ShaderCode` 构成干净的水平链 |
| `Cooked ShaderCode` | Node | 位于 `GPU` 上方中轴，继承前页 shader 产物位置记忆 |
| `Cooked ShaderCode -> GPU` | Edge | 垂直下送，延续前页从 shader 产物通向运行时核心的阅读方式 |

## Validation Target

这一轮修改算成功，需要同时满足：

- `page_05` 最终态仍然看得见 `VertexData` 图形表达
- `Mesh` 在主轴左侧，并与 `VertexData` 做水平接入
- `Mesh` / `Material` 使用浅绿色资产风格，且无内部装饰条纹
- `page_04 -> page_05` 中途帧能看出承接关系，不是纯淡入淡出
- `GPU`、主轴、箭头宽度与主要路由语言保持稳定
