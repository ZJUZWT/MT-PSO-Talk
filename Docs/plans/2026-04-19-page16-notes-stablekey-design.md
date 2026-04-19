# Page16 Notes StableKey Design

**Goal:** 在 `page_16` 的左侧说明面板中展示一张真实的 `ShaderStableKey` 全字段样例表，而不改动 slide 画布里的 page16 图形结构。

## Decision

- 只修改左侧说明面板，不修改 `page_16` 的 Remotion 画布节点、边、转场或时长锚点。
- 在左侧说明面板新增一个仅对 `page_16` 生效的键值表 section。
- 表格数据使用 `Supplement/Shk解析/ShaderStableInfo-PSO-GLSL_ES3_1_ANDROID.shk.csv` 中 `Material /Game/MyActor/MyMaterial.MyMaterial` 的真实样例行。
- 表中保留 CSV 的全部 13 个字段，确保“这是哪个 material 的 StableKey”与底层键空间信息同时可见。

## Why

- 用户明确指出想补的是左侧“写东西的卡片”，不是 slide 图中的任何卡片。
- `Asset` 路径是回答“这是谁的 StableKey”的核心字段，应在左侧说明区完整展示。
- 左侧说明面板比画布右下角更适合承载高密度字段信息，也不会破坏 page16 当前的主链视觉结构。

## Scope

- 允许修改：`SlideApp/src/components/NotesPanel.tsx`、`SlideApp/src/storyboard-data/pso-storyboard.ts`、相关测试。
- 不允许修改：`SlideApp/src/remotion/pages/Page10Scene.tsx` 中 page16 的图形布局与动画时长。
