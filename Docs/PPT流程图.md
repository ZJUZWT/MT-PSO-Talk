# UE Shader 管线与 PSO —— 一张图讲透

> **分享主题**：从 `A → f(x) → B` 出发，一张图不断演化，理解 UE Shader 管线与 PSO 机制
> **核心方法**：全程只用**一张不断生长的图**，每个阶段在图上"长出"新东西
> **预计时长**：30 分钟（5个演化阶段 × ~5min + 结语 ~3min）

---

## 演讲节奏总览

| 阶段 | 图的演化 | 时间 | 一句话 |
|------|---------|------|--------|
| Lv.0 | 最简模型 | 1 min | 渲染就是 A → f(x) → B |
| Lv.1 | 展开函数内部 | 4 min | f(x) = Shader + 管线状态，OpenGL vs Vulkan |
| Lv.2 | 映射到 UE + InlineCode | 5 min | UE 怎么组织这些东西，InlineCode 的问题 |
| Lv.3 | ShareCode + PSO 登场 | 7 min | 为什么需要全局 ShaderArchive，PSO 是什么 |
| Lv.4 | PSO 全流程 + 难题 | 8 min | 收集→预编译→使用，以及为什么这是个难题 |
| Lv.5 | PSO 编译内部 + 数据 | 3 min | 驱动层到底在干什么，实际耗时数据 |
| 结语 | — | 2 min | AI 时代的三个维度 |

---

## Lv.0 —— 最简模型（1 min）

> **PPT 动作**：屏幕上只出现一个极简的图

```
┌───────────┐         ┌──────────┐         ┌───────────┐
│           │         │          │         │           │
│  A (输入)  │───────→│  f(x)    │───────→│  B (输出)  │
│           │         │          │         │           │
└───────────┘         └──────────┘         └───────────┘
```

**讲述**：
> "渲染的本质，就是一个函数调用。给它输入数据 A，经过某种计算 f(x)，产出结果 B。
> 今天我们要做的事情，就是不断地把这张图展开、变复杂，最终理解 UE 的整个 Shader 管线和 PSO 机制。"

---

## Lv.1 —— 展开 f(x) 的内部（4 min）

> **PPT 动作**：f(x) 这个方块"裂开"，展示内部结构；A 和 B 也标注具体含义

```
                          ┌─────────────────────────────────────┐
                          │            f(x) 展开                │
┌──────────────┐          │  ┌─────────────────────────────┐    │          ┌──────────────┐
│  A = 顶点数据 │          │  │  Shader 程序 (VS/PS/GS...)  │    │          │  B = 像素     │
│  · 位置       │─────────→│  │  "做什么计算"               │    │─────────→│  · 颜色       │
│  · 法线       │          │  └─────────────────────────────┘    │          │  · 深度       │
│  · UV        │          │  ┌─────────────────────────────┐    │          │  · G-Buffer   │
│  · 颜色       │          │  │  管线状态 (State)            │    │          │  · 或其他     │
└──────────────┘          │  │  "怎么做这个计算"             │    │          │    Buffer     │
                          │  │  · BlendState               │    │          └──────────────┘
                          │  │  · DepthStencilState        │    │
                          │  │  · RasterizerState          │    │
                          │  │  · RenderTarget 格式         │    │
                          │  └─────────────────────────────┘    │
                          └─────────────────────────────────────┘
```

**讲述要点**：

1. **Shader 是"做什么"**：顶点怎么变换、像素怎么着色
2. **管线状态是"怎么做"**：要不要深度测试、怎么混合、怎么光栅化
3. **两者合在一起才是完整的渲染管线**

然后引出 OpenGL vs Vulkan 的关键区别（口述，不需要单独的 PPT 页）：

> "在 OpenGL 里，管线状态是零散设置的，驱动在 Draw 时才偷偷组装完整管线——可能卡。
> 在 Vulkan/Metal/D3D12 里，你必须**提前把 Shader + State 打包成一个 PSO**，创建 PSO 很重，但用起来很快。
> 这个'提前打包'的需求，就是后面所有故事的起点。"

**可选**：如果听众对图形 API 不熟，可以快速展示一下 OpenGL 和 Vulkan 的关键代码对比（各 5 行伪代码）：

