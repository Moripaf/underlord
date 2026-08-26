# Phase 1: Root-Task Hypervisor and One VMM Instance

## Status and purpose

This is an implementation proposal for the first executable hypervisor
milestone. It replaces `my-vmm` as the seL4 root task with a new `hypervisor`
project and has the hypervisor start one separate `my-vmm` task.

The objective is deliberately narrow: prove that Underlord can use its initial
seL4 authority to create, provision, start, and observe a distinct VMM process.
It does **not** boot a Unikraft VM yet. That remains the next VMM milestone.

## Recommended Phase 1 boundary

Make `hypervisor` the only rootserver. It owns the capabilities delivered by
seL4 bootinfo and is the only component allowed to allocate kernel objects from
the initial untyped memory. `my-vmm` becomes an ordinary user-space child task
with its own CSpace, VSpace, TCB, and fault endpoint.

```
QEMU / elfloader
        │
        ▼
seL4 kernel
        │ bootinfo + initial capabilities
        ▼
hypervisor (root task / rootserver)
  ├── root allocator and capability authority
  ├── VMM image archive
  ├── VMM task record
  ├── fault endpoint / observation loop
  └── starts one child
          │
          ▼
my-vmm (non-root VMM task)
  ├── its own allocator bootstrap
  ├── receives only delegated capabilities
  └── initially reports startup, then waits
```

This is the right ownership direction for the long-term architecture. A VMM
must be restartable and constrained by the hypervisor; making it the root task
would give it authority that is hard to later retract or account for.

## Key design decisions

### Use the seL4 process helpers for Phase 1

Use `sel4utils_configure_process_custom()` plus
`sel4utils_spawn_process[_v]()` to create and start the child. They already
create a CNode, VSpace root, fault endpoint, TCB, user stack, and ELF mappings.
The hypervisor should use the `simple_t`-based process configuration so the
child receives the boot-time authority required by sel4runtime.

Do not hand-roll TCB register configuration or ELF mapping in this phase. The
process helpers give a smaller, well-understood first step; later requirements
such as fixed virtual layout or multiple vCPUs can justify replacing selected
parts with custom setup.

### Package the VMM as a child ELF

The rootserver mechanism can start only one initial task, so `my-vmm` must be
included inside the hypervisor image as a CPIO entry. The existing seL4 CMake
helper `MakeCPIO()` and `libcpio` support this model, and `sel4utils` loads an
ELF by name from that archive.

The build should therefore:

1. build `my-vmm` as an executable but **not** declare it as a rootserver;
2. build `hypervisor` as the sole rootserver;
3. create a CPIO archive containing the VMM ELF under a stable name such as
   `my-vmm`; and
4. link that archive into `hypervisor` under the default `_cpio_archive`
   symbol expected by `sel4utils`.

The first implementation must verify at runtime that the archive contains the
expected name before attempting the spawn.

### Keep capability delegation explicit, even while it is coarse

“Allocate all of the VMM's capabilities” should mean that the hypervisor
creates and installs every cap the VMM needs in the VMM's CSpace. It should not
mean copying the root task's entire initial CSpace unchanged into the VMM.

For Phase 1, use a small, explicit capability manifest. It can be broad and
will be refined later, but its contents and destinations must be named in code.
The hypervisor retains the source caps and passes derived copies to the VMM.

| Capability category | Phase 1 VMM grant | Reason |
|---|---|---|
| VMM's TCB, CSpace, VSpace | created by hypervisor | Required to run a separate task; not general root authority |
| ASID pool / initial runtime caps | required `sel4runtime` grants | Required by the standard child-process setup |
| Fault endpoint | one VMM fault endpoint | Lets the hypervisor observe VMM failure |
| Debug output | only if the VMM needs current `printf` diagnostics | Temporary development grant |
| Untyped memory / allocator authority | a deliberately chosen coarse pool, or none until needed | Required for VMM-side object allocation; never pass all root untypeds by default |
| IRQ, device, I/O, scheduling authority | none in Phase 1 | No guest or virtual device exists yet |
| VCPU and VM-related caps | none in Phase 1 | Belong to the later VM-boot milestone |

The practical recommendation is to first pass **no untyped caps** until the
VMM has a concrete allocation requirement. If the VMM must bootstrap an
allocator now, pass a bounded set of designated untyped caps and record their
slots in the manifest. This keeps the Phase 1 demonstration meaningful without
pretending that full resource policy already exists.

## Component responsibilities

### `hypervisor`

The new component owns:

