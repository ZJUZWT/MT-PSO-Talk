# Mesa Patch Notes

This directory stores the benchmark-specific Mesa instrumentation patches that
emit stable CPU-side stage timing events for PSO-related work.

The initial parser in `Mesa/adapters` expects simple key-value lines such as:

```text
stage=frontend duration_us=12
```

Future tasks will replace placeholder test data with real Mesa trace output and
document the exact patch application flow here.
