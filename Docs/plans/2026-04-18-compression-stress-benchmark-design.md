# Compression Stress Benchmark Design

**Problem**

The current compression benchmark product path is too narrow to support confident cross-device comparisons. It defaults to a single 256KB PSO-like payload and a low iteration count, the console matrix only shows the first payload size, and roundtrip validation relies on size plus hash instead of an explicit byte-for-byte equality check.

**Goal**

Make compression benchmarking look and behave like a real stress test across Android, iOS, macOS, and Windows by broadening the payload mix, increasing iteration depth, surfacing the benchmark shape in the output, and recording stronger correctness checks.

**Approved Approach**

- Run multiple payload profiles by default:
  - `pso_like`
  - `high_compressibility`
  - `low_compressibility`
- Run multiple payload sizes by default:
  - `64KB`
  - `256KB`
  - `1MB`
- Record more iterations per case and allow the CLI to override iteration count, payload sizes, and payload profiles.
- Add explicit roundtrip metadata to each `CompressionResult`:
  - input hash
  - payload profile
  - iteration index
  - hash-match flag
  - bytewise-match flag
- Continue failing the benchmark when the roundtrip is incorrect, but also export enough evidence to explain why.
- Update the compression matrix so it reports every payload-profile and payload-size group rather than silently showing only the first payload size.

**Design Notes**

- Payload generation stays deterministic so cross-device comparisons remain reproducible.
- Roundtrip verification should use both hash equality and direct byte vector equality. Hash equality is useful for logs and CSV diffs; bytewise equality is the canonical correctness check.
- The release scripts can keep calling the same benchmark binary, but the heavier default benchmark means iOS polling timeouts should leave enough headroom for slower devices.

**Verification**

- Extend unit tests to cover:
  - profile-expanded result counts
  - roundtrip flags on passing and failing runs
  - JSON and CSV export of the new metadata
  - orchestrator text output that surfaces payload profiles
- Finish with a fresh full `ctest` run.

