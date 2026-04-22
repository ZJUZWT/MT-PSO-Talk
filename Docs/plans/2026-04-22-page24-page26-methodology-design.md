# Page24 Page26 Methodology Design

**Goal:** Reframe `page_24` and `page_26` from UE-specific implementation boards into two clearer methodology pages that explain when a class of optimization applies, why PSO satisfies those conditions, and which concrete UE/PSO techniques sit underneath.

**Approved Direction:** Use a unified method-language:

- `page_24` becomes `资源过重：减体积 / 虚拟化`
- `page_26` becomes `编译过慢：分集合 / 并行化`

Both pages should read like design-pattern cards instead of nested engineering diagrams.

## Method Contract

### Page24

- Left half expresses `减体积`
  - condition: `高体积、高重复、体积成本敏感`
  - familiar analogy: `纹理压缩`
  - PSO mapping: `ShaderCode 压缩、去重、共享化`
- Right half expresses `虚拟化`
  - condition: `冷热分化明显、物理资源受限、允许按需回填`
  - familiar analogy: `虚拟内存`
  - PSO mapping: `热 PSO 常驻、冷 PSO 外存、按需映射`

### Page26

- Left half expresses `分集合`
  - condition: `对象可聚类、需求具上下文、允许分组裁剪`
  - familiar analogy: `画质分档`
  - PSO mapping: `UsageMask、Per-Map Compile`
- Right half expresses `并行化`
  - condition: `任务可拆分、依赖稀疏、结果可汇总`
  - shown as hierarchy instead of metaphor: `SIMD / Thread / GPU`
  - PSO mapping: `并行预编译`

## Visual Direction

- Reduce nested node count aggressively.
- Replace secondary/tertiary capsules with:
  - corner tags
  - inline labels
  - arrow-adjacent text
  - short footer summaries
- Keep at most `2-3` dominant structural nodes per half.
- Preserve the existing late-tail shell and overall transition pacing.

## Acceptance Criteria

- `page_24` still visually reads as left/right dual strategy, but the right side no longer contains the old structure strip, lane shell, or bottom method capsules.
- `page_26` still shows `Game = A` vs `Compile = A + B`, but the old queue/worker/throughput topology is replaced by a lighter methodology card.
- Both pages can be introduced quickly from title + condition + one familiar analogy without relying on long prose.
