#!/bin/bash
mkdir -p build
gcc -O2 -Wall -Wextra src/odts_multivar.c -o build/odts -lm
./build/odts
echo ""
echo "=== TRACE ==="
cat odts_trace_c.csv
echo ""
echo "=== AUDIT JSON ==="
cat odts_audit_c.json
