# SharedShaderCode Material 哈希位置分析

## 资源文件列表

| 文件 | 大小 | 类型 |
|------|-----|------|
| `Material/MyMaterial.uasset` | - | Material |
| `Material/MyMaterial.uexp` | 18875 字节 | Material |
| `Material/MyMaterial_Inst.uasset` | - | Material Instance |
| `Material/MyMaterial_Inst.uexp` | 20257 字节 | Material Instance (修改 StaticBool) |
| `Material/MyMaterial_SameInst.uasset` | - | Material Instance |
| `Material/MyMaterial_SameInst.uexp` | 20257 字节 | Material Instance (相同参数) |

## ShaderMapHash 位置汇总

### 1. MyMaterial (原始 Material)

| 平台 | ShaderMapHash | MyMaterial.uexp 偏移 (十进制) | MyMaterial.uexp 偏移 (十六进制) |
|------|--------------|----------------------------|------------------------------|
| OPENGL_ES3_1_ANDROID | `121386FC10142E0F3123F5789CF2FAE908D71165` | 18847 | 0x499F |
| VULKAN_ES3_1_ANDROID | `A60F13A2AEF96C61487BA9DF0C324EB04CB9A560` | 10620 | 0x297C |

### 2. MyMaterial_Inst (修改 StaticBool)

| 平台 | ShaderMapHash | MyMaterial_Inst.uexp 偏移 (十进制) | MyMaterial_Inst.uexp 偏移 (十六进制) |
|------|--------------|-----------------------------------|-------------------------------------|
| OPENGL_ES3_1_ANDROID | `BC10CB48479878A1BC7BC13FD095BEA0B4A6DB57` | 20233 | 0x4F09 |
| VULKAN_ES3_1_ANDROID | `8DD283A74B34030C87D255AA0911DDD7E60A34B5` | 11246 | 0x2BEE |

### 3. MyMaterial_SameInst (相同参数)

| 平台 | ShaderMapHash | MyMaterial_SameInst.uexp 偏移 (十进制) | MyMaterial_SameInst.uexp 偏移 (十六进制) |
|------|--------------|---------------------------------------|-----------------------------------------|
| OPENGL_ES3_1_ANDROID | `BC10CB48479878A1BC7BC13FD095BEA0B4A6DB57` | 20233 | 0x4F09 |
| VULKAN_ES3_1_ANDROID | `8DD283A74B34030C87D255AA0911DDD7E60A34B5` | 11246 | 0x2BEE |

## 验证结果

| 文件 | 找到 Hash |
|------|----------|
| MyMaterial.uasset | ❌ 未找到 |
| MyMaterial.uexp | ✅ 找到 (2 个) |
| MyMaterial_Inst.uasset | ❌ 未找到 |
| MyMaterial_Inst.uexp | ✅ 找到 (2 个) |
| MyMaterial_SameInst.uasset | ❌ 未找到 |
| MyMaterial_SameInst.uexp | ✅ 找到 (2 个) |

## JSON 引用关系

### ShaderAssetInfo-PSO-GLSL_ES3_1_ANDROID-OPENGL_ES3_1_ANDROID.assetinfo.json

```json
{
  "ShaderMapHash": "121386FC10142E0F3123F5789CF2FAE908D71165",
  "Assets": ["/Game/MyActor/MyMaterial"]
},
{
  "ShaderMapHash": "BC10CB48479878A1BC7BC13FD095BEA0B4A6DB57",
  "Assets": [
    "/Game/MyActor/MyMaterial_Inst",
    "/Game/MyActor/MyMaterial_SameInst"
  ]
}
```

### ShaderAssetInfo-PSO-SF_VULKAN_ES31_ANDROID-VULKAN_ES3_1_ANDROID.assetinfo.json

```json
{
  "ShaderMapHash": "A60F13A2AEF96C61487BA9DF0C324EB04CB9A560",
  "Assets": ["/Game/MyActor/MyMaterial"]
},
{
  "ShaderMapHash": "8DD283A74B34030C87D255AA0911DDD7E60A34B5",
  "Assets": [
    "/Game/MyActor/MyMaterial_Inst",
    "/Game/MyActor/MyMaterial_SameInst"
  ]
}
```

## 关键观察

1. **MyMaterial** 使用独立的 Hash (`121386FC...` / `A60F13A2...`)
2. **MyMaterial_Inst** 和 **MyMaterial_SameInst** 共享相同的 Hash (`BC10CB48...` / `8DD283A7...`)
3. Instance 的 Hash 与 Parent Material 不同（StaticBool 参数导致不同的 Shader 编译结果）
4. 两个参数相同的 Instance 共享同一套 Shader（节省磁盘空间）
5. 所有 Hash 都存储在 `.uexp` 文件中，而非 `.uasset`
