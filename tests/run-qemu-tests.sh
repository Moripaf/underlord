#!/bin/sh
set -eu

[ $# -eq 1 ] || { echo "usage: $0 /path/to/simulate" >&2; exit 2; }
simulator=$1
log_file=$(mktemp)
trap 'rm -f "$log_file"' EXIT

if ! timeout 30s "$simulator" >"$log_file" 2>&1; then
    cat "$log_file"
    exit 1
fi

pass_count=$(grep -c '^UNDERLORD_TEST_RESULT: PASS$' "$log_file" || true)
fail_count=$(grep -c '^UNDERLORD_TEST_RESULT: FAIL$' "$log_file" || true)
cat "$log_file"
[ "$pass_count" -eq 1 ] && [ "$fail_count" -eq 0 ]