```
// OpenGL：零散设置，Draw 时隐式编译
glUseProgram(shader);
glEnable(GL_DEPTH_TEST);        // 状态零散设置
glBlendFunc(GL_SRC_ALPHA, ...);
glDrawArrays(GL_TRIANGLES, ...); // 驱动在这里偷偷编译管线

// Vulkan：提前打包成 PSO
VkGraphicsPipelineCreateInfo info = {
    .pStages = {VS, PS},         // Shader
    .pDepthStencilState = ...,   // 状态一次性打包
    .pColorBlendState = ...,
};
vkCreateGraphicsPipelines(info); // ← 重操作！提前创建
vkCmdBindPipeline(pso);         // 运行时只需绑定，很快
vkCmdDraw(...);
```

---

## Lv.2 —— 映射到 UE + InlineCode（5 min）

> **PPT 动作**：在 Lv.1 的图上，给每个部分标注 UE 中的对应概念，然后在右侧"长出" uasset 的存储结构

```
                          ┌─────────────────────────────────────┐
                          │            f(x) = UE 渲染管线       │
┌──────────────┐          │                                     │          ┌──────────────┐
│  A = VF      │          │  Shader = FShader                   │          │  B = RT      │
│ VertexFactory│─────────→│  · TBasePassVS / TBasePassPS        │─────────→│ RenderTarget │
│ · Local      │          │  · TShadowDepthVS                   │          │ · G-Buffer   │
│ · GPUSkin    │          │  · 由 Material 编译产生              │          │ · ShadowMap  │
│ · Landscape  │          │                                     │          │ · SceneColor │
└──────────────┘          │  State = 管线状态                    │          └──────────────┘
                          │  · BlendState                       │
                          │  · DepthStencilState                │
                          │  · RasterizerState                  │
                          └──────────┬──────────────────────────┘
                                     │
                                     │ Shader 从哪来？
                                     ▼
                          ┌─────────────────────────────────────┐
                          │  UMaterial                           │
                          │    ↓ Cook 编译                       │
                          │  FMaterialResource × N              │
                          │  (FeatureLevel × Quality × Dynamic) │
                          │    ↓                                 │
                          │  FMaterialShaderMap                  │
                          │  (ShaderType × VF × Permutation)    │
                          │    ↓ 编译产物                        │
                          │  FShaderMapResourceCode              │
                          │  · ShaderHashes[] (每个shader的hash) │
                          │  · ShaderEntries[] (每个shader的code)│
                          │  · ResourceHash = SHA1(all hashes)  │
                          └─────────────────────────────────────┘
```

**讲述要点**：

1. **UE 的 A/f(x)/B 对应关系**：VertexFactory / FShader+State / RenderTarget
2. **Material 编译链**：UMaterial → FMaterialResource × N → ShaderMap → 编译产物
3. **两种 Hash**：
   - `ShaderHash`：单个 shader 的 hash（一个 VS 或一个 PS）
   - `ResourceHash`：整个 ShaderMap 的聚合 hash = SHA1(所有 ShaderHash 拼接)

然后引出 **InlineCode 模式**：

> "编译完了，这些 ShaderCode 存哪？最简单的方式——直接塞进 uasset 里。"

```
┌─────── uasset (InlineCode) ───────┐
│  FrozenContent (元数据)            │
│  ShaderCode (VS code + PS code)   │  ← 100 个材质用同一个 VS？存 100 份！
└───────────────────────────────────┘
```

**一句话点出问题**：
> "InlineCode 简单直接，但有两个致命问题：**包体膨胀**（同一个 shader 存 N 份），以及**无法支持 PSO 预编译**——为什么？下一步就知道了。"

---

## Lv.3 —— ShareCode + PSO 登场（7 min，核心章节）

> **PPT 动作**：图的左下角"长出"全局 ShaderArchive；图的右侧"长出" PSO 缓存系统。这是信息量最大的一步。

### 第一步：ShareCode 解决存储问题

