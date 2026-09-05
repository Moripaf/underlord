#pragma once

#include <stddef.h>

/***
 * @file vmm_guest_console.h
 * Bounded, architecture-independent capture of one guest console stream.
 */

#define VMM_GUEST_CONSOLE_LINE_MAX 128U

/***
 * @struct vmm_guest_console_t
 * Fixed-memory guest output recognizer.
 * @param line Current unterminated line bytes.
 * @param length Number of bytes in line.
 * @param saw_cr True when the previous byte was carriage return.
 * @param hello_progress Number of C hello-token bytes matched in stream.
 * @param c_fs_progress Number of c-fs marker bytes matched in stream.
 * @param guest_started True after an approved guest-start token was observed.
 */
typedef struct {
    char line[VMM_GUEST_CONSOLE_LINE_MAX];
    size_t length;
    int saw_cr;
    size_t hello_progress;
    size_t c_fs_progress;
    int guest_started;
} vmm_guest_console_t;

/*** @function vmm_guest_console_init(console)
 * Initialize bounded guest console state.
 * @param console Writable state; must not be NULL.
 * @return None.
 * @sideeffect Clears all state.
 */
void vmm_guest_console_init(vmm_guest_console_t *console);

/*** @function vmm_guest_console_feed(console, bytes, length, emit, cookie)
 * Consume guest UART bytes and emit completed normalized lines.
 * @param console Initialized state.
 * @param bytes Input bytes; may be NULL only when length is zero.
 * @param length Number of input bytes.
 * @param emit Callback receiving a NUL-terminated line without CR/LF.
 * @param cookie Opaque callback value.
 * @return 0 on success, -1 on invalid arguments.
 * @sideeffect Updates buffering and guest-start recognition; calls emit on line end.
 */
int vmm_guest_console_feed(vmm_guest_console_t *console, const char *bytes,
                           size_t length, void (*emit)(const char *, void *),
                           void *cookie);

/*** @function vmm_guest_console_start_seen(console)
 * Query recognition of an approved guest-start token.
 * @param console Initialized state.
 * @return Nonzero after `Hello from Unikraft!` or `UNDERLORD_C_FS_FILE: PASS`
 * was observed.
 */
int vmm_guest_console_start_seen(const vmm_guest_console_t *console);
