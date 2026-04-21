# Tree Geometry Harness Design

## Goal

把 slide geometry harness 从“平面 node 审计”升级成“显式树结构审计”，保证每一页都满足两条基础约束：

- 同一层级的兄弟实体不能打架
- 每个孩子都必须留在父容器内部

同时保留现有 harness 的优点：

- 先做数学约束判断
- 可接前台原生 Microsoft Edge 的真实测量结果
- 继续复用 `geometryMetrics -> geometryScorePolicy -> geometryReviewArtifact` 这条 review 链

## Problem Statement

当前 harness 有两个根本盲点：

1. `overlapCount` 只检查 `sketch.nodes` 之间的平面重叠
2. `textOverflowCount` 只检查“节点自己的文字是否从自己体内溢出”

这会漏掉像 `page_31` 那种情况：

- 一段自由文本真实渲染后压住了同层卡片
- 但因为它不是一等 node，所以当前算法报告 `overlapCount = 0`

用户提出的“每一页是一个树结构”正好能把这个问题从算法层面修正掉，而不是再做页面特判。

## Chosen Approach

采用“显式树模型 + 向后兼容平面接口”的方案。

核心原则：

- 页面上的所有可见元素都必须入树
- `containerId` 升级成显式 `parentId`
- 自由文本、edge label、caption、图片说明都不再是审计盲区
- 浏览器真实测量优先于公式估计
- 旧的 review/score/audit 链路尽量保留，只在 metrics 层注入树约束

不采用的方案：

- 只在平面数组上继续打补丁：短期快，长期会继续漏
- 完全以 DOM 树作为唯一真相：后验太重，contract 可解释性下降

## Data Model

建议新增统一的页面实体模型：

```ts
type GeometryEntityKind =
  | "container"
  | "card"
  | "text"
  | "edgeLabel"
  | "image"
  | "caption"
  | "junction"
  | "decorative";

type GeometryEntity = {
  id: string;
  kind: GeometryEntityKind;
  label?: string;
  parentId?: string;
  x: number;
  y: number;
  width: number;
  height: number;
  renderStyle?: "default" | "outline" | "textOnly";
  participatesInSiblingCollision?: boolean;
  participatesInParentContainment?: boolean;
  measurementMode?: "formula" | "browser" | "hybrid";
};
```

`GeometrySketchDefinition` 增量扩展为：

```ts
type GeometrySketchDefinition = {
  id: string;
  label: string;
  stepId: StoryStepId;
  contract: SketchContract;
  entities?: GeometryEntity[];
  nodes: SketchNode[];
  edges: SketchEdge[];
};
```

兼容策略：

- 新页面优先写 `entities`
- 老页面保留 `nodes`
- review 入口统一调用 `resolveGeometryEntities(sketch)` 做兼容映射

## Mathematical Constraints

对任意实体 `e`，定义真实边界盒：

```text
B(e) = [l(e), r(e), t(e), b(e)]
```

边界来源：

```text
B_truth(e) = B_browser(e) if browser probe exists
B_truth(e) = B_formula(e) otherwise
```

### Sibling Collision

若 `parentId(a) = parentId(b)`，且两者都参与兄弟碰撞审计，则：

```text
overlapW(a,b) = max(0, min(r(a), r(b)) - max(l(a), l(b)))
overlapH(a,b) = max(0, min(b(a), b(b)) - max(t(a), t(b)))
overlapArea(a,b) = overlapW(a,b) * overlapH(a,b)
```

判定：

```text
siblingCollision(a,b) = overlapArea(a,b) > epsilon_area
```

建议：

```text
epsilon_area = 4 px²
```

### Parent Containment

对任意非根孩子 `c` 及其父节点 `p`：

```text
overflowLeft(c,p)   = max(0, l(p) - l(c))
overflowRight(c,p)  = max(0, r(c) - r(p))
overflowTop(c,p)    = max(0, t(p) - t(c))
overflowBottom(c,p) = max(0, b(c) - b(p))

containmentOverflow(c,p) =
  overflowLeft + overflowRight + overflowTop + overflowBottom
```

判定：

```text
childOutOfBounds(c) = containmentOverflow(c,p) > epsilon_len
```

建议：

```text
epsilon_len = 1.5 px
```

### Internal Safety Margin

对任意父子对再定义：

```text
padLeft(c,p)   = l(c) - l(p)
padRight(c,p)  = r(p) - r(c)
padTop(c,p)    = t(c) - t(p)
padBottom(c,p) = b(p) - b(c)

tightestContainmentPad(c,p) = min(padLeft, padRight, padTop, padBottom)
```

这个值用于发现“虽然没出框，但已经贴边危险”的页面。

## Tree Metrics

在现有 `GeometryMetrics` 上新增以下字段：

