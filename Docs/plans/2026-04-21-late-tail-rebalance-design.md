# Late-Tail Rebalance Design

**目标**

- 重新分配 `page_19 / page_24 / page_26` 的信息密度，让 late-tail 的三页主承载都更像“一个主问题 + 两个并列优化方向”，而不是把不同层级的内容硬压在同一条细长条或三行总表里。

**用户确认后的设计结论**

- `page_19`
  - 保留原主图：`stable.upipelinecache -> UE PSO -> GPU -> 内存中 PSO -> 硬盘中的 PSO`。
  - 删除细长的 `binary-summary` 横条。
  - 改成主图下方 3 条 note，分别讲：
    - `Shader / State`
    - `codegen / 映射`
    - `OS / Driver / GPU / API`
  - `Metal Binary Archive 2 ?` 保留为单独问号 badge。
- `page_24`
  - 从“三行总览”改成“两组并列”，统一归到“包体 / 内存问题”。
  - 左右两列、上下两层：
    - 左上：`压缩`
    - 右上：压缩亮点数据 `LZ4 / zstd / Oodle Leviathan`
    - 左下：`LRU + mmap`
    - 右下：`SQL 可选 / 载体只是载体`
  - 删除本页里的“并行”宿主地位。
- `page_26`
  - 保留“地图 B 下载完成 -> 开始编译地图 B 的 PSO”的事件链与 `Game UsageMask / Compile UsageMask`。
  - 同页新增“并行”作为另一条优化路径，但不把它讲成与 `UsageMask` 同一内容。
  - 页面表达重点改成：
    - `UsageMask`：减少要编译的集合
    - `并行`：提升编译执行吞吐
  - 两者只是同属于“预编译优化”的并列手段。
- continuity
  - `page_25 / page_27` 继续保留用于动画连续性，不删 timeline。
  - `page_27` 的文案身份从“已吸收进 page24”改成“已吸收进 page26”。

**版面原则**

- 优先拉开模块层级，不再追求把所有点压在一条横向 strip 里。
- 保持“左侧讲方法名，右侧讲证据或展开”的阅读习惯。
- 不新增大色块背景，只在既有卡片语法内重排。
- `page_26` 明确避免把两个优化手段画成因果链，只画成并列双路径。

**需要同步的文件**

- 渲染：`SlideApp/src/remotion/pages/Page10Scene.tsx`
- storyboard：`SlideApp/src/storyboard-data/pso-storyboard.ts`
- 测试：`SlideApp/src/storyboard/pso-storyboard.test.ts`
- 测试：`SlideApp/src/remotion/Composition.test.tsx`
- 剧本：`Docs/剧本/19-第十九页-正式动画.md`
- 剧本：`Docs/剧本/24-第二十四页-正式动画.md`
- 剧本：`Docs/剧本/26-第二十六页-正式动画.md`
- 剧本：`Docs/剧本/27-第二十七页-正式动画.md`

**验收标准**

- `page_19` 不再依赖细长横条承载长句，改成 3 条 note 后真实浏览器文字仍为正 padding。
- `page_24` 从 3 行变 2 组后，观众能明显读出“包体 / 内存”的两类办法。
- `page_26` 同时出现 `UsageMask` 与 `并行`，但不会被误读为同一种机制。
- `page_24 -> page_25` 与 `page_26 -> page_27` 的 continuity 语义说明同步更新。