```
┌─── uasset (ShareCode) ───┐          ┌──── .ushaderbytecode (全局) ────┐
│  FrozenContent (元数据)    │          │  ShaderHashes[]  (全局去重)     │
│  ResourceHash (仅一个hash) │─────────→│  ShaderEntries[] (全局去重)     │
│  （不再存 ShaderCode！）   │  查找     │  ShaderMapEntries[]             │
└───────────────────────────┘          │  ShaderIndices[] (间接索引表)   │
                                       └─────────────────────────────────┘
```

**讲述**：
> "ShareCode 模式把所有 ShaderCode 集中到一个全局文件（.ushaderbytecode），uasset 里只存一个 ResourceHash 作为索引。
> 100 个材质用同一个 VS？全局只存 1 份。包体直接缩小。"

### 第二步：PSO 为什么需要 ShareCode

> **PPT 动作**：在图的最右侧，从 f(x) 那个框"长出" PSO 的概念

```
                    ┌─────────────────────────────────────┐
                    │  PSO = Shader + State 的完整打包     │
                    │  ┌───────────────────────────────┐  │
                    │  │ VS Hash: a1b2c3...            │  │
                    │  │ PS Hash: d4e5f6...            │  │
                    │  │ BlendState: {...}              │  │
                    │  │ DepthStencilState: {...}       │  │
                    │  │ RasterizerState: {...}         │  │
                    │  │ RT Format: {...}               │  │
                    │  └───────────────────────────────┘  │
                    │  存储在 .upipelinecache 文件中       │
                    └──────────────────┬──────────────────┘
                                       │
                    预编译时需要：Hash → Code
                                       │
                    ┌──────────────────▼──────────────────┐
                    │  InlineCode: ❌ Code 散落在各 uasset │
                    │  → 没有全局 Hash→Code 的反查能力      │
                    │  → PSO 系统直接不初始化！              │
                    │                                      │
                    │  ShareCode: ✅ 全局 ShaderArchive     │
                    │  → FShaderCodeLibrary::CreateShader   │
                    │    (Hash) → 直接找到 Code             │
                    └──────────────────────────────────────┘
```

**讲述**：
> "PSO 缓存文件里只存了 Shader 的 Hash，不存 Code。预编译时需要'给我一个 Hash，还我一份 Code'。
> InlineCode 做不到——Code 散落在每个 uasset 里，没有全局索引。
> ShareCode 做得到——全局 ShaderArchive 就是一个 Hash→Code 的大字典。
> **所以 PSO 缓存必须依赖 ShareCode 模式。** 代码里有硬性检查：`FShaderCodeLibrary::IsEnabled()` 为 false 时，PSO 系统直接跳过初始化。"

---

## Lv.4 —— PSO 全流程 + 为什么是难题（8 min）

> **PPT 动作**：在图上画出 PSO 的完整生命周期——一个循环

### 完整的图（Lv.4 最终形态）

```
┌─────────────────────────────────────────────────────────────────────────────────┐
│                                                                                 │
│   ┌──────────┐       ┌──────────────────────┐       ┌──────────┐               │
│   │ A = VF   │──────→│ f(x) = Shader+State  │──────→│ B = RT   │               │
│   │ 顶点数据  │       │                      │       │ 像素输出  │               │
│   └──────────┘       └──────────┬───────────┘       └──────────┘               │
│                                 │                                               │
│                    ┌────────────┼────────────┐                                  │
│                    │            │            │                                   │
│                    ▼            ▼            ▼                                   │
│              ┌──────────┐ ┌─────────┐ ┌──────────────────────┐                  │
│              │ Shader   │ │  State  │ │  PSO = 两者的打包     │                  │
│              │ 从哪来？  │ │ Blend   │ │  存储: .upipelinecache│                  │
│              │          │ │ Depth   │ │  内容: ShaderHash     │                  │
│              │          │ │ Raster  │ │      + State 描述     │                  │
│              └────┬─────┘ └─────────┘ └──────────┬───────────┘                  │
│                   │                               │                              │
│                   ▼                               │                              │
│   ┌───────────────────────────┐                   │                              │
│   │ .ushaderbytecode (全局)   │                   │                              │
│   │ Hash → Code 的大字典      │◄──────────────────┘                              │
│   │ ShareCode 模式必须开启    │   预编译时反查                                    │
│   └───────────────────────────┘                                                  │
│                                                                                  │
│   ┌──────────────────── PSO 生命周期 ────────────────────┐                       │
│   │                                                      │                       │
│   │  ① 收集 ──→ ② 保存 ──→ ③ 下次启动加载 ──→ ④ 预编译  │                       │
│   │  运行时记录   .upipelinecache   排序+分批     Expand   │                       │
│   │  新遇到的PSO                                 + Build  │                       │
│   │                                                ↓      │                       │
│   │                              ⑤ 运行时使用 ←── 缓存结果 │                       │
│   │                              命中=零延迟               │                       │
│   │                              未命中=卡顿！             │                       │
│   └──────────────────────────────────────────────────────┘                       │
└─────────────────────────────────────────────────────────────────────────────────┘
```

