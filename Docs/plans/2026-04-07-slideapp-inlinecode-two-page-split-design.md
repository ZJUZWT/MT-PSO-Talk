# SlideApp InlineCode 双页拆分设计

## 背景

当前 `InlineCode` 这一页同时承担了两类信息：

1. `ShaderPlatform`、`FeatureLevel × QualityLevel`、`ShaderType / VertexFactory / Permutation` 这些“区分因素到底落在哪一层”。
2. `FShader -> ResourceIndex -> FShaderMapResource_InlineCode -> FShaderMapResourceCode -> ShaderEntries[i] -> Cooked ShaderCode` 这条“运行时如何拿到 code”的执行路径。

在网页近看时两者都能勉强成立，但放进 PPT 或投影场景后，这页的信息密度明显过高，导致：

- 左侧说明表和右侧存储结构彼此争空间
- 右侧 `FShaderMapResource_InlineCode` 重点不够突出
- 字号被迫压小，远距离可读性不足

## 目标

把现有 `InlineCode` 页拆成连续的两页，让每页只承担一个教学目标：

- 第一页回答：哪些因素在 UE 的哪一层把 shader 区分开
- 第二页回答：运行时最终如何沿着索引拿到 `InlineCode` 中的 `ShaderCode`

## 方案概览

### 新第一页：区分因素在哪一层

这一页保留左侧两串的映射关系：

- 因素串：
  - `ShaderPlatform`
  - `FeatureLevel / QualityLevel`
  - `ShaderType / VertexFactory / Permutation`
- 对象串：
  - `Material`
  - `FMaterialResource`
  - `FMaterialShaderMap`

这一页的重点是“分层归属”，不是“拿 code 的细节”。

因此右侧只保留一个弱化的 `FShaderMapResource_InlineCode` 锚点，或者只保留一条很轻的后续指向关系，用来告诉观众“下一页会继续展开”，但不在这一页展开 `ShaderEntries[i]`、`ShaderHashes[i]` 和 `Cooked ShaderCode`。

### 新第二页：放大 InlineCode 存储结构

这一页把左侧分层提示全部退场：

- 删除 `ShaderPlatform` / `FeatureLevel / QualityLevel` / `ShaderType / VertexFactory / Permutation` 三张表
- 删除 `Material`、`FMaterialResource`、`FMaterialShaderMap` 的阴影层
- 只保留最小必要锚点，例如：
  - `FMaterialShaderMap`
  - `FShader`
  - `ResourceIndex = i`

腾出来的空间全部让给右侧：

- 放大 `FShaderMapResource_InlineCode`
- 在其中清晰展开 `FShaderMapResourceCode`
- 强调 `ShaderEntries[i]`
- 保留 `ShaderHashes[i]` 作为旁路元数据
- 用一条清晰水平箭头连接到 `Cooked ShaderCode`

这一页的重点不再是“有哪些区分因素”，而是“最终怎么命中 code”。

## 页间演化关系

### 第一页到第二页

页间必须保持“同一骨架继续推进”，不能像换了一张完全不同的图：

- `FMaterialShaderMap` 到 `FShader` 这条链继续保留
- 左侧三张表和阴影卡片从“强调信息”变成“退场信息”
- 右侧 `FShaderMapResource_InlineCode` 从弱锚点演化为主角
- 镜头感上应该是“左边退场，右边接管舞台”，而不是整页重排

### 第二页到后续 PSO / SharedCode

拆页后，后续内容顺延一页更合理：

- `PSO cache 为什么只存 Hash` 接在新的第二页之后
- `SharedCode 为什么成为必需` 再往后顺延

这样叙事变成：

1. 区分因素在哪一层
2. InlineCode 如何拿到 code
3. PSO cache 只存 Hash
4. SharedCode 为什么成为必需

## 视觉规则

- 第一页优先保证左侧两串字号足够大，适合投影阅读
- 第二页优先保证右侧 `InlineCode` 结构字号足够大，适合投影阅读
- 第二页右侧不再保留无意义大留白，重点区域要充分占满
- `ShaderHashes[i]` 继续作为旁路信息，不抢主链风头
- `Cooked ShaderCode` 继续与 `ShaderEntries[i]` 水平对齐，保持因果清晰

## 验收标准

- 第一页观众能一眼回答“这些区分因素分别属于哪一层”
- 第二页观众能一眼回答“运行时是怎么沿着 `i` 拿到 code 的”
- 两页都能在 PPT 场景下保证主要节点与关键标签可读
- 两页之间看起来像一个结构被继续展开，而不是重新画了一张图
