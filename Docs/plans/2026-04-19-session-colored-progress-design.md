# Session Colored Progress Design

**Goal:** 让底栏 progress rail 中不同 `session` 的步骤点点拥有可辨识的分组颜色，同时不破坏当前页展开态的主高亮。

## Decision

- 为底栏 `progress bubbles` 引入基于 `session` 的颜色分组。
- 紧凑未展开的小圆点使用更明确的 session 色，帮助用户一眼看出 session 边界。
- 展开态的大 pill 只做轻量 session 偏色，保留现有“当前页更亮、更抬起”的高亮体系。
- 不修改 step 顺序、session 分组、底栏交互行为或动画时长。

## Rationale

- 用户的核心目标是“很好地区别不同 session 之间的点点”，因此最重要的是 compact dot 的分组可读性。
- 如果展开态完整改成强色，会和当前页高亮抢视觉主导权；轻量偏色更稳。
- 现有 storyboard 已经有 `sessions` 定义，可以直接复用，不需要额外维护一套 step -> color 映射表。

## Visual Intent

- `Session 1-6` 各有一组温和但可辨识的色相。
- 未展开 dot：更明显的填充/边框差异。
- 展开 pill：同色系浅底 + 轻边框，当前页仍使用更高亮的状态层。

## Scope

- 允许修改：`SlideApp/src/components/ProgressBubbles.tsx`、`SlideApp/src/app.css`、相关测试。
- 不允许修改：底栏步骤顺序、session 划分、当前页切换逻辑。
