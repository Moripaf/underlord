#!/bin/sh
set -eu

[ $# -eq 1 ] || { echo "usage: $0 /path/to/simulate" >&2; exit 2; }
simulator=$1
sim_dir=$(CDPATH= cd -- "$(dirname -- "$simulator")" && pwd)
sim_name=$(basename -- "$simulator")
log_file=$(mktemp)
trap 'rm -f "$log_file"' EXIT

(
    cd "$sim_dir"
    "./$sim_name"
) >"$log_file" 2>&1 &
sim_pid=$!
for _ in $(seq 1 300); do
    if grep -q '^UNDERLORD_PHASE2_RESULT: PASS$' "$log_file"; then
        kill "$sim_pid" 2>/dev/null || true
        wait "$sim_pid" 2>/dev/null || true
        break
    fi
    if ! kill -0 "$sim_pid" 2>/dev/null; then
        wait "$sim_pid" 2>/dev/null || true
        break
    fi
    sleep 0.1
done

pass_count=$(grep -c '^UNDERLORD_PHASE2_RESULT: PASS$' "$log_file" || true)
hello_count=$(grep -c '^\[INFO\] vmm\[0\]-guest: Hello from Unikraft!$' "$log_file" || true)
cat "$log_file"
[ "$pass_count" -eq 1 ] && [ "$hello_count" -ge 1 ]
