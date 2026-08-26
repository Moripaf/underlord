# Testing Strategy: AArch64 QEMU

## Decision

Use two complementary test layers:

1. **Host unit tests** for pure C logic that has no seL4 dependency.
2. **seL4/QEMU tests** for all capability, allocator, process, IPC, fault, and
   ELF/CPIO behaviour.

The current checkout's `libsel4test` pulls an unavailable `sel4rpc`/nanopb
dependency, so the test rootserver uses a small local runner for the extracted
pure contracts. It remains a separate target and can migrate to `libsel4test`
when that dependency is available.

Do not make host tests the acceptance criterion for kernel-facing code. The
acceptance suite must build the actual `qemu-arm-virt`/AArch64 configuration and
run it with `src/build/simulate`.

## Test layout

```
src/
├── underlord-utils/
│   └── tests/                   # Host tests for pure format/state helpers
├── hypervisor/
│   └── tests/                   # Target-side module/component tests
├── my-vmm/
│   └── tests/                   # Target-side VMM protocol tests
└── tests/
    ├── sel4/                    # Common test runner and test rootserver
    ├── integration/             # Hypervisor + child-VMM scenarios
    └── run-qemu-tests.sh        # Starts simulate and checks machine-readable result
```

Tests must not include a component's `main.c`. Split testable code into
libraries or small modules first—for example, VMM lifecycle transitions,
capability-manifest validation, CPIO-image validation, and log-message
formatting. `main.c` should only wire those modules together.

## Unit tests by component

| Component | Host unit tests | seL4/QEMU component tests |
|---|---|---|
| `underlord-utils` | Log-level name lookup, prefix construction, truncation, and invalid levels. Extract formatting from the direct `printf` backend so it can be asserted. | Verify emitted target output has the expected `[LEVEL] module:` form. |
| `hypervisor` | Lifecycle transition rules and capability-manifest policy, if factored into pure modules. | Root bootstrap succeeds; CPIO contains `my-vmm`; child process configures; control cap appears only at the manifest slot; failure paths leave the instance non-running. |
| `my-vmm` | Control-message encoding/decoding and local state machines, once introduced. | Starts with its expected slot; sends/receives a control message; blocks when idle; fault-test build faults predictably. |
| CPIO/ELF packaging | Archive-name and metadata validation that can be pure. | The target-side loader finds and loads the embedded VMM ELF. |

Keep capability tests target-side. A host test cannot prove CSpace contents,
cap rights, TCB configuration, fault delivery, or seL4 IPC semantics.

## Target-side unit/component runner

Add an `UNDERLORD_BUILD_TESTS` CMake option, defaulting to `OFF`. When enabled:

- link a dedicated `underlord-sel4-tests` rootserver against the modules being
  tested;
- add a small local runner that prints one
  machine-readable terminal record: `UNDERLORD_TEST_RESULT: PASS` or `FAIL`;
- have the test rootserver stop in a controlled way after reporting its result.

Use a separate test rootserver/image rather than adding test code to the normal
`hypervisor` image. This keeps production capabilities and test-only fault
injection out of the normal artifact.

The runner owns its local test functions and result record. It does not add
test-only capabilities to the production image.

## Integration tests

The integration target must be the real hypervisor build, not a mocked process
test. Add an internal, versioned control protocol before writing these tests.
The smallest useful protocol is a child-to-hypervisor `READY` message and a
test-only `FAULT` scenario; this is an internal Phase 1 protocol, not the later
external management API.

Required scenarios:

1. **Normal start:** hypervisor bootstraps, finds the CPIO ELF, starts instance
   `0`, receives `READY`, and marks it `running`.
2. **Capability boundary:** the VMM receives its control endpoint at slot 8;
   it does not receive untyped, IRQ, device, I/O, VM, or vCPU capabilities.
3. **Fault supervision:** configure `VMM_FAULT_TEST=ON`; the VMM faults after
   startup; the hypervisor receives the fault endpoint event and marks instance
   `0` `faulted`.
4. **Packaging failure:** a deliberately absent/renamed CPIO entry produces a
   clear hypervisor failure and does not start a child.
5. **Regression smoke test:** normal `hypervisor` image emits the expected
   startup sequence and does not unexpectedly fault during a bounded run.

Do not use console text as the sole proof of correctness. Logs are useful
diagnostics, but integration pass/fail should be driven by explicit control
messages and the terminal result marker.

## QEMU execution and automation

The normal runnable artifact remains `src/build/simulate`. A test wrapper
should run it with a timeout, capture the serial log, require exactly one
terminal result marker, and return a non-zero host exit status on failure or
timeout. This turns the cross-compiled emulator run into a CI-friendly command.

Suggested developer commands:

```sh
cd $PROJECTROOT/src
./init-build.sh --build-dir build-tests -DUNDERLORD_BUILD_TESTS=ON
cd build-tests
ninja
ctest --output-on-failure
../tests/run-qemu-tests.sh ./simulate
```

The test wrapper must preserve the configured simulator defaults: QEMU
`qemu-arm-virt`, AArch64, GICv2, 1024 MiB memory, and ARM virtualization
enabled. That ensures failures are representative of the deployable build.

## Adoption order

1. Factor pure logic out of `underlord-utils`, `hypervisor`, and `my-vmm`.
2. Add host unit tests for that pure logic and run them through CTest.
3. Add `UNDERLORD_BUILD_TESTS`, the target-side runner, and the
   QEMU result wrapper.
4. Add the normal-start and fault-supervision integration scenarios.
5. Gate changes on host unit tests plus the AArch64 QEMU suite.

Every change to test architecture, build targets, result protocol, or simulator
workflow must update this document and `project_architecture.md` when the
current implementation changes.

## Implemented test boundary

The test option selects `underlord-sel4-tests` as a separate rootserver rather
than adding hooks to the normal hypervisor image. Its local runner cases cover
the extracted utils formatter, hypervisor lifecycle/image validation, and VMM
READY protocol. The QEMU wrapper enforces a 30-second timeout and accepts only
one PASS result marker. Production integration scenarios are run from the
normal build and retain the READY handshake as their machine-checkable startup
evidence.