### 讲述 PSO 生命周期（5 步）

**① 收集**：运行时首次遇到新的 Shader+State 组合 → 从 RHIShader 取 Hash → 记录
- `r.ShaderPipelineCache.LogPSO = 1`
- 需要人工/自动化遍历所有场景

**② 保存**：定期保存到 `.upipelinecache` 文件

**③ 加载**：下次启动时读取，按优先级排序（使用频率、首次出现帧）

**④ 预编译（Expand + Build）**：
- **Expand**：ShaderHash → 查 ShaderArchive → 取 Code → 创建 RHIShader 对象
- **Build**：组装完整 PSO → 提交给驱动编译（`vkCreateGraphicsPipelines`）
- 分批进行：`Precompile` / `Fast` / `Background` 三种 BatchMode

**⑤ 使用**：运行时命中缓存 → 零延迟；未命中 → 实时创建 → 卡顿

### 为什么 PSO 是一个难题（3 min）

> **PPT 动作**：在图的周围标注"难点"气泡

```
        ┌─────────────────────┐
        │ 难点1: 收集的两难    │
        │ 收集多→编译慢→启动慢 │
        │ 收集少→运行时卡顿    │
        └─────────┬───────────┘
                  │
    ┌─────────────┼─────────────────┐
    │             │                 │
    ▼             ▼                 ▼
┌────────┐  ┌──────────┐  ┌──────────────┐
│难点2    │  │难点3      │  │难点4          │
│多地图   │  │设备碎片化 │  │编译本身很重   │
│不同地图 │  │不同GPU    │  │SPIR-V→GPU ISA│
│PSO集合  │  │不同驱动   │  │寄存器分配     │
│完全不同 │  │编译结果   │  │是NP-hard问题  │
│         │  │都不一样   │  │的近似求解     │
└────────┘  └──────────┘  └──────────────┘
```

**快速口述四个难点**：

1. **收集的两难**：收集多了启动编译慢，收集少了运行时卡顿
2. **多地图问题**：不同地图的材质组合不同，PSO 集合差异大
3. **设备碎片化**：同一个 PSO，Adreno/Mali/PowerVR 编译结果不同，某些设备可能编译失败
4. **编译本身很重**：驱动层需要做 SPIR-V→GPU ISA 的编译，其中寄存器分配是 NP-hard 问题的近似求解，占编译时间的 50-60%

---

## Lv.5 —— PSO 编译内部（3 min，快速过）

> **PPT 动作**：把 Lv.4 图中"Build"那个步骤"放大"

```
vkCreateGraphicsPipelines() 内部发生了什么？

SPIR-V 字节码
    ↓ (~5%)   格式转换
NIR 中间表示
    ↓ (~20%)  优化 Pass（常量折叠、死代码消除...）
NIR Lowered
    ↓ (~60%)  ★ 最耗时！ISA 编译（指令选择 + 寄存器分配 + 调度）
GPU 二进制指令
    ↓ (~10%)  管线组装
完整 PSO
```

**讲述**：
> "Mesa 开源驱动让我们能看到 PSO 编译的内部。最耗时的是 ISA 编译——把中间表示翻译成 GPU 能执行的机器码。
> 其中寄存器分配占了大头，因为 GPU 的寄存器数量有限，分配问题本质上是 NP-hard 的。
> 好消息是 `VkPipelineCache` 可以缓存编译结果，第二次创建同样的 PSO 几乎瞬时。
> 坏消息是这个缓存是设备本地的，换设备/更新驱动就失效了。"

**可选**：如果有实际数据，快速展示一个表格：

| 设备 | 2000 个 PSO 总编译时间 | 平均单个 |
|------|----------------------|---------|
| Adreno 750 | ~Xs | ~Xms |
| Mali-G715 | ~Xs | ~Xms |
| A17 Pro (Metal) | ~Xs | ~Xms |

> （需要你自己填入实测数据）

---

## 结语 —— AI 时代的三个维度（2 min）

> **PPT 动作**：回到最初的 `A → f(x) → B`，但这次 A/f(x)/B 换成人的隐喻

```
┌───────────────┐       ┌──────────────────┐       ┌───────────────┐
│  A = 你自己    │──────→│  f(x) = 你的能力  │──────→│  B = 你的输出  │
│               │       │                  │       │               │
│  · 身体 🏃    │       │  · 审美判断力 🎨  │       │  · 创造价值    │
│    锻炼是基础  │       │    AI能生成       │       │  · 参与世界    │
│               │       │    但不能"感受"    │       │    的运转      │
│               │       │  · 金融理解力 💰  │       │               │
│               │       │    理解资本运作    │       │               │
│               │       │    参与"生产"     │       │               │
└───────────────┘       └──────────────────┘       └───────────────┘
```

> "最后回到我们的核心图。人也是一个 `A → f(x) → B` 的系统。
> A 是你的身体——这是硬件，AI 替代不了，要锻炼。
> f(x) 是你的能力——AI 时代，**审美判断力**（能分辨好坏比能执行更重要）和**对金融资本主义的理解**（从被雇佣到参与资本运作）是两个关键函数。
> B 是你的输出——不只是写代码，而是创造价值、参与世界的运转。"

---

## 一张图的完整演化路径（给自己看的备忘）

```
Lv.0:  A ──→ f(x) ──→ B                          （1min，建立模型）
         │
Lv.1:  展开 f(x) = Shader + State                  （4min，OpenGL vs Vulkan）
         │
Lv.2:  标注 UE 概念 + 长出编译链 + InlineCode       （5min，UE映射+问题）
         │
Lv.3:  长出 .ushaderbytecode + PSO 概念             （7min，ShareCode+PSO为什么需要它）
         │
Lv.4:  长出 PSO 生命周期环 + 难点气泡               （8min，全流程+难题）
         │
Lv.5:  放大 Build 步骤 → 驱动内部                   （3min，编译耗时分析）
         │
结语:   A→f(x)→B 的人生隐喻                        （2min，三个维度）
```

> **关键 PPT 技巧**：每一页都是在上一页的基础上"加东西"，而不是换一张全新的图。
> 建议用 PPT 动画的"出现"效果，让新元素逐步浮现。
> 或者用一个大画布（如 Figma/Miro），每次 zoom in 到新区域。

---

## 需要准备的材料清单（精简版）

### 必须准备
- [ ] **一张可以逐步展开的核心图**（建议用 Figma/PowerPoint 做，支持动画逐步展开）
- [ ] OpenGL vs Vulkan 的 5 行伪代码对比
- [ ] 项目中 InlineCode vs ShareCode 的包体大小对比数据（一个数字即可）
- [ ] PSO 预编译的运行时日志截图（展示编译数量和耗时）

### 有则更好
- [ ] 不同设备的 PSO 编译耗时实测数据
- [ ] PSO 收集覆盖率数据
- [ ] Mesa 编译流程的参考图

### 关键代码片段（备用，听众问到再展示）
- [ ] `FShaderPipelineCache::Initialize` 中的 `FShaderCodeLibrary::IsEnabled()` 检查
- [ ] `FPipelineCacheFileFormatPSO::GraphicsDescriptor` 结构体
- [ ] `FShaderMapResource_SharedCode::CreateRHIShader` 的索引转换逻辑

### 参考资料
- [ ] ShaderCode 教程文档（你自己写的那份）
- [ ] Mesa ACO 编译器文档：https://gitlab.freedesktop.org/mesa/mesa/-/blob/main/src/amd/compiler/README.md
- [ ] Vulkan Pipeline Cache 规范：https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineCache.html
