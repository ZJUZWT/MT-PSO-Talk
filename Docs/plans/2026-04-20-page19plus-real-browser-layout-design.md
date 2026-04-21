# Page 19+ Real Browser Layout Design

**Date:** 2026-04-20  
**Scope:** `page_19` 到 `page_33`

## Goal

把 `page_19+` 的正式页面排版整理到同一轮可审校状态，并且让这段页面都能从前台原生 Edge 直接拿到真实文本渲染数据，而不是只靠公式估算。

## User-Approved Direction

- 先补齐真实采集钩子
- 再逐页切换前台 Edge 页面取真实数据
- 再逐页精修排版，直到最后一页
- 不使用 headless 作为主数据源

## Constraints

- 必须先走现有 harness 数学约束，再做图像复核
- 不允许把公式估算伪装成真实数据
- 真实数据来源限定为前台 Edge 活页
- 页面改动后必须同步 formal review、机械审校和剧本

## Current Reality

- `page_21`、`page_22` 已经能从前台 Edge 拿到真实超框数据
- `page_24~31` 只接了一部分 geometry hook
- `page_19`、`page_32`、`page_33` 当前真实 probe 覆盖不足
- 当前机械低分页主要集中在 `page_24~27`，其次是 `page_28~31`

## Design

### 1. Real Probe Coverage First

先把 `page_19~33` 的主要 formal review node 全部映射到正式页面 DOM：

- 给已有复用组件补 `geometryNodeId` 能力
- 给还没有复用组件承载的页面块增加 geometry wrapper
- 让 probe 至少能稳定识别每页的主要卡片、代码块、token、footer、quote 或图示节点

### 2. Review Contract Consistency

真实 probe 只覆盖“文本测量真值”：

- `textOverflowCount`
- `maxTextOverflowPx`
- `minRenderedFontPx`
- `internal padding`

非文本几何仍然沿用 formal sketch，但 formal sketch 必须与正式页面结构同构，不能继续停留在旧模板代理状态。

### 3. Page-by-Page Polishing

按页面逐页处理，而不是整组家族化统一替换：

- `page_19`
- `page_21`
- `page_22`
- `page_24`
- `page_25`
- `page_26`
- `page_27`
- `page_28`
- `page_29`
- `page_30`
- `page_31`
- `page_32`
- `page_33`

每页处理顺序固定：

1. 切到前台 Edge 当前页
2. 读取真实 probe 数据
3. 跑机械审校
4. 修改页面排版
5. 更新 formal review / 剧本
6. 再次复核

## Success Criteria

- `page_19~33` 都能从前台 Edge 拿到非空 probe 数据，或明确说明该页哪些节点是非文本几何代理
- `page_21`、`page_22` 的真实超框问题被修掉
- `page_24~31` 的主要低分页问题被逐页缓解
- `page_32`、`page_33` 有真实 probe 覆盖
- 完成后跑完整 transition audit、storyboard sync、mechanical review
