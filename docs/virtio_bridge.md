# Virtio bridge

The `virtio-bridge/` directory currently owns the fixed-size control contract
for the planned synchronous QEMU virtio-9p bridge. Commands are versioned and
carry lengths only; request and response bytes remain in separately mapped
512 KiB buffers. No guest memory, device frame, IRQ, or host-directory
authority is granted by this library.

The pure `virtio_mmio_model` also enforces modern transport version 2, the 9P
mount-tag and version-1 feature bits, and a single queue capped at 128
descriptors. It is host-tested before being connected to guest MMIO faults.

The bridge process and VMM transport are not enabled yet. They must be added
together with the hypervisor capability and shared-buffer mappings, then
validated with the `c-fs-9p` acceptance guest through `build/simulate`.

`tests/run-9p-simulate.sh` provides the reproducible QEMU backend argument
shape from the catalog reference. It requires an existing export directory and
acceptance mode; the mode is carried by the generated guest FDT.

The generated guest FDT now supplies the configured `UNDERLORD_C_FS_9P_MODE`
boot argument (default `exercise`) and
advertises the reserved virtio-MMIO location; legacy guests ignore the extra
node and argument.
