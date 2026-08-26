#include <assert.h>
#include <string.h>

#include <underlord/log_format.h>

int main(void)
{
    char buffer[32];

    assert(strcmp(underlord_log_level_name(UNDERLORD_LOG_INFO), "INFO") == 0);
    assert(strcmp(underlord_log_level_name((underlord_log_level_t)99), "UNKNOWN") == 0);
    assert(underlord_format_log(buffer, sizeof(buffer), "vmm[0]",
                                UNDERLORD_LOG_INFO, "%s", "started") == 0);
    assert(strcmp(buffer, "[INFO] vmm[0]: started") == 0);
    assert(underlord_format_log(buffer, sizeof(buffer), "module",
                                UNDERLORD_LOG_WARN, "%s", "abcdefghijklmnopqrstuvwxyz") == 0);
    assert(buffer[sizeof(buffer) - 1] == '\0');
    assert(underlord_format_log(NULL, sizeof(buffer), "module",
                                UNDERLORD_LOG_INFO, "x") == -1);
    return 0;
}
