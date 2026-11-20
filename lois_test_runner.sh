#!/usr/bin/env bash

set -u

EXE="./build/lois"
RESULTS="test-results/core-audit"
TIMEOUT=15

mkdir -p "$RESULTS/pass-candidates" "$RESULTS/failed" \
         "$RESULTS/skipped-input" "$RESULTS/invalid"

if [ ! -x "$EXE" ]; then
    echo "LOIS executable not found. Build first."
    exit 1
fi

TOTAL=0
RAN=0
SKIPPED=0
FAILED=0
NONZERO=0
TIMED_OUT=0

echo "LOIS CORE TEST RUN"
echo "Executable: $EXE"
echo

while IFS= read -r -d '' file; do
    TOTAL=$((TOTAL + 1))
    name=$(basename "$file" .is)

    # Deliberately invalid programs are tested separately.
    if [[ "$name" == invalid_* || "$name" == error_* ||
          "$name" == add_invalid || "$name" == set_final_bad_* ]]; then
        cp "$file" "$RESULTS/invalid/"
        SKIPPED=$((SKIPPED + 1))
        echo "INVALID-CASE: $file"
        continue
    fi

    # Interactive programs need controlled input fixtures.
    if grep -Eq '^[[:space:]]*(then[[:space:]]+)?input([[:space:]]|$)' "$file"; then
        cp "$file" "$RESULTS/skipped-input/"
        SKIPPED=$((SKIPPED + 1))
        echo "INPUT-REQUIRED: $file"
        continue
    fi

    RAN=$((RAN + 1))
    output="$RESULTS/pass-candidates/$name.txt"

    timeout "$TIMEOUT" "$EXE" "$file" >"$output" 2>&1
    status=$?

    if [ "$status" -eq 0 ]; then
        echo "EXIT-0: $file"
    elif [ "$status" -eq 124 ]; then
        TIMED_OUT=$((TIMED_OUT + 1))
        FAILED=$((FAILED + 1))
        mv "$output" "$RESULTS/failed/$name.txt"
        echo "TIMEOUT: $file"
    else
        NONZERO=$((NONZERO + 1))
        FAILED=$((FAILED + 1))
        mv "$output" "$RESULTS/failed/$name.txt"
        echo "NONZERO ($status): $file"
    fi

done < <(find tests -type f -name '*.is' -print0 | sort -z)

echo
echo "===================================="
echo "TOTAL:          $TOTAL"
echo "EXECUTED:       $RAN"
echo "SKIPPED:        $SKIPPED"
echo "NONZERO:        $NONZERO"
echo "TIMEOUTS:       $TIMED_OUT"
echo "FAILED:         $FAILED"
echo "===================================="

echo
echo "Results saved in: $RESULTS"
