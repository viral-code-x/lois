#!/usr/bin/env bash

# LOIS Core Language Audit
# Diagnostic only. Does not rewrite source files.

set -uo pipefail

REPORT="lois-full-audit-$(date +%Y%m%d-%H%M%S).txt"

exec > >(tee "$REPORT") 2>&1

section() {
    echo
    echo "=================================================="
    echo "$1"
    echo "=================================================="
}

section "1. ENVIRONMENT"
pwd
date
uname -a
cc --version | head -n 2
cmake --version | head -n 1

section "2. GIT STATUS"
git status --short
git branch --show-current
git log -5 --oneline
git diff --check

section "3. PROJECT STRUCTURE"
find . -maxdepth 3 -type f \
    ! -path './.git/*' \
    ! -path './build/*' \
    ! -path './build-audit/*' \
    | sort

section "4. BUILD CONFIGURATION"
if [ -f CMakeLists.txt ]; then
    cat CMakeLists.txt
else
    echo "ERROR: CMakeLists.txt not found"
fi

section "5. CORE SOURCE FILES"
for file in src/*.c src/*.h; do
    [ -f "$file" ] || continue
    echo "--- $file ---"
    wc -l "$file"
done

section "6. FEATURE IMPLEMENTATION LOCATIONS"
grep -RniE \
'upto|decimal|round|precision|sin|cos|tan|asin|acos|atan|sqrt|log|pow|for|while|repeat|stop|skip|input|output|function|return|MAX_LOOP_ITERATIONS' \
src tests \
--include='*.c' --include='*.h' --include='*.is' \
2>/dev/null || true

section "7. EXISTING TEST INVENTORY"
if [ -d tests ]; then
    find tests -type f | sort
    echo
    echo "Total test files:"
    find tests -type f \( -name '*.is' -o -name '*.c' \) | wc -l
else
    echo "No tests directory found"
fi

section "8. NORMAL CLEAN BUILD"
if [ -d build ]; then
    cmake --build build --clean-first --parallel 2
else
    echo "Existing build directory missing."
    echo "Configuring normal build..."
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug &&
    cmake --build build --parallel 2
fi

section "9. REGISTERED TEST SUITE"
if [ -f build/CTestTestfile.cmake ]; then
    ctest --test-dir build --output-on-failure
else
    echo "No registered CTest suite found."
fi

section "10. STRICT COMPILER AND SANITIZER BUILD"

cmake -S . -B build-audit \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_C_FLAGS="-Wall -Wextra -Wpedantic -Wshadow -Wformat=2 -fsanitize=address,undefined -fno-omit-frame-pointer" \
    -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined"

CONFIG_STATUS=$?

if [ "$CONFIG_STATUS" -eq 0 ]; then
    cmake --build build-audit --parallel 2
    BUILD_STATUS=$?

    if [ "$BUILD_STATUS" -eq 0 ]; then
        echo "Sanitizer build completed."
        if [ -f build-audit/CTestTestfile.cmake ]; then
            ASAN_OPTIONS=detect_leaks=1 \
            UBSAN_OPTIONS=print_stacktrace=1 \
            ctest --test-dir build-audit --output-on-failure
        fi
    else
        echo "Sanitizer build FAILED."
    fi
else
    echo "Sanitizer configuration FAILED."
fi

section "11. SOURCE DIFF SUMMARY"
git diff --stat
git status --short

section "12. AUDIT COMPLETE"
echo "Report: $REPORT"
echo "No source files were intentionally modified."
