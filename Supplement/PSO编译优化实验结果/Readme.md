# PSO编译优化实验结果

本目录整理了一份可直接对外引用的 Shader Compile / PSO 编译优化实验归档，包含：

- 桌面端原始结果
- Android 端原始结果
- 演讲中使用的两张 compile count 配图

## 实验目的

这组实验主要想回答一个很具体的问题：

> 当渲染状态在编译期就已知时，驱动是否有机会做更激进的优化

这里选择了一个非常适合观察差异的切口：

- 在 Vertex Shader 中放入高开销循环
- Fragment Shader 只负责把结果写出去
- 然后把 `colorWriteMask` 关掉

如果 API/驱动在编译期就知道“颜色根本不会被写出”，理论上就有机会把整条无效计算链一起删掉。

## 核心结论

### 1. 桌面端结果非常明确

在 `RTX 3080 + NVIDIA 551.86` 上，Vulkan 在 `colorWriteMask=0` 时，GPU 时间几乎贴着 baseline：

- baseline: `0.0670 ms`
- Vulkan heavy shader, `loopCount=5000`, `mask=0`: `0.0645 ms`
- OpenGL heavy shader, `loopCount=5000`, `mask=0`: `32.0594 ms`

这说明在这组实验里，Vulkan 的 PSO 编译路径确实利用了编译期状态信息，做出了非常激进的跨 stage DCE。

### 2. OpenGL 没有体现出同等级别的编译期优化

OpenGL 的 `glColorMask` 是运行时状态，不是在 `glLinkProgram()` 时 baked 进去的状态。

所以即使最后颜色不写出，编译器也无法在链接阶段直接把整条计算链删掉，heavy loop 依然会明显随 `loopCount` 增长。

### 3. Android 端结果不支持直接照搬桌面结论

在本次 Android 测试里，Vulkan / GLES 在 `mask=0` 和 `mask=RGBA` 之间没有出现桌面端那种“瞬间掉回 baseline”的效果。

更准确的说法是：

- `PSO 提供了更强的编译期信息`
- 但 `驱动是否真的兑现成对应优化` 仍然取决于具体平台与实现

因此，这份实验更适合作为“PSO 能提供什么优化空间”的证据，而不是“所有 Vulkan/Metal 设备都会自动得到同样收益”的绝对证明。

## 建议讲述方式

如果要在演讲里快速引用，这组结果最适合支撑下面这句话：

> PSO 的价值不只是缓存和复用，它还可能把更多状态前移到编译期，从而为更激进的优化打开空间

同时建议配上一句保守限定：

> 这个空间是否被驱动真正兑现，要看具体平台

## 文件说明

### `桌面结果/`

- `benchmark_summary.md`
  - 英文整理版，包含实验设计、shader、计时方法与结果表格
- `benchmark_summary.txt`
  - 终端摘要输出，适合快速查数字
- `colorwrite_test.txt`
  - 桌面端关键跑数结果，最能直接看出 Vulkan / OpenGL 差异
- `benchmark_20260421_173114.txt`
  - 一份完整 benchmark 记录

### `移动结果/`

- `android_colorwrite_result.txt`
  - Android 端 Vulkan / GLES 对照结果

### `配图/`

- `opengl-compile-count.png`
  - OpenGL 相关 compile count 配图
- `ios-compile-count.png`
  - iOS / Metal 相关 compile count 配图

## 推荐阅读顺序

1. 先看 `桌面结果/colorwrite_test.txt`
2. 再看 `移动结果/android_colorwrite_result.txt`
3. 最后看 `桌面结果/benchmark_summary.md`

这样最容易先建立直觉，再回到实验设计本身。
