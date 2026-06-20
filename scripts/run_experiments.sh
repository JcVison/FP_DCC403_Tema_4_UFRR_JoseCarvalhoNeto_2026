#!/bin/bash

echo "policy,page_kb,physical_kb,total_accesses,page_faults,disk_writes,fault_rate"

for policy in lru fifo random
do
  for page in 4 8 16
  do
    for mem in 128 256 512 1024 2048
    do
      output=$(./edge-mem-profiler $policy tests/trace_teste.log $page $mem)

      total=$(echo "$output" | grep "Total de Acessos" | awk '{print $5}')
      faults=$(echo "$output" | grep "Page Faults" | awk '{print $6}')
      writes=$(echo "$output" | grep "Paginas Sujas" | awk '{print $5}')
      rate=$(echo "$output" | grep "Taxa de Page Faults" | awk '{print $5}' | tr -d '%')

      echo "$policy,$page,$mem,$total,$faults,$writes,$rate"
    done
  done
done
