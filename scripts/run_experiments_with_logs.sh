#!/bin/bash

set -euo pipefail

mkdir -p logs docs

OUT_CSV=docs/resultados_with_logs.csv
echo "policy,page_kb,physical_kb,total_accesses,page_faults,disk_writes,fault_rate,log_file" > "$OUT_CSV"

for policy in lru fifo random; do
  for page in 4 8 16; do
    for mem in 128 256 512 1024 2048; do
      log_file="logs/run_${policy}_${page}kb_${mem}kb.log"
      echo "Run: $(date)" > "$log_file"
      echo "Policy: $policy  Page: ${page}KB  Physical: ${mem}KB" >> "$log_file"

      ./edge-mem-profiler "$policy" tests/trace_teste.log "$page" "$mem" >> "$log_file" 2>&1 || true

      output=$(cat "$log_file")

      total=$(echo "$output" | grep "Total de Acessos" | awk '{print $5}' || echo "0")
      faults=$(echo "$output" | grep "Page Faults" | awk '{print $6}' || echo "0")
      writes=$(echo "$output" | grep "Paginas Sujas" | awk '{print $5}' || echo "0")
      rate=$(echo "$output" | grep "Taxa de Page Faults" | awk '{print $5}' | tr -d '%' || echo "0")

      echo "$policy,$page,$mem,$total,$faults,$writes,$rate,$log_file" >> "$OUT_CSV"
    done
  done
done

echo "Finished. CSV: $OUT_CSV"
