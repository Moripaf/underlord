#include <assert.h>
#include <string.h>

#include <vmm_guest_console.h>

static char output[VMM_GUEST_CONSOLE_LINE_MAX];
static void capture(const char *line, void *cookie)
{
    (void)cookie;
    strncpy(output, line, sizeof(output));
    output[sizeof(output) - 1U] = '\0';
}

int main(void)
{
    vmm_guest_console_t console;
    vmm_guest_console_init(&console);
    assert(vmm_guest_console_feed(&console, "Hello from Uni", 14, capture, NULL) == 0);
    assert(!vmm_guest_console_hello_seen(&console));
    assert(vmm_guest_console_feed(&console, "kraft!\r\n", 8, capture, NULL) == 0);
    assert(vmm_guest_console_hello_seen(&console));
    assert(strcmp(output, "") == 0);
    assert(vmm_guest_console_feed(&console, "x\ny\r", 4, capture, NULL) == 0);
    assert(strcmp(output, "y") == 0);
    assert(vmm_guest_console_feed(&console, "%s\n", 3, capture, NULL) == 0);
    assert(strcmp(output, "%s") == 0);
    return 0;
}
