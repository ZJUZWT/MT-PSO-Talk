# 总结

## Nubia Z60 Ultra

### OpenGL（CPU 总计：795.2ms）

| Name | Count | Incl | I.Max | I.Avg (Incl/Count) |
|------|-------|--------|---------|------------------|
| glLinkProgram | 42 | 318 ms | 30.58 ms | 7.57 ms |
| glCompileShader | 64 | 232.4 ms | 19.80 ms | 3.63 ms |
| glProgramBinary | 339 | 144.6 ms | 2.06 ms | 426 μs |
| BindProgramPipeline | 37,272 | 99.7 ms | 1.29 ms | 2.68 μs |
| ConfigureStageStates | 42 | 492 μs | 60 μs | 11.71 μs |

### Vulkan（CPU 总计：~1.4s）

| Name | Count | Incl | I.Max | I.Avg (Incl/Count) |
|------|-------|--------|---------|------------------|
| CreateGfxPipeline(Standard) | 90 | ~1.3 s | 59.58 ms | 14.44 ms |
| CreateComputePipeline | 11 | 148.3 ms | 25.70 ms | 13.48 ms |
| BindGfxPipeline | 23,410 | 26 ms | 470 μs | 1.11 μs |
| BindComputePipeline | 10,938 | 5.9 ms | 80 μs | 0.54 μs |
| CreateShaderModule | 83 | 3.1 ms | 1.13 ms | 37.4 μs |