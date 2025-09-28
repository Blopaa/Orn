# Orn Compiler Benchmarks

This document presents benchmark results comparing the Orn compiler against TCC (Tiny C Compiler) across different test scenarios.

## Basic Performance Benchmark

### Test Configuration
- **Test Program**: Fibonacci calculation
- **Comparison Target**: TCC (Tiny C Compiler)
- **Metrics**: Compilation time and complete pipeline execution

### Results Summary

| Metric | TCC | Orn | Ratio |
|--------|-----|-----|-------|
| Compilation Only (average) | 7ms | 8ms | 1.1x slower |
| Complete Pipeline | 9ms | 41ms | 4.6x slower |

### Detailed Results

**Compilation Phase Only:**
- TCC compile to .o: 8ms
- Orn compile to .s: 8ms

**Complete Pipeline:**
- TCC complete: 9ms
- Orn complete: 41ms

**Multiple Run Averages (5 runs each):**
- TCC compilation: 7ms average (range: 7-8ms)
- Orn compilation: 8ms average (consistent 8ms)

**Output Verification:**
- Both compilers produce identical results (output: 6765)

## Linear Scaling Benchmark

This benchmark tests how compilation time scales with input size, using varying values of N to generate different program complexities.

### Scaling Results

| Input Size (N) | Orn to .s | TCC to .o | Ratio | Orn Pipeline | TCC Pipeline | Pipeline Ratio |
|----------------|-----------|-----------|-------|--------------|--------------|----------------|
| 1,000 | 18ms | 8ms | 2.2x | 60ms | 10ms | 6.0x |
| 5,000 | 54ms | 11ms | 4.9x | 114ms | 13ms | 8.7x |
| 10,000 | 106ms | 14ms | 7.5x | 205ms | 18ms | 11.3x |
| 50,000 | 485ms | 36ms | 13.4x | 834ms | 44ms | 18.9x |
| 100,000 | 962ms | 64ms | 15.0x | 1,548ms | 75ms | 20.6x |

### Key Observations

**Compilation Phase Scaling:**
- Orn shows non-linear scaling behavior, with performance gap widening as input size increases
- At N=1,000: Orn is 2.2x slower than TCC
- At N=100,000: Orn is 15.0x slower than TCC

**Complete Pipeline Scaling:**
- Pipeline overhead becomes more pronounced with larger inputs
- Performance gap ranges from 6.0x to 20.6x slower than TCC
- All test cases produce verified identical outputs (result: 14)

## Performance Analysis

### Strengths
- **Correctness**: All benchmarks show identical output verification between Orn and TCC
- **Consistency**: Orn shows consistent performance in basic benchmarks

### Areas for Optimization
- **Pipeline Efficiency**: Complete pipeline shows significant overhead compared to TCC
- **Scaling Performance**: Non-linear scaling suggests potential algorithmic improvements needed
- **Large Input Handling**: Performance gap widens substantially with larger codebases

## Benchmark Environment

- **Orn Compiler**: Located at `../build/orn`
- **Runtime**: `../runtime/runtime.s`
- **Comparison Baseline**: TCC (Tiny C Compiler)
- **Test Methodology**: Multiple runs with averaged results for reliability