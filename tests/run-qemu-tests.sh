#!/bin/sh
set -eu

[ $# -eq 2 ] || { echo "usage: $0 /path/to/simulate expected-guest-payload" >&2; exit 2; }
simulator=$1
expected_guest_payload=$2
sim_dir=$(CDPATH= cd -- "$(dirname -- "$simulator")" && pwd)
sim_name=$(basename -- "$simulator")
log_file=$(mktemp)
trap 'rm -f "$log_file"' EXIT

setsid sh -c 'cd "$1" && exec "$2"' sh "$sim_dir" "./$sim_name" >"$log_file" 2>&1 &
sim_pid=$!
for _ in $(seq 1 300); do
    if grep -q '^UNDERLORD_PHASE2_RESULT: PASS$' "$log_file"; then
        kill -TERM -- "-$sim_pid" 2>/dev/null || true
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
guest_count=$(grep -Fxc "[INFO] vmm[0]-guest: $expected_guest_payload" "$log_file" || true)
cat "$log_file"
[ "$pass_count" -eq 1 ] && [ "$guest_count" -ge 1 ]
