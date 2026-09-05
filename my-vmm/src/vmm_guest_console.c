#include <string.h>

#include <vmm_guest_console.h>

static const char hello[] = "Hello from Unikraft!";

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
        if (!console->hello_seen) {
            if (byte == (unsigned char)hello[console->hello_progress]) {
                console->hello_progress++;
                if (console->hello_progress == sizeof(hello) - 1U)
                    console->hello_seen = 1;
            } else {
                console->hello_progress = byte == (unsigned char)hello[0] ? 1U : 0U;
            }
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

int vmm_guest_console_hello_seen(const vmm_guest_console_t *console)
{
    return console != NULL && console->hello_seen;
}