```ts
type GeometryMetrics = {
  ...
  siblingOverlapCount: number;
  maxSiblingOverlapArea: number;
  minSiblingGap: number;
  childOutOfBoundsCount: number;
  maxChildOverflowPx: number;
  minContainmentPad: number;
  freeTextCollisionCount: number;
  edgeLabelCollisionCount: number;
};
```

指标含义：

- `siblingOverlapCount`: 非法同层兄弟碰撞数
- `maxSiblingOverlapArea`: 最大兄弟重叠面积
- `minSiblingGap`: 非碰撞兄弟中的最小净距
- `childOutOfBoundsCount`: 超出父容器的孩子数
- `maxChildOverflowPx`: 最大越界量
- `minContainmentPad`: 最小父内安全余量
- `freeTextCollisionCount`: 自由文本造成的兄弟碰撞数
- `edgeLabelCollisionCount`: edge label 造成的兄弟碰撞数

兼容语义：

- 旧的 `overlapCount` 改为承载 `siblingOverlapCount`
- 父子包围关系不再算 overlap，而算 containment

## Browser Truth Integration

当前浏览器探针只采“节点内文字”。设计上要升级成“实体测量探针”。

建议新增：

```ts
type BrowserGeometryEntityProbe = {
  sketchId?: string;
  stepId?: string;
  sourceUrl?: string;
  entities: Array<{
    entityId: string;
    kind: GeometryEntityKind;
    bounds: {x: number; y: number; width: number; height: number};
    textBounds?: {x: number; y: number; width: number; height: number};
  }>;
};
```

真实测量优先级：

```text
resolveEntityBounds(entity) =
  browserProbe.bounds(entity.id) ?? formulaBounds(entity)
```

这就把回环严格定义为：

```text
前台 Edge 真实结果
-> entity bounds
-> tree metrics
-> score/policy
-> 下一轮布局修改
```

## Harness Integration

### `geometry-sketch-types.ts`

- 新增 `GeometryEntity`
- 扩展 `GeometrySketchDefinition.entities`
- 保留旧 `SketchNode` 兼容路径

### `browserGeometryTextProbe.ts`

- 从“节点文字探针”升级成“实体真值探针”
- 支持容器、卡片、自由文本、edge label、caption 的真实 bbox

### `geometryMetrics.ts`

新增三层逻辑：

1. `resolveGeometryEntities(sketch)`
2. `resolveTruthBounds(entities, browserProbe)`
3. `collectTreeGeometryMetrics(...)`

### `geometryScorePolicy.ts`

`blockerOpen` 条件升级为：

```text
siblingOverlapCount > 0
|| childOutOfBoundsCount > 0
|| textOverflowCount > 0
|| crossingCount > 0
|| nodePierceCount > 0
```

### `geometryReviewArtifact.ts`

输出必须同时覆盖三层：

- 整体布局 Review
- 模块空间结构 Review
- 单节点与单边 Review

模块层负责回答“哪个父容器内部出了问题”，节点层负责回答“到底是谁撞了谁”。

## Why This Fixes Page 31

`page_31` 那句游离文本一旦变成正式 `text` 实体，它就会拥有：

- 自己的 `entityId`
- 自己的 `parentId`
- 自己来自 Edge 的真实 bbox

于是它会和同父的 `feedback-node`、`policy-node`、`metrics-node` 自动组成兄弟对，并套用同一套兄弟碰撞公式。

因此：

- `siblingOverlapCount` 会被正确加 3
- `freeTextCollisionCount` 会被正确加 3
- `blockerOpen` 会被拉起

这不是页面特判，而是树语义自然推出的结果。

## Validation Strategy

测试分四层：

1. 类型与兼容映射测试
2. 纯数学单测
3. 浏览器探针单测
4. score/artifact/formal review 集成回归

特别要覆盖：

- free text vs sibling card
- edge label vs sibling entity
- child 跑出 parent
- browser bbox 覆盖 formula bbox
- 真实 `page_31` case 不再报告 `Overlaps: 0`

## Rollout Strategy

按五步迁移：

1. 加类型和兼容层
2. 加 tree metrics
3. 升级 browser probe
4. 接 score / artifact / formal review
5. 逐页补 contract

风险控制：

- 先加 feature flag，支持新旧 harness 并行
- 先输出 dual report，比较 legacy flat metrics 与 tree metrics

## Success Criteria

达到以下条件时，认为这次 redesign 成功：

- 所有可见元素都能入树或被兼容映射入树
- 浏览器真实 bbox 能进入树 metrics
- 同层兄弟碰撞与父子越界都能稳定命中 blocker
- `page_31` 这类自由文本压住兄弟节点的情况不再漏报
- 现有 formal review、storyboard audit、transition audit 仍可通过
