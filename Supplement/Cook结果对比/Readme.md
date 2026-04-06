# UE5 Android Shader 打包方式对比

本目录包含两种 UE5 Android 平台 Shader 打包方式的 Cook 结果对比。

## 一、开关配置

### SharedShaderCode 启用方式
在 `DefaultEngine.ini` 中设置：
```ini
[/Script/Engine.RendererSettings]
bSharedShaderCode=True
```

### InlineShaderCode 启用方式
在 `DefaultEngine.ini` 中设置：
```ini
[/Script/Engine.RendererSettings]
bSharedShaderCode=False
```

---

## 二、单文件体积对比

### Material 文件

| 打包方式 | MyMaterial.uexp 大小 | 差异 |
|----------|---------------------|------|
| InlineShaderCode | 171,637 字节 | - |
| SharedShaderCode | 18,875 字节 | 减少 152,762 字节 (89%) |

### Material Instance 文件

| 打包方式 | MyMaterial_Inst.uexp 大小 | 差异 |
|----------|--------------------------|------|
| InlineShaderCode | 182,350 字节 | - |
| SharedShaderCode | 20,257 字节 | 减少 162,093 字节 (89%) |

---

## 三、SharedShaderCode Reference

### 文件结构
```
SharedShaderCode/
├── Material/
│   ├── MyMaterial.uasset              # Material 元数据
│   ├── MyMaterial.uexp                # Material 导出数据
│   ├── MyMaterial_Inst.uasset         # Material Instance 元数据
│   ├── MyMaterial_Inst.uexp           # Material Instance 导出数据
│   ├── MyMaterial_SameInst.uasset     # Material Instance 元数据
│   └── MyMaterial_SameInst.uexp       # Material Instance 导出数据
└── ShaderCode/
    ├── ShaderArchive-PSO-GLSL_ES3_1_ANDROID-OPENGL_ES3_1_ANDROID.ushaderbytecode
    ├── ShaderArchive-PSO-SF_VULKAN_ES31_ANDROID-VULKAN_ES3_1_ANDROID.ushaderbytecode
    ├── ShaderAssetInfo-PSO-GLSL_ES3_1_ANDROID-OPENGL_ES3_1_ANDROID.assetinfo.json
    └── ShaderAssetInfo-PSO-SF_VULKAN_ES31_ANDROID-VULKAN_ES3_1_ANDROID.assetinfo.json
```

### ShaderMapHash 位置汇总

#### MyMaterial (Parent Material)

| 平台 | ShaderMapHash | 偏移 (字节) |
|------|--------------|------------|
| OPENGL_ES3_1_ANDROID | `121386FC10142E0F3123F5789CF2FAE908D71165` | 18847 |
| VULKAN_ES3_1_ANDROID | `A60F13A2AEF96C61487BA9DF0C324EB04CB9A560` | 10620 |

#### MyMaterial_Inst (StaticBool=True)

| 平台 | ShaderMapHash | 偏移 (字节) |
|------|--------------|------------|
| OPENGL_ES3_1_ANDROID | `BC10CB48479878A1BC7BC13FD095BEA0B4A6DB57` | 20233 |
| VULKAN_ES3_1_ANDROID | `8DD283A74B34030C87D255AA0911DDD7E60A34B5` | 11246 |

#### MyMaterial_SameInst (StaticBool=True, 相同参数)

| 平台 | ShaderMapHash | 偏移 (字节) |
|------|--------------|------------|
| OPENGL_ES3_1_ANDROID | `BC10CB48479878A1BC7BC13FD095BEA0B4A6DB57` | 20233 |
| VULKAN_ES3_1_ANDROID | `8DD283A74B34030C87D255AA0911DDD7E60A34B5` | 11246 |

### JSON 引用关系

**ShaderAssetInfo-PSO-GLSL_ES3_1_ANDROID-OPENGL_ES3_1_ANDROID.assetinfo.json**
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

**ShaderAssetInfo-PSO-SF_VULKAN_ES31_ANDROID-VULKAN_ES3_1_ANDROID.assetinfo.json**
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
