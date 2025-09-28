#!/bin/bash
# benchmark.sh - Benchmark Orn vs TCC (FIXED for /benchmarks directory)

set -e  # Exit on error

echo "=== ORN COMPILER BENCHMARK ==="
echo "Comparing against TCC (Tiny C Compiler)"
echo

# Verificar que TCC está instalado
if ! command -v tcc &> /dev/null; then
    echo "Error: TCC not found. Install with:"
    echo "  Ubuntu/Debian: sudo apt install tcc"
    echo "  macOS: brew install tcc"
    echo "  Arch: sudo pacman -S tcc"
    exit 1
fi

# Verificar que Orn está compilado (desde benchmarks directory)
if [ ! -f "../build/orn" ]; then
    echo "Error: Orn compiler not found at ../build/orn"
    echo "Build with:"
    echo "  mkdir -p build && cd build && cmake .. && cmake --build ."
    exit 1
fi

# Verificar que runtime existe (desde benchmarks directory)
if [ ! -f "../runtime/runtime.s" ]; then
    echo "Error: Runtime not found at ../runtime/runtime.s"
    echo "Make sure you're running from benchmarks directory"
    exit 1
fi

# Limpiar archivos anteriores
rm -f test.c test.orn test_tcc test_orn *.o *.s

echo "Found Orn compiler at: ../build/orn"
echo "Found runtime at: ../runtime/runtime.s"
echo

# Crear programa de prueba en C para TCC
cat > test.c << 'EOF'
#include <stdio.h>

int fibonacci(int n) {
    if (n <= 1) return n;
    return fibonacci(n-1) + fibonacci(n-2);
}

int main() {
    int result = fibonacci(20);
    printf("%d\n", result);
    return 0;
}
EOF

# Crear programa equivalente en Orn
cat > test.orn << 'EOF'
fn fibonacci(n: int) -> int {
    n <= 1 ? {
        return n;
    };
    return fibonacci(n-1) + fibonacci(n-2);
}

int result = fibonacci(20);
print(result);
EOF

echo "Test programs created (fibonacci calculation)"
echo

# Función para medir tiempo
measure_time() {
    local cmd="$1"
    local desc="$2"
    echo -n "$desc: "
    
    local start_time=$(date +%s%N)
    eval "$cmd" > /dev/null 2>&1
    local end_time=$(date +%s%N)
    
    local duration=$(( (end_time - start_time) / 1000000 ))
    echo "${duration}ms"
    
    return 0
}

echo "--- COMPILATION ONLY ---"

# TCC - solo compilación a object file
measure_time "tcc -c test.c" "TCC compile to .o   "

# Orn - solo compilación a assembly  
measure_time "../build/orn test.orn" "Orn compile to .s   "

echo
echo "--- COMPLETE PIPELINE ---"

# TCC - compilación completa
measure_time "tcc -o test_tcc test.c" "TCC complete        "

# Pipeline completo de Orn
echo -n "Orn complete        : "
start_time=$(date +%s%N)

# Ejecutar pipeline paso a paso (fixed paths)
if ../build/orn test.orn > /dev/null 2>&1 && \
   as --64 -o test.o output.s > /dev/null 2>&1 && \
   as --64 -o runtime.o ../runtime/runtime.s > /dev/null 2>&1 && \
   ld -o test_orn test.o runtime.o > /dev/null 2>&1; then
    
    end_time=$(date +%s%N)
    duration=$(( (end_time - start_time) / 1000000 ))
    echo "${duration}ms"
else
    echo "FAILED"
    echo "Debugging pipeline failure..."
    echo "Step 1: Orn compilation"
    ../build/orn test.orn
    echo "Step 2: Assembly"
    as --64 -o test.o output.s
    echo "Step 3: Runtime assembly"
    as --64 -o runtime.o ../runtime/runtime.s
    echo "Step 4: Linking"
    ld -o test_orn test.o runtime.o
    exit 1
fi

echo
echo "--- VERIFICATION ---"
if [ -f test_tcc ]; then
    echo -n "TCC output: "
    ./test_tcc
else
    echo "TCC executable not created"
fi

if [ -f test_orn ]; then
    echo -n "Orn output: "
    ./test_orn
else
    echo "Orn executable not created"
fi

echo
echo "--- MULTIPLE RUNS (for average) ---"

echo "TCC compilation (5 runs):"
total_tcc=0
for i in {1..5}; do
    start_time=$(date +%s%N)
    tcc -c test.c -o test$i.o > /dev/null 2>&1
    end_time=$(date +%s%N)
    duration=$(( (end_time - start_time) / 1000000 ))
    echo "  Run $i: ${duration}ms"
    total_tcc=$((total_tcc + duration))
done
avg_tcc=$((total_tcc / 5))
echo "  Average: ${avg_tcc}ms"

echo
echo "Orn compilation (5 runs):"
total_orn=0
for i in {1..5}; do
    start_time=$(date +%s%N)
    ../build/orn test.orn > /dev/null 2>&1
    end_time=$(date +%s%N)
    duration=$(( (end_time - start_time) / 1000000 ))
    echo "  Run $i: ${duration}ms"
    total_orn=$((total_orn + duration))
    if [ -f output.s ]; then
        mv output.s output$i.s 2>/dev/null || true
    fi
done
avg_orn=$((total_orn / 5))
echo "  Average: ${avg_orn}ms"

echo
echo "--- PERFORMANCE RATIO ---"
if [ $avg_tcc -gt 0 ]; then
    # Usar aritmética entera para evitar dependencia de bc
    ratio=$((avg_orn * 10 / avg_tcc))
    ratio_int=$((ratio / 10))
    ratio_dec=$((ratio % 10))
    echo "Orn is ${ratio_int}.${ratio_dec}x compared to TCC (average)"
    
    if [ $avg_orn -lt $avg_tcc ]; then
        echo "🎉 Orn is FASTER than TCC!"
    else
        echo "TCC is faster than Orn"
    fi
else
    echo "TCC too fast to measure accurately"
fi

# Limpiar archivos temporales
rm -f test*.o output*.s test_tcc test_orn runtime.o test.c test.orn

echo
echo "=== BENCHMARK COMPLETE ==="