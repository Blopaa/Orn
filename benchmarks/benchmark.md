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

Scaling Analysis:

Better than linear scaling: Input size increased 100x (1k → 100k), but compilation time only increased ~53x (18ms → 962ms for Orn)
This indicates linear time complexity with significant constant overhead dominating small inputs
The widening performance gap at small scales suggests constant factors (process setup, I/O, syscalls) dominate the measurement

Constant Overhead Analysis:

Small input performance is unreliable due to constant factors
TCC at N=100k: 64ms suggests ~0.64ms for N=1k if purely linear, but measures 8ms (7ms constant overhead)
Orn at N=100k: 962ms suggests ~9.6ms for N=1k if purely linear, but measures 18ms (8.4ms constant overhead)
For measurements under 100ms, constant overhead significantly skews results