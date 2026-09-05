#include <string.h>

#include <vmm_guest_console.h>

static const char hello[] = "Hello from Unikraft!";
static const char c_fs[] = "UNDERLORD_C_FS_FILE: PASS";

static size_t token_progress(const char *token, size_t progress,
                             unsigned char byte)
{
    if (byte == (unsigned char)token[progress]) return progress + 1U;
    return byte == (unsigned char)token[0] ? 1U : 0U;
}

void vmm_guest_console_init(vmm_guest_console_t *console)
{
    if (console != NULL) memset(console, 0, sizeof(*console));
}

static void finish_line(vmm_guest_console_t *console,
                        void (*emit)(const char *, void *), void *cookie)
{
    console->line[console->length] = '\0';
    if (emit != NULL) emit(console->line, cookie);
    console->length = 0;
}

int vmm_guest_console_feed(vmm_guest_console_t *console, const char *bytes,
                           size_t length, void (*emit)(const char *, void *),
                           void *cookie)
{
    if (console == NULL || (bytes == NULL && length != 0)) return -1;
    for (size_t i = 0; i < length; i++) {
        unsigned char byte = (unsigned char)bytes[i];
        if (byte == '\n') {
            finish_line(console, emit, cookie);
            console->saw_cr = 0;
            continue;
        }
        if (byte == '\r') {
            finish_line(console, emit, cookie);
            console->saw_cr = 1;
            continue;
        }
        console->saw_cr = 0;
        if (!console->guest_started) {
            console->hello_progress = token_progress(hello, console->hello_progress, byte);
            console->c_fs_progress = token_progress(c_fs, console->c_fs_progress, byte);
            if (console->hello_progress == sizeof(hello) - 1U ||
                console->c_fs_progress == sizeof(c_fs) - 1U)
                console->guest_started = 1;
        }
        if (console->length < VMM_GUEST_CONSOLE_LINE_MAX - 1U) {
            console->line[console->length++] = (char)byte;
        } else {
            finish_line(console, emit, cookie);
            console->line[console->length++] = (char)byte;
        }
    }
    return 0;
}

int vmm_guest_console_start_seen(const vmm_guest_console_t *console)
{
    return console != NULL && console->guest_started;
}
