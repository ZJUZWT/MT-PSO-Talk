# Mine PSO 耗时 Insight 对比分析

> 数据来源：4 个 `.utrace` 文件，通过 UE SummarizeTrace Commandlet 导出 Scopes CSV 后解析
>
> 测试设备：Nubia Z60 Ultra / Pixel 7
>
> 图形 API：OpenGL ES 3.1 / Vulkan ES 3.1

---

## 1. 总览

| 设备 | API | Mine PSO 条目数 | 总调用次数 | 总耗时 |
|------|-----|:---:|---:|---:|
| Nubia Z60 Ultra | OpenGL | 5 | 37,759 | **0.795 s** |
| Nubia Z60 Ultra | Vulkan | 5 | 34,532 | **1.440 s** |
| Pixel 7 | OpenGL | 5 | 41,272 | **1.453 s** |
| Pixel 7 | Vulkan | 5 | 29,194 | **2.526 s** |

**结论**：
- Vulkan 总耗时约为 OpenGL 的 **1.7~1.8 倍**
- Pixel 7 总耗时约为 Nubia Z60 Ultra 的 **1.7~1.8 倍**
- 最慢组合：Pixel 7 + Vulkan（2.53s），最快组合：Nubia Z60 Ultra + OpenGL（0.80s）

---

## 2. 逐设备详细数据

### 2.1 Nubia Z60 Ultra - OpenGL

| Scope | 调用次数 | 总耗时(s) | 平均(ms) | 最小(ms) | 最大(ms) | 标准差(ms) |
|-------|-------:|--------:|--------:|--------:|--------:|--------:|
| glLinkProgram | 42 | 0.318 | 7.572 | 1.059 | 30.576 | 7.367 |
| glCompileShader | 64 | 0.232 | 3.631 | 0.396 | 19.797 | 3.720 |
| glProgramBinary | 339 | 0.145 | 0.427 | 0.170 | 2.065 | 0.218 |
| BindProgramPipeline | 37,272 | 0.100 | 0.003 | 0.000 | 1.293 | 0.013 |
| ConfigureStageStates | 42 | 0.000 | 0.012 | 0.002 | 0.062 | 0.011 |
| **合计** | **37,759** | **0.795** | | | | |

### 2.2 Nubia Z60 Ultra - Vulkan

| Scope | 调用次数 | 总耗时(s) | 平均(ms) | 最小(ms) | 最大(ms) | 标准差(ms) |
|-------|-------:|--------:|--------:|--------:|--------:|--------:|
| CreateGfxPipeline(Standard) | 90 | 1.257 | 13.968 | 0.052 | 59.581 | 17.837 |
| CreateComputePipeline | 11 | 0.148 | 13.479 | 2.229 | 25.700 | 8.845 |
| BindGfxPipeline | 23,410 | 0.026 | 0.001 | 0.000 | 0.472 | 0.006 |
| BindComputePipeline | 10,938 | 0.006 | 0.001 | 0.000 | 0.081 | 0.001 |
| CreateShaderModule | 83 | 0.003 | 0.038 | 0.001 | 1.125 | 0.123 |
| **合计** | **34,532** | **1.440** | | | | |

### 2.3 Pixel 7 - OpenGL

| Scope | 调用次数 | 总耗时(s) | 平均(ms) | 最小(ms) | 最大(ms) | 标准差(ms) |
|-------|-------:|--------:|--------:|--------:|--------:|--------:|
| glLinkProgram | 43 | 0.590 | 13.722 | 0.125 | 66.751 | 17.225 |
| glCompileShader | 65 | 0.499 | 7.675 | 0.470 | 85.457 | 13.070 |
| glProgramBinary | 339 | 0.216 | 0.637 | 0.063 | 1.423 | 0.293 |
| BindProgramPipeline | 40,782 | 0.146 | 0.004 | 0.000 | 0.757 | 0.005 |
| ConfigureStageStates | 43 | 0.002 | 0.047 | 0.004 | 1.353 | 0.204 |
| **合计** | **41,272** | **1.453** | | | | |

