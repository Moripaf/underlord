#include <stdarg.h>
#include <stdio.h>

#include <underlord/hlog.h>

static const char *log_level_name(underlord_log_level_t level)
{
    static const char *const names[] = {
        [UNDERLORD_LOG_TRACE] = "TRACE",
        [UNDERLORD_LOG_DEBUG] = "DEBUG",
        [UNDERLORD_LOG_INFO] = "INFO",
        [UNDERLORD_LOG_WARN] = "WARN",
        [UNDERLORD_LOG_ERROR] = "ERROR",
    };

    return level <= UNDERLORD_LOG_ERROR ? names[level] : "UNKNOWN";
}

static void log_message(const char *module, underlord_log_level_t level,
                        const char *format, va_list args)
{
    char message[256];
    int prefix_length = snprintf(message, sizeof(message), "[%s] %s: ",
                                 log_level_name(level), module);
    if (prefix_length < 0) {
        return;
    }

    size_t offset = (size_t)prefix_length;
    if (offset >= sizeof(message)) {
        offset = sizeof(message) - 1;
    }
    vsnprintf(message + offset, sizeof(message) - offset, format, args);
    printf("%s\n", message);
}

void underlord_hlog(underlord_log_level_t level, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_message("hypervisor", level, format, args);
    va_end(args);
}

void underlord_vlog(unsigned int instance_id, underlord_log_level_t level,
                    const char *format, ...)
{
    char module[24];
    va_list args;

    snprintf(module, sizeof(module), "vmm[%u]", instance_id);
    va_start(args, format);
    log_message(module, level, format, args);
    va_end(args);
}
