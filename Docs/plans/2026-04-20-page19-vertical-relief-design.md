# Page 19 Vertical Relief Design

**Date:** 2026-04-20  
**Scope:** `SlideApp` `page_19` 正式动画页

## Goal

在不改动 `page_19` 主叙事结构和横向读法的前提下，按当前 harness 规则对这页做一次“纵向释压”调整：保留现有观感基调，只利用纵向空间去修正字体、内边距和 GPU 竖链端点问题。

## User-Approved Direction

用户已经从 3 套方案中选择：

- `C = 纵向释压`

这意味着本次调整优先级为：

1. 保留当前页的整体观感和模块关系
2. 不做明显横向重排
3. 用更合理的纵向空间分配去改善 harness 事实

## Existing Problems To Target

以当前 `page_19` formal sketch 的几何审校为准，现状核心问题是：

- `nodePierceCount = 2`
- `minRenderedFontPx = 15`
- `minInternalPadding = 4`

同时，页面仍然有较大的上下留白：

- `topMargin = 196`
- `bottomMargin = 196`

因此本次不应该继续做横向压缩，而应优先回收纵向空白预算。

## Page Contract

### Page goal

让观众继续按如下顺序读完页面：

`stable.upipelinecache -> UE PSO -> GPU 竖列 -> 内存中 PSO -> 硬盘中的 PSO`

### Node inventory

- `stable-cache-node`
  - displayed text: `stable.` / `upipelinecache`
  - role: 主输入文件节点
  - visual kind: 强调型文件节点
  - region: 左侧主轴起点
  - continuity: 保持本页既有角色，不变更语义

- `uepso-group`
  - displayed text: `UE PSO`, `PSO 1`, `PSO 2`, `PSO ...`
  - role: UE PSO 聚合组
  - visual kind: 外层组框 + 3 个行内子框
  - region: 左中
  - continuity: 保留当前结构，只调整纵向间距和行盒高度

- `gpu-stack`
  - displayed text: `GPU`
  - role: page14 风格的竖向运行时链
  - visual kind: 上三角图标 + 中间 GPU 文本 + 下像素图标
  - region: 中轴
  - continuity: 必须保持 page14 同款竖向读法，不允许横摊

- `memory-group`
  - displayed text: `内存中 PSO`, `OpenGL / Program Binary`, `Vulkan / Pipeline Cache`, `Metal / Binary Archive / 系统管理`
  - role: API 运行时内存态组
  - visual kind: 外层组框 + 3 个子框
  - region: 右中
  - continuity: 保留现有语义和横向关系，只增加纵向呼吸空间

- `disk-group`
  - displayed text: `硬盘中的 PSO`, `Program Binary / Cache`, `VulkanPSO.cache`, `BinaryArchive / functions.data`
  - role: 本地缓存组
  - visual kind: 外层组框 + 3 个子框
  - region: 最右
  - continuity: 保留现有语义和三条虚线映射

### Edge inventory

- `stable-to-ue`
  - source/target: `stable -> UE PSO`
  - meaning: stable 输入进入 UE 聚合
  - route grammar: 水平直线
  - max bend: `0`

- `ue-to-gpu`
  - source/target: `UE PSO -> GPU`
  - meaning: UE 聚合送入 GPU 竖链
  - route grammar: 水平直线
  - max bend: `0`

- `vertex-to-gpu`
  - source/target: 上三角图标 -> GPU
  - meaning: page14 风格的上半链
  - route grammar: 竖向直线
  - max bend: `0`
  - special rule: 必须准确落在 GPU 边界，不允许穿入文字节点内部

- `gpu-to-pixel`
  - source/target: GPU -> 下像素图标
  - meaning: page14 风格的下半链
  - route grammar: 竖向直线
  - max bend: `0`
  - special rule: 必须准确从 GPU 边界出发，不允许穿入文字节点内部

- `gpu-to-memory`
  - source/target: `GPU -> 内存中 PSO`
  - meaning: GPU 产出进入 API 内存态
  - route grammar: 水平直线
  - max bend: `0`

- `gl-to-disk`, `vk-to-disk`, `metal-to-disk`
  - source/target: 3 个 API 内存节点 -> 3 个本地缓存节点
  - meaning: 本地缓存映射
  - route grammar: 水平虚线
  - max bend: `0`
  - sibling rule: 三条边必须同风格、同节奏、同样水平

### Spatial constraints

- 主中轴保持 `y = 360`
- 横向顺序不变：`stable | ue | gpu | memory | disk`
- 纵向释压主要作用于：
  - `UE` 组的总高度和 3 行子框高度
  - `GPU` 上图标 / 文本 / 下图标间距
  - `memory` / `disk` 两组的总高度和 3 行子框高度
- `memory` 与 `disk` 顶/底边继续同线，保持右半区对齐感
- 允许做很小的横向微移，但不允许改变这 5 段相对读法

### Element styles

- 保持现有浅底组框、浅色节点、克制箭头头尺寸
- 不新增背景大色块
- 不新增折线
- 不增删页面语义节点，只调整尺寸、位置和端点

### Assembly order

1. 固定横向五段关系
2. 先扩 `UE` / `memory` / `disk` 的纵向框体
3. 再调整内部 3 行子框的高度与垂直分布
4. 再拉开 `GPU` 上下图标与文字的垂直间距
5. 最后把 `vertex-to-gpu` 与 `gpu-to-pixel` 的端点对齐到 GPU 边界

### Acceptance checklist

1. 本页仍然一眼读成 `stable -> UE PSO -> GPU -> 内存中 PSO -> 硬盘中的 PSO`
2. `GPU` 仍然保持 `page_14` 风格的竖向三段
3. 不出现新的横向重排感
4. `nodePierceCount` 要优先从 `2` 降下去
5. `minRenderedFontPx` 和 `minInternalPadding` 必须比当前值改善
6. `page_19 -> page_21` 的时长与过渡逻辑不变
