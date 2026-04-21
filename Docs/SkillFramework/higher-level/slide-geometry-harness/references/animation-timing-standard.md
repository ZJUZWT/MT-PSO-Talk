# Animation Timing Standard (Harness v1.0)

本标准用于把转场时长从“主观感觉”改成“可计算、可复核、可复用”的统一约束。

本文件是 timing 专题说明；完整 harness 公式总表见：

- `references/harness-formula-registry.md`

适用范围：`SlideApp` 所有页面过渡（`from-step -> to-step`），尤其是包含 node 平移、edge 延伸、淡入淡出的转场。

---

## 1) 动作类型与公式

统一公式：

`duration_ms = clamp(base_ms + metric * per_unit_ms, min_ms, max_ms)`

其中 `metric` 对应动作几何量（如位移像素、线段长度）。

| 动作类型 | metric | 公式参数 | 最终公式 |
| --- | --- | --- | --- |
| `node_move` | `distancePx` | `base=180`, `per_unit=0.95`, `min=180`, `max=960` | `clamp(180 + distancePx*0.95, 180, 960)` |
| `edge_grow` | `lengthPx` | `base=140`, `per_unit=0.75`, `min=140`, `max=840` | `clamp(140 + lengthPx*0.75, 140, 840)` |
| `fade_in` | 无 | 固定基准 `220`，上下限 `180~420` | `220ms`（再按 `180~420` 夹紧） |
| `fade_out` | 无 | 固定基准 `220`，上下限 `180~420` | `220ms`（再按 `180~420` 夹紧） |

备注：

- 如动作给了 `durationMs`，允许手动覆盖，但仍会被类型上下限夹紧。
- 如 `node_move` / `edge_grow` 缺少 metric，回退到 base 时长并在报告里给 warning。

---

## 2) 统一时间轴顺序

默认 phase 顺序：

`hold_before -> fade_out -> node_motion -> edge_motion -> fade_in -> hold_after`

默认 hold：

- `hold_before = 120ms`
- `hold_after = 180ms`

相位时长规则：

- 每个 phase 的时长 = 该 phase 内动作时长的最大值（hold 相位取 `max(hold, actions)`）。
- phase 之间串行推进，动作在所属 phase 内并行起跑。

---

## 3) 时长 verdict 规则

对某个过渡，脚本同时产出两套判定：

1. 传统区间判定（`--profile`）  
2. 标准时序判定（本标准）

标准判定依据：

- `requiredSec = 标准时间轴总时长`
- `allowedMinSec = requiredSec`
- `allowedMaxSec = requiredSec * 1.35`

判定：

- 当前时长 `< allowedMinSec` => `too_short`
- 当前时长 `> allowedMaxSec` => `too_long`
- 否则 `in_range`

如果是 `too_short/too_long`，需要通过调整 `SlideApp/src/remotion/embed.ts` 的 step frame 锚点修正。

---

## 4) Workload JSON 约定

最小结构：

```json
{
  "holdBeforeMs": 120,
  "holdAfterMs": 180,
  "phaseOrder": [
    "hold_before",
    "fade_out",
    "node_motion",
    "edge_motion",
    "fade_in",
    "hold_after"
  ],
  "actions": [
    { "id": "n_material", "kind": "node_move", "distancePx": 280 },
    { "id": "e_main", "kind": "edge_grow", "lengthPx": 420 },
    { "id": "n_new_label", "kind": "fade_in" }
  ]
}
```

字段说明：

- `actions[].kind`：`node_move | edge_grow | fade_in | fade_out`（支持常见别名）。
- `actions[].phase`：可选；不填则按 kind 默认路由到标准 phase。
- `actions[].durationMs`：可选手动覆盖。
- 可直接复制模板：`references/workload-template.json`

---

## 5) 命令示例

```bash
python3 scripts/slide-geometry-harness/probe_transition_timeline.py \
  --from-step page_14 \
  --to-step page_15 \
  --workload-json Docs/剧本/workloads/page14_to_page15.json \
  --emit-markdown
```

输出会包含：

- `timingStandardPlan`（动作、相位、公式、推荐时长）
- `timingStandardEval`（too_short / in_range / too_long）
- 带 phase+action 表格的 markdown 片段（可直接贴入剧本）

---

## 6) 与既有规则的关系

- 插入前置/中段动画时，必须保留原始基础动画时长（不允许“挤压塞回原总时长”）。
- 本标准负责“动作时长与顺序”一致性；几何美感和覆盖/交叉仍由 geometry review 体系判定。