- root-task bootstrap (`platsupport_get_bootinfo`, `simple_t`, `allocman`,
  `vka_t`, and the root VSpace);
- storage for an explicit `vmm_instance_t` record;
- child-process construction, capability installation, and start;
- a VMM fault endpoint and fault-reporting loop; and
- status logging: `created`, `started`, `faulted`, and `exited/not running`.

The initial `vmm_instance_t` should retain the `sel4utils_process_t`, its
fault endpoint, a lifecycle state, and a stable instance ID. One static record
is sufficient for Phase 1, but using the record now avoids baking global VMM
state into the hypervisor.

### `my-vmm`

Move the allocator bootstrap currently in `my-vmm/src/init.c` to remain with
the VMM. The VMM should initialise only from capabilities intentionally placed
in its CSpace; it must not rely on being the root task or on the full original
bootinfo capability set.

Its initial Phase 1 behaviour should be minimal and observable:

1. report that it started;
2. bootstrap its supplied allocator, if an untyped pool is granted;
3. report success or a structured failure; and
4. wait on a control endpoint or block safely.

An infinite busy loop is not a useful steady state: it consumes CPU and gives
the hypervisor no lifecycle boundary. A temporary blocking wait is sufficient
until the real control protocol exists.

## Implementation sequence

### 1. Restructure the CMake build

- Add `hypervisor/` with its own `CMakeLists.txt`, `src/`, and `include/`.
- Change the top-level CMake project to add both `hypervisor` and `my-vmm`.
- Remove `DeclareRootserver(my-vmm)`; declare `hypervisor` as the sole
  rootserver.
- Build `my-vmm` first, package its ELF with `MakeCPIO()`, and link the archive
  into the hypervisor target.
- Keep the generated `build/simulate` script as the integration-test entry
  point; it should now boot the hypervisor rootserver.

### 2. Establish root-task bootstrap in `hypervisor`

- Move or factor the root bootstrap code out of `my-vmm` into a
  hypervisor-owned module.
- Check every return value and abort startup with an unambiguous diagnostic if
  bootinfo, allocman, VKA setup, or VSpace setup fails.
- Do not share the current VMM globals across the process boundary. Each task
  owns its own state.

### 3. Create and start one VMM process

- Construct a `sel4utils_process_config_t` using the root task's `simple_t`.
- Configure the child from the `my-vmm` CPIO image.
- Allocate the named VMM capability manifest into the child CSpace.
- Spawn the VMM with the normal sel4runtime-compatible entry path.
- Store the returned process resources in `vmm_instance_t` and emit a start
  log including its instance ID.

### 4. Add minimum supervision

- Bind the VMM's fault endpoint to the hypervisor's wait loop.
- Decode and log a VMM fault with its instance ID.
- Treat a fault as terminal in Phase 1; do not attempt restart yet.
- Keep a single in-memory state transition path:
  `created → starting → running → faulted`.

### 5. Validate on the simulator

Run this feedback loop after each milestone:

```sh
cd $PROJECTROOT/src
./init-build.sh
cd build
ninja
./simulate
```

Expected console order for the completed Phase 1 milestone:

```text
hypervisor: root allocator ready
hypervisor: VMM instance 0 created
hypervisor: VMM instance 0 started
my-vmm[0]: started
my-vmm[0]: allocator ready
```

The exact strings may change, but the ordering proves that the root task
created a separate task and that the child executed independently.

## Definition of done

Phase 1 is complete when all of the following are true:

- `hypervisor` is the only declared rootserver.
- `my-vmm` is packaged as a CPIO child ELF, not booted directly by elfloader.
- The hypervisor creates one VMM with a distinct TCB, CSpace, and VSpace.
- The VMM starts, prints a child-only startup message, and blocks without
  consuming CPU.
- The capability manifest is present in code and documents every VMM grant.
- A VMM fault reaches the hypervisor's fault endpoint and is logged with the
  VMM instance ID.
- A clean `init-build.sh`, `ninja`, and `./simulate` run reproduces the result.

## Explicitly deferred

Phase 1 must not expand into these later concerns:

- starting a Unikraft guest or creating a seL4 VM/vCPU;
- multiple VMMs, restart, stop, unload, or resource reclamation;
- dynamic requests, a management API, CLI, or wire protocol;
- fine-grained capability policy, quotas, revocation, or accounting;
- device delegation/emulation, IRQ routing, or virtual devices; and
- persistent monitoring, metrics, or a scheduler policy.

The primary output is a trustworthy process and authority boundary. Once this
exists, VM lifecycle and resource-management work can be added behind it
without moving root-task authority back into the VMM.
