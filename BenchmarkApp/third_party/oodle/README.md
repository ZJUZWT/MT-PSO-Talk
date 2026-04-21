# Oodle SDK Placeholder

This repository does not include the proprietary Oodle SDK binaries or headers.

`BenchmarkApp` can use Oodle `2.9.12`, but each developer must supply their own
licensed SDK copy locally. The committed files in this directory are only
placeholders so the expected layout is visible in git.

## How To Enable Oodle

Use one of these approaches:

1. Copy a licensed Unreal Engine `OodleDataCompression` SDK `2.9.12` into:

```text
BenchmarkApp/third_party/oodle/2.9.12
```

2. Or keep the SDK elsewhere and pass a CMake override:

```bash
cmake -S BenchmarkApp -B build/benchmark \
  -DCMAKE_BUILD_TYPE=Release \
  -DBENCHMARK_OODLE_ROOT=/absolute/path/to/OodleDataCompression/Sdks/2.9.12
```

3. Or export an environment variable before running the platform build scripts:

```bash
export BENCHMARK_OODLE_ROOT=/absolute/path/to/OodleDataCompression/Sdks/2.9.12
```

## Expected Layout

```text
BenchmarkApp/third_party/oodle/2.9.12/
├── include/
│   ├── oodle2.h
│   └── oodle2base.h
└── lib/
    ├── Android/<abi>/liboo2coreandroid.a
    ├── IOS/liboo2coreios.a
    ├── Mac/liboo2coremac64.a
    └── Win64/oo2core_win64.lib
```

If the SDK is absent, `BenchmarkApp` still builds and runs, but the Oodle
algorithms are omitted from the benchmark.
