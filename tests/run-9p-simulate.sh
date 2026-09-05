#!/bin/sh
set -eu

test "$#" -eq 3 || {
    echo "usage: $0 /path/to/simulate /path/to/export exercise|verify|verify-host" >&2
    exit 2
}
simulator=$1
export_dir=$2
mode=$3
test -x "$simulator" || { echo "simulate is not executable" >&2; exit 2; }
test -d "$export_dir" || { echo "export directory does not exist" >&2; exit 2; }
case "$mode" in exercise|verify|verify-host) ;; *) exit 2 ;; esac

args="-global virtio-mmio.force-legacy=false -fsdev local,id=cfs9p,path=$export_dir,security_model=none -device virtio-9p-device,fsdev=cfs9p,mount_tag=fs0"
exec "$simulator" --extra-qemu-args "$args"
