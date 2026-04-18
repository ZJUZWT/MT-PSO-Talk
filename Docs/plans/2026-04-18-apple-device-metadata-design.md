# Apple Device Metadata Design

**Problem**

Apple 平台的 benchmark 结果虽然是真跑出来的，但导出的 `device_model` / `soc` 一直是占位值：

- macOS: `Mac` / `Apple Silicon`
- iOS: `iOS Device` / `Apple Silicon`

这会直接削弱跨设备对比的可信度，也让结果归档缺少最关键的设备信息。

**Goal**

让 benchmark 在 Apple 平台导出的 JSON、CSV、`run_info.txt` 中都尽可能落下具体、稳定、可比对的设备标识：

- macOS 至少写出 `hw.model` 和真实 CPU/SoC 名称
- iOS 至少写出 `productType`，已知机型再补出营销名和 SoC
- 结果文件在遇到逗号型号标识时仍保持合法 CSV

**Approved Approach**

- macOS 直接通过 `sysctl` 查询真实信息：
  - `device_model <- hw.model`
  - `soc <- machdep.cpu.brand_string`
  - `os_version <- kern.osproductversion`
- iOS 通过 `hw.machine` / `uname.machine` 获取设备标识符。
- 对常见 iPad / iPhone 维护一份本地映射表，把 `productType` 映射到：
  - `marketing_name`
  - `soc`
- iOS 未命中映射时不再退回 `Apple Silicon`，而是诚实地保留设备标识并给出 `Unknown Apple SoC [...]`。
- 更新 iOS runner，把 `marketingName` / `productType` / `hardwareModel` 一起写进 `run_info.txt`。
- 修复 CSV 导出，对包含逗号、引号、换行的字符串字段做标准 CSV escaping。

**Verification**

- 新增 `device_info_test`，锁住：
  - macOS 不再返回 `Mac` / `Apple Silicon`
  - 已知 iOS 机型映射正确
- 新增 `ios_device_metadata_source_test.sh`，防止 iOS 分支回退成硬编码占位值。
- 扩展 `ios_device_utils_test.sh`，验证 runner 能提取营销名、产品型号和硬件型号。
- 扩展 `exporter_test.cpp`，验证真实 Apple 型号写入 CSV 时会正确加引号。
- 最终跑一次完整 `ctest`，再跑一次真实 macOS benchmark 落盘检查。
