# Opening Question Page Design

## Goal

在现有 `page_01` 前面新增一张真正的开场页，把整段分享先收束成一个“问题入口”：

- 用 `page_13_img` 的卡顿证据图先说明“我们确实在解决一个真实痛点”
- 用 `page_28` 的两张样本图先说明“同样内容，结果未必一样”
- 把后文要逐步回答的 6 个问题一次性摆出来

这样观众在进入最小模型 `Input -> f(x) -> Output` 之前，已经知道整段分享到底在回答什么问题。

## Chosen Approach

采用用户确认的 `A / 1` 方案：

- 只新增 **1 张** 总开场页，不拆成多张前置插页
- 版式采用 **左大图 + 右问题列表 + 底部双样本图**
- 这张页本身不回答问题，只负责把问题抛出来；答案留在后续页面逐步展开

## Page Role

这张新页会成为新的第一步，建议 step id 使用 `page_00`。

- 现有 `page_01` 不删除，不重写核心语义，只顺延成第二张页
- `page_00` 的职责是“问题驱动开场”
- `page_01` 继续承担“最小模型起手式”

这比直接把问题塞进现有 `page_01` 更稳，因为：

- 不会破坏 `page_01` 目前已经稳定的最小模型语义
- 不会把问题页和公式页压成一张高密度混合页
- 时间轴上可以通过单独的 `page_00 -> page_01` 转场实现“问题页淡出、最小模型淡入”

## Visual Structure

### 左侧主视觉

- 复用 `page_13_img` 所对应的主图：`/supplement/pso-stutter.png`
- 这张图承担“运行时编译导致卡顿”的第一层情绪锚点
- 尺寸应明显大于底部两张样本图，成为全页第一视觉中心

### 右侧问题区

- 用一张纯文本卡片列出 6 个问题
- 问题本身加粗，保持可扫读
- 不在这页提前展开完整答案，最多只保留很轻的“后文会回答”语气，不把悬念泄完

建议问题文案直接采用问题句式：

1. 为什么需要预编译着色器？
2. 为什么不能直接分发 binary？
3. 为什么需要打开 `SharedShaderCode` 开关？
4. 为什么需要 `stablepc.csv`？
5. 为什么许多游戏只需要下载安装后的首次编译？
6. `PSO Cache` 方法有副作用吗？

### 底部现象样本区

- 复用 `page_28` 的两张样本图：
  - `/supplement/ogl-mtl/opengl-compile-count.png`
  - `/supplement/ogl-mtl/ios-compile-count.png`
- 两张图只作为“样本 A / 样本 B”出现，不在开场页写平台名字
- 它们承担第二层悬念：为什么“看起来相似的内容”会落出不同结果

## Narrative Order

这张页的阅读顺序应当非常明确：

1. 先看到左侧卡顿主图，意识到“有真实痛点”
2. 再看右侧问题列表，理解整段分享的提问范围
3. 最后扫到底部双样本图，意识到“平台/状态差异”也是问题的一部分

因此这页不是海报，也不是知识总表，而是一个明确的“开题页”。

## Transition Strategy

`page_00` 作为新第一页，默认静态停留。

进入 `page_01` 时遵循现有仓库的图片/解释页规则：

1. 淡出 `page_00` 的整页内容
2. 淡入现有 `page_01` 的最小模型
3. 然后继续原有 `page_01 -> page_02` 节奏

不在 `page_00 -> page_01` 之间插入无关旧主图闪回，也不额外换大色块背景。

## Repo Fit

这个方案与当前仓库结构是匹配的：

- 图片素材都已存在于 `SlideApp/public/supplement/`
- 图片页与样本图的视觉语言已经在 `page_13_img`、`page_28` 中被验证过
- 现有 `page_01` 可保持不变，只需在时间轴前面插入一个独立 step
- Notes/Storyboard/Step rail 都已有基于 `StoryStepId` 的扩展点

## Risks To Avoid

- 不要让右侧问题列表比左侧痛点图更重，否则“为什么会卡”的开场冲击会被削弱
- 不要在底部双样本图上提前写平台名字，否则会把 `page_28` 的匿名对照效果提前剧透
- 不要在这页直接写出完整答案，否则后文章节会失去“逐步解答”的推进感
- 不要破坏现有 `page_01` 的最小模型独立性

## Validation

- 新页加入后，step 顺序、session 归属、默认 step、时间轴窗口都要同步更新
- 必须补一份新的剧本文档：`Docs/剧本/00-开场问题页.md`
- 必须更新 `page_01` 的上一页说明
- 必须跑：
  - `SlideApp` 的 storyboard / timeline / composition 定向测试
  - `python3 scripts/slide-geometry-harness/audit_transition_timings.py`
  - `python3 scripts/slide-geometry-harness/audit_storyboard_sync.py`
  - `npm --silent --prefix SlideApp run review:mechanical -- --from page_19`
