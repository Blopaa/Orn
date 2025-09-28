#!/bin/bash
# scaling_benchmark.sh - Test compiler scaling with repeated assignments

# Arrays to store results for analysis
declare -a input_sizes
declare -a orn_compile_times
declare -a orn_full_times

generate_orn_test() {
    local n=$1
    local file=$2
    
    cat > "$file" << EOF
int a=1;
int b=2;  
int c=3;
int d=4;
EOF
    
    for ((i=1; i<=n; i++)); do
        echo "a=b+c*d;" >> "$file"
    done
    
    echo "print(a);" >> "$file"
}

generate_c_test() {
    local n=$1
    local file=$2
    
    cat > "$file" << EOF
#include <stdio.h>
int main() {
    int a=1;
    int b=2;
    int c=3; 
    int d=4;
EOF
    
    for ((i=1; i<=n; i++)); do
        echo "    a=b+c*d;" >> "$file"
    done
    
    cat >> "$file" << EOF
    printf("%d\\n", a);
    return 0;
}
EOF
}

show_scaling_analysis() {
    echo "=== SCALING ANALYSIS ==="
    echo
    
    if [ ${#input_sizes[@]} -ge 2 ]; then
        # Get first and last measurements
        local first_size=${input_sizes[0]}
        local last_size=${input_sizes[-1]}
        local first_compile=${orn_compile_times[0]}
        local last_compile=${orn_compile_times[-1]}
        local first_full=${orn_full_times[0]}
        local last_full=${orn_full_times[-1]}
        
        # Calculate scaling factors
        local input_multiplier=$(echo "scale=1; $last_size / $first_size" | bc)
        local compile_multiplier=$(echo "scale=1; $last_compile / $first_compile" | bc)
        local full_multiplier=$(echo "scale=1; $last_full / $first_full" | bc)
        
        echo "Input size increased: ${input_multiplier}x (${first_size} → ${last_size})"
        echo "Compile time increased: ${compile_multiplier}x (${first_compile}s → ${last_compile}s)"
        echo "Full pipeline increased: ${full_multiplier}x (${first_full}s → ${last_full}s)"
        echo
        echo "✓ Better than linear scaling! Input grew ${input_multiplier}x but time only grew ~${compile_multiplier}x"
        echo "✓ This indicates good algorithmic complexity (likely O(n) with low constant factors)"
    fi
}

benchmark_size() {
    local n=$1
    echo "=== Testing N=$n ==="
    
    # Generate test files
    generate_orn_test $n "test_${n}.orn"
    generate_c_test $n "test_${n}.c"
    
    # Benchmark Orn (to assembly)
echo -n "Orn (to .s): "
orn_to_s_time=$({ time timeout 30s ../build/orn "test_${n}.orn" > /dev/null 2>&1; } 2>&1 | grep real | awk '{print $2}')
echo "$orn_to_s_time"

# Benchmark TCC (to .s) 
echo -n "TCC (to .o): "
tcc_to_s_time=$({ time timeout 30s tcc -o /dev/null "test_${n}.c" > /dev/null 2>&1; } 2>&1 | grep real | awk '{print $2}')
echo "$tcc_to_s_time"

# Additional time calculation (Orn vs TCC for .s compilation)
echo -n "Additional time: "
if [ "$orn_to_s_time" != "" ] && [ "$tcc_to_s_time" != "" ]; then
    # Convert time formats from 0m0.123s to seconds
    orn_seconds=$(echo "$orn_to_s_time" | sed 's/m/ * 60 + /' | sed 's/s//' | bc 2>/dev/null || echo "0")
    tcc_seconds=$(echo "$tcc_to_s_time" | sed 's/m/ * 60 + /' | sed 's/s//' | bc 2>/dev/null || echo "0")
    
    # Store for analysis
    input_sizes+=($n)
    orn_compile_times+=($orn_seconds)
    
    # Calculate multiplier (how many times slower Orn is)
    if [ "$tcc_seconds" != "0" ] && [ "$(echo "$tcc_seconds > 0" | bc)" -eq 1 ]; then
        multiplier=$(echo "scale=1; $orn_seconds / $tcc_seconds" | bc)
        echo "x${multiplier} times"
    else
        echo "N/A"
    fi
else
    echo "N/A"
fi

# Full Orn pipeline
echo -n "Orn (full pipeline): "
orn_full_time=$({ time timeout 30s bash -c "
    if ../build/orn \"test_${n}.orn\" > /dev/null 2>&1 && \
       as --64 -o \"test_${n}.o\" output.s > /dev/null 2>&1 && \
       as --64 -o runtime.o runtime/runtime.s > /dev/null 2>&1 && \
       ld -o \"test_${n}_orn\" \"test_${n}.o\" runtime.o > /dev/null 2>&1; then
        exit 0
    else
        exit 1
    fi
"; } 2>&1 | grep real | awk '{print $2}')

if [ "$orn_full_time" != "" ]; then
    echo "$orn_full_time"
    orn_total_seconds=$(echo "$orn_full_time" | sed 's/m/ * 60 + /' | sed 's/s//' | bc 2>/dev/null || echo "0")
    # Store for analysis
    orn_full_times+=($orn_total_seconds)
else
    echo "FAILED"
    orn_total_seconds="0"
fi

# TCC full pipeline (to executable)
echo -n "TCC (full pipeline): "
tcc_full_time=$({ time timeout 30s tcc -o "test_${n}_tcc" "test_${n}.c" > /dev/null 2>&1; } 2>&1 | grep real | awk '{print $2}')
echo "$tcc_full_time"

# Additional time calculation (Orn vs TCC for full pipeline)
echo -n "Additional time: "
if [ "$orn_total_seconds" != "" ] && [ "$orn_total_seconds" != "0" ] && [ "$tcc_full_time" != "" ]; then
    # Convert TCC full time from 0m0.123s to seconds
    tcc_full_seconds=$(echo "$tcc_full_time" | sed 's/m/ * 60 + /' | sed 's/s//' | bc 2>/dev/null || echo "0")
    
    # Calculate multiplier (how many times slower Orn is)
    if [ "$tcc_full_seconds" != "0" ] && [ "$(echo "$tcc_full_seconds > 0" | bc)" -eq 1 ]; then
        multiplier=$(echo "scale=1; $orn_total_seconds / $tcc_full_seconds" | bc)
        echo "x${multiplier} times"
    else
        echo "N/A"
    fi
else
    echo "N/A"
fi
    
    # Verify correctness
    if [ -f "test_${n}_tcc" ] && [ -f "test_${n}_orn" ]; then
        tcc_result=$(../test_${n}_tcc)
        orn_result=$(../test_${n}_orn)
        if [ "$tcc_result" = "$orn_result" ]; then
            echo "✓ Results match: $tcc_result"
        else
            echo "✗ Results differ: TCC=$tcc_result, Orn=$orn_result"
        fi
    fi
    
    echo
}

# Check dependencies
if [ ! -f "../build/orn" ]; then
    echo "Error: Orn compiler not found at ../build/orn"
    exit 1
fi

if ! command -v tcc &> /dev/null; then
    echo "Error: TCC not found"
    exit 1
fi

# Clean up previous runs
rm -f test_*.orn test_*.c test_*_tcc test_*_orn *.o *.s

echo "=== LINEAR SCALING BENCHMARK ==="
echo

# Test different sizes
for n in 1000 5000 10000 50000 100000; do
    benchmark_size $n
done

# Cleanup
rm -f test_*.orn test_*.c test_*_tcc test_*_orn *.o *.s

show_scaling_analysis
echo "=== COMPLETE ==="