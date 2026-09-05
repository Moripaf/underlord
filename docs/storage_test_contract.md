# Storage test guest contracts

## 9P filesystem preflight

`c-fs-9p` is a direct-QEMU AArch64 test guest for the single-user filesystem
layer needed before SQLite. It mounts the `fs0` 9P2000.L export at `/data` and
proves exclusive create, write, patch, file `fsync`, truncate, rename, unlink,
and readback across a new guest process. A stopped guest permits Linux to add
`host.txt`, which the next guest invocation reads exactly.

This guest does not test locking, WAL, SQLite, crash recovery, or power-loss
durability. Underlord must later expose a modern virtio-MMIO 9P device through
the guest FDT, provide mount tag `fs0`, forward 9P2000.L requests to a backing
filesystem that survives VMM recreation, and pass its selected mode as a boot
argument. Direct-QEMU results do not establish Underlord `build/simulate`
support.
