# Page 32 Vertical Feedback Chain Design

## Goal

把当前 `page_32` 从“纯文字桥页”改成一张更像图的抽象结构页：

- 复用前面已经成熟的圆角卡块语言
- 中间做单列纵向主链
- 用更少文字讲清 `harness -> loss + back propagation -> feedback system -> Input / f(x) / Output`

## Approved Direction

### Core structure

- 页面上方保留标题：`反馈系统与人的学习`
- 主舞台中间只放 4 个卡块，自上而下排列：
  - `harness`
  - `loss + back propagation`
  - `feedback system`
  - `Input / f(x) / Output`

### Arrow logic

- 主链用 3 条竖直向下直箭头串起来
- 再从 `feedback system` 补 1 条向上的直箭头指回 `harness`
- 这一条回指要明确表达：`feedback system` 本身又能落回具体系统，而不是单向终点

### Visual language

- 卡块样式复用 `page_31` 那套圆角浅底 + 橙色强调边框语言
- 页面气质偏“抽象函数图”而不是“说明文页”
- 不再让 3 行英文长句占据主舞台
- 底部只保留 1 句很轻的收束句，用来带出“看似无用的东西”

## Layout Decision

- 主链整体居中，单列排布
- 每个卡块横向宽度一致，视觉上形成很强的中轴
- `harness` 与 `feedback system` 用强调卡，`loss + back propagation` 与 `Input / f(x) / Output` 用中性卡
- 回指箭头尽量简单直接，不再画成复杂回环框架，避免抢主链

## Narrative Rationale

- `page_31` 已经把真实 harness 回环讲得很具体
- `page_32` 不需要再继续解释细节，而是把这条具体回环往上抽象
- 用“图”替换“句子”，可以让观众更快抓住这页的职责：
  - 它是一张结构桥页
  - 不是一张工程说明页
  - 也不是最终结论页

## Verification Targets

- 屏幕中心一眼就能读出 4 级抽象链
- 主链自上而下清楚成立
- `feedback system -> harness` 的回指能被看懂，但不打乱主阅读流
- 页面整体比当前文字版更轻、更像图
- `page_31 -> page_32` 与 `page_32 -> page_33` 时长仍需维持 `in_range`
