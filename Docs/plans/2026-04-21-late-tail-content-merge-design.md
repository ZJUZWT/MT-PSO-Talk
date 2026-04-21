# Late Tail Content Merge Design

**Date:** 2026-04-21  
**Scope:** `page_19 / page_24 / page_29` 内容吸收式合并；保留 `page_21 / page_25 / page_27 / page_30` step 与动画位点

## Goal

在不删除 late-tail 现有 step、不过早改动画页数的前提下，先完成内容层合并：

1. `page_19` 吸收 `page_21` 的核心结论
2. `page_24` 吸收 `page_25 + page_27` 的策略内容
3. `page_29` 吸收 `page_30` 的治理结论

本轮重点不是删页，而是把主叙事压缩到更少的宿主页里，让信息密度更聚焦。

## Approved Hosting

- `page_19` 继续作为“预编译落地页”，但在主图下方追加一段 summary 收束，重点改成：
  `binary / local cache` 不稳定，稳定缓存只能前移当前版本的可复用成本。
- `page_24` 改成三行策略总页：
  - 第一行：左 `压缩`，右 `实测压缩数据长条`
  - 第二行：左 `LRU + mmap`，右 `SQL 等额外可选项`
  - 第三行：左 `并行`，右 `任务独立 / 纯 CPU 计算`
- `page_29` 改成“代码 + 图片”双列根因/治理页：
  - 上半区保留两段代码
  - 下半区把两张图片分别放到对应代码下方
  - 去掉原有 tab 组织

## Content Direction

### `page_19`

- 保留 `stable.upipelinecache -> UE PSO -> GPU -> 内存态 / 本地缓存态` 的主中轴。
- 不再把“失效条件”拆成整页 summary 板，而是在当前页就收成一句更强的工程判断：
  `我们真正要强调的不是缓存永远有效，而是 binary / 本地缓存天然不稳定。`
- 下方 summary 只保留与本页主图强相关的三类不稳定源：
  - `Shader / State` 变化
  - `codegen / 映射 / 构建` 变化
  - `OS / Driver / GPU / API` 变化

### `page_24`

- 不再分散成三张独立策略页来讲“资源形态 / 存储位置 / 执行方式”。
- 新页按三行压缩：
  - 行 1 只讲 `Compression`
  - 行 2 只讲 `Storage / IO`
  - 行 3 只讲 `Execution`
- 第一行右侧压缩数据只保留最亮眼、最有课堂记忆点的算法：
  `LZ4 / ZSTD / Leviathan`
- 第二行右侧只强调“可选载体”，避免扩成存储系统百科。
- 第三行右侧只强调“为什么适合并行”，不再展开 GPU 硬件科普。

### `page_29`

- 这页从“纯代码根因页”升级成“代码证据 + 图片证据 + 治理结论”合并页。
- 左列仍围绕 `VertexDescriptor / InitRHI`。
- 右列仍围绕 `LocalVertexFactory.ush / FVertexFactoryInput`。
- 每段代码下方各放一张对应图片，让观众建立“代码输入差异 -> 实际结果差异”的直连。
- 删除 tab 结构，减少无关交互语义。

## Non-Goals

- 本轮不删除 `page_21 / page_25 / page_27 / page_30`。
- 本轮不改 late-tail step 顺序。
- 本轮不压缩动画总页数，只改各宿主页的内容承载。
- 本轮不把 `Metal Binary Archive 2` 讲成确定答案；在 `page_19` 仍只保留问号口径。

## Motion Direction

- 维持现有 step 锚点，优先保证转场不断裂。
- 如果宿主页内容增加，只允许通过内部重排和轻量 crossfade 吸收，不新增强插页。
- 原有“图片页/解释页淡入淡出，再切正式页”的规则保持不变。

## Validation Direction

- 先更新 storyboard 文案与 formal page ledger，避免渲染和剧本分工错位。
- 再更新 late-tail renderers 和 composition tests。
- 最后跑：
  - `npm --silent --prefix SlideApp run review:mechanical -- --from page_19`
  - `python3 scripts/slide-geometry-harness/audit_transition_timings.py`
  - `python3 scripts/slide-geometry-harness/audit_storyboard_sync.py`
