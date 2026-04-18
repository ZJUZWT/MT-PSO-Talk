# Apple Device Metadata Implementation

**Goal**

把 Apple 平台 benchmark 结果里的设备元数据从占位值升级成真实可用的型号 / SoC 信息，并保证导出文件格式稳定。

**Implemented**

- 新增 Apple 机型映射：
  - `BenchmarkApp/lib/driver/common/apple_device_lookup.h`
  - `BenchmarkApp/lib/driver/common/apple_device_lookup.cpp`
- 重写 Apple 设备探测：
  - macOS 现在写出 `MacBookPro18,1` 这类真实 model identifier
  - macOS 现在写出 `Apple M1 Pro` 这类真实 CPU/SoC 名称
  - iOS 现在写出 `iPad mini (6th generation) [iPad14,1]` 这类设备标识
  - iOS 已知机型会写出 `Apple A15 Bionic` / `Apple M2` / `Apple M4` 等 SoC
- runner 元数据增强：
  - iOS `run_info.txt` 新增 `Marketing Name` / `Product Type` / `Hardware Model`
  - macOS `run_info.txt` 新增 `Model Identifier` / `SoC`
- CSV escaping 修复：
  - `MacBookPro18,1` 这类带逗号的值现在会被正确写成 `"MacBookPro18,1"`
- benchmark test 修正：
  - `compression_benchmark_test` 允许极快路径的 `decompress_us == 0`

**Verification**

- 通过完整测试：
  - `ctest --test-dir /tmp/mt-pso-apple-metadata --output-on-failure`
  - 结果：`27/27` 通过
- 通过真实 macOS benchmark：
  - 结果目录：
    - `/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/benchmark_results/macos/20260418T160125Z`
  - 验证点：
    - `compression_results.csv` 中 `device_model` 已变为 `"MacBookPro18,1"`
    - `soc` 已变为 `Apple M1 Pro`
    - `os_version` 已变为 `macOS 14.6.1`
    - `run_info.txt` 已包含 `Model Identifier` 和 `SoC`

**Current iOS Status**

- iOS device 包重新构建成功，且签名、安装都成功。
- 真机自动拉起在 `devicectl device process launch` 阶段被系统拒绝，错误原因为：
  - `profile has not been explicitly trusted by the user`
- 这意味着代码和包都已生成，但当前这台 iPad 还需要在系统里再次明确允许该开发者后，脚本才能自动完成新一轮真机结果采集。