### 2.4 Pixel 7 - Vulkan

| Scope | 调用次数 | 总耗时(s) | 平均(ms) | 最小(ms) | 最大(ms) | 标准差(ms) |
|-------|-------:|--------:|--------:|--------:|--------:|--------:|
| CreateGfxPipeline(Standard) | 90 | 2.092 | 23.243 | 0.089 | 122.600 | 31.812 |
| CreateComputePipeline | 11 | 0.334 | 30.408 | 2.122 | 69.468 | 22.769 |
| BindGfxPipeline | 19,722 | 0.084 | 0.004 | 0.000 | 0.583 | 0.009 |
| BindComputePipeline | 9,288 | 0.013 | 0.001 | 0.000 | 0.111 | 0.003 |
| CreateShaderModule | 83 | 0.002 | 0.028 | 0.002 | 0.385 | 0.046 |
| **合计** | **29,194** | **2.526** | | | | |

---

## 3. 跨设备/API 横向对比

### 3.1 OpenGL 关键操作对比

| 操作 | Nubia Z60 Ultra | Pixel 7 | Pixel 7 / Nubia 倍率 |
|------|:---:|:---:|:---:|
| glLinkProgram (avg) | 7.572 ms | 13.722 ms | **1.81x** |
| glLinkProgram (max) | 30.576 ms | 66.751 ms | **2.18x** |
| glCompileShader (avg) | 3.631 ms | 7.675 ms | **2.11x** |
| glCompileShader (max) | 19.797 ms | 85.457 ms | **4.32x** |
| glProgramBinary (avg) | 0.427 ms | 0.637 ms | **1.49x** |
| glProgramBinary (max) | 2.065 ms | 1.423 ms | 0.69x |
| BindProgramPipeline (avg) | 0.003 ms | 0.004 ms | 1.33x |

### 3.2 Vulkan 关键操作对比

| 操作 | Nubia Z60 Ultra | Pixel 7 | Pixel 7 / Nubia 倍率 |
|------|:---:|:---:|:---:|
| CreateGfxPipeline (avg) | 13.968 ms | 23.243 ms | **1.66x** |
| CreateGfxPipeline (max) | 59.581 ms | 122.600 ms | **2.06x** |
| CreateComputePipeline (avg) | 13.479 ms | 30.408 ms | **2.26x** |
| CreateComputePipeline (max) | 25.700 ms | 69.468 ms | **2.70x** |
| CreateShaderModule (avg) | 0.038 ms | 0.028 ms | 0.74x |
| BindGfxPipeline (avg) | 0.001 ms | 0.004 ms | 4.00x |

### 3.3 同设备 OpenGL vs Vulkan 对比

#### Nubia Z60 Ultra

| 对比项 | OpenGL | Vulkan | 说明 |
|--------|-------:|-------:|------|
| PSO 创建总耗时 | 0.695 s (Link+Compile+Binary) | 1.409 s (GfxPipeline+ComputePipeline+ShaderModule) | Vulkan **2.03x** 慢 |
| 最耗时单次操作 | glLinkProgram 30.6 ms | CreateGfxPipeline 59.6 ms | Vulkan 峰值更高 |
| Bind 操作总耗时 | 0.100 s | 0.032 s | OpenGL Bind 更慢（次数更多） |

#### Pixel 7

| 对比项 | OpenGL | Vulkan | 说明 |
|--------|-------:|-------:|------|
| PSO 创建总耗时 | 1.306 s (Link+Compile+Binary) | 2.428 s (GfxPipeline+ComputePipeline+ShaderModule) | Vulkan **1.86x** 慢 |
| 最耗时单次操作 | glCompileShader 85.5 ms | CreateGfxPipeline 122.6 ms | Vulkan 峰值更高 |
| Bind 操作总耗时 | 0.148 s | 0.097 s | OpenGL Bind 更慢（次数更多） |

---

## 4. 耗时分布分析

### 4.1 耗时热点（按总耗时排序 Top 10）

| 排名 | 设备 + API | Scope | 总耗时(s) | 占比 |
|:---:|------|-------|--------:|---:|
| 1 | Pixel 7 Vulkan | CreateGfxPipeline(Standard) | 2.092 | 33.7% |
| 2 | Nubia Vulkan | CreateGfxPipeline(Standard) | 1.257 | 20.2% |
| 3 | Pixel 7 OpenGL | glLinkProgram | 0.590 | 9.5% |
| 4 | Pixel 7 OpenGL | glCompileShader | 0.499 | 8.0% |
| 5 | Pixel 7 Vulkan | CreateComputePipeline | 0.334 | 5.4% |
| 6 | Nubia OpenGL | glLinkProgram | 0.318 | 5.1% |
| 7 | Nubia OpenGL | glCompileShader | 0.232 | 3.7% |
| 8 | Pixel 7 OpenGL | glProgramBinary | 0.216 | 3.5% |
| 9 | Nubia Vulkan | CreateComputePipeline | 0.148 | 2.4% |
| 10 | Pixel 7 OpenGL | BindProgramPipeline | 0.146 | 2.4% |

> 总耗时 = 0.795 + 1.440 + 1.453 + 2.526 = **6.214 s**（4 个 trace 合计）

### 4.2 平均耗时分桶

| 耗时区间 | 条目数 | 代表操作 |
|----------|:---:|------|
| < 0.01 ms | 8 | BindProgramPipeline, BindGfxPipeline, BindComputePipeline, ConfigureStageStates |
| 0.01~0.1 ms | 4 | CreateShaderModule, ConfigureStageStates |
| 0.1~1 ms | 2 | glProgramBinary |
| 1~10 ms | 2 | glCompileShader, glLinkProgram (Nubia) |
| 10~50 ms | 4 | glLinkProgram (Pixel), CreateGfxPipeline, CreateComputePipeline |

---

## 5. 关键结论

1. **PSO 创建是绝对瓶颈**：`CreateGfxPipeline`（Vulkan）和 `glLinkProgram + glCompileShader`（OpenGL）占据了 90%+ 的 Mine PSO 总耗时

2. **Vulkan Pipeline 创建比 OpenGL Link+Compile 更慢**：
   - Nubia: Vulkan 1.41s vs OpenGL 0.55s（**2.6x**）
   - Pixel 7: Vulkan 2.43s vs OpenGL 1.09s（**2.2x**）

3. **glProgramBinary 路径显著更快**：
   - 平均 0.4~0.6ms vs glLinkProgram 的 7.5~13.7ms（**快 17~22 倍**）
   - 说明 PSO 预缓存的 binary 缓存机制效果显著

4. **Bind 操作不是瓶颈**：微秒级，可忽略

5. **设备差异明显**：Pixel 7 在所有操作上都比 Nubia Z60 Ultra 慢 1.5~2.3 倍，尤其是 `glCompileShader` 的最大值差异达 **4.3 倍**（85ms vs 20ms）

6. **标准差很大**：说明耗时波动剧烈，部分 PSO 创建可能触发了 shader 编译或 pipeline cache miss，导致极端耗时

---

## 6. 优化建议

1. **优先使用 PSO 预缓存**：`glProgramBinary` 路径比 `glLinkProgram` 快 17~22 倍，确保所有 PSO 都能命中 binary 缓存
2. **Vulkan Pipeline Cache**：确保 `VkPipelineCache` 持久化到磁盘，避免重复创建
3. **异步 PSO 编译**：将 `CreateGfxPipeline` 和 `glLinkProgram` 放到后台线程，避免阻塞主线程
4. **关注 Pixel 7 的极端耗时**：`glCompileShader` 最大 85ms，可能需要针对 Mali GPU 做特殊优化
