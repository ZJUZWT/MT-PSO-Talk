# SlideApp Late Tail Redesign Design

**Date:** 2026-04-20  
**Scope:** `SlideApp` 后半段 `page_22` 之后的正式页面重组

## Goal

把当前“`page_23` 导入页 + 4 张占位策略页 + 2 张结尾页”的旧尾声，重构成用户已经确认过的新尾声：

1. 删除 `page_23`
2. 保留 4 张技术策略页
3. 增加 3 张平台治理页
4. 在推荐页前增加 1 张 harness 机制介绍页
5. 保留推荐页与最终引句页
6. 同步更新时间轴、转场审查与剧本文档

## Approved Structure

从 `page_22` 往后，正式结构固定为：

- `page_24` 改资源形态：时空互换
- `page_25` 改存储位置：让 IO 承担空间压力
- `page_26` 改发生时机：收集与编译解耦
- `page_27` 改执行方式：并行化批处理任务
- `page_28` 平台差异的表象：同样内容，未必落成同一组 PSO
- `page_29` 平台差异的来源：输入侧差异会直接进入 PSO 维度
- `page_30` 前置治理决定上限：把问题消灭在项目输入侧
- `page_31` 项目 harness 机制与数学约束
- `page_32` 延伸阅读 / 另一些推荐
- `page_33` 《庄子·逍遥游》

## Content Direction

- 策略页统一采用“左侧放 PSO 内具体做法，右侧放策略扩展”的版式。
- 治理页强调“现象 -> 原因 -> 结论”的三页链路，并使用用户给出的图片与代码材料。
- Harness 页不讲空泛流程，直接给出项目里真实的数学约束、评分链路和时长公式。
- 推荐页继续保留真实链接。
- 收尾页保持极简，不再塞入新信息。

## Motion Direction

- 图片页、代码页、解释页都沿用仓库 `AGENTS.md` 规定的淡入淡出逻辑，不插入背景大色块。
- 删除 `page_23` 后，`page_22 -> page_24` 成为新的 late-tail 起点。
- 新增治理页和 harness 页只增加总时长，不压缩原有基础转场时长。

## Asset Direction

- 治理图片从 `/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk-SlideApp/supplement` 引入。
- 运行时展示统一走 `SlideApp/public/supplement`，必要时复制为新的公开素材路径。
- 压缩页使用 `release/results` 下 4 台设备的压缩结果做左侧表格。
