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

static void hlog_at_level(underlord_log_level_t level, const char *format,
                          va_list args)
{
    log_message("hypervisor", level, format, args);
}

static void vlog_at_level(unsigned int instance_id, underlord_log_level_t level,
                          const char *format, va_list args)
{
    char module[24];

    snprintf(module, sizeof(module), "vmm[%u]", instance_id);
    log_message(module, level, format, args);
}

#define DEFINE_HLOG_HELPER(name, level)                                        \
    void underlord_hlog_##name(const char *format, ...)                         \
    {                                                                            \
        va_list args;                                                            \
        va_start(args, format);                                                  \
        hlog_at_level(level, format, args);                                      \
        va_end(args);                                                            \
    }

#define DEFINE_VLOG_HELPER(name, level)                                        \
    void underlord_vlog_##name(unsigned int instance_id, const char *format, ...) \
    {                                                                            \
        va_list args;                                                            \
        va_start(args, format);                                                  \
        vlog_at_level(instance_id, level, format, args);                         \
        va_end(args);                                                            \
    }

DEFINE_HLOG_HELPER(trace, UNDERLORD_LOG_TRACE)
DEFINE_HLOG_HELPER(debug, UNDERLORD_LOG_DEBUG)
DEFINE_HLOG_HELPER(info, UNDERLORD_LOG_INFO)
DEFINE_HLOG_HELPER(warn, UNDERLORD_LOG_WARN)
DEFINE_HLOG_HELPER(error, UNDERLORD_LOG_ERROR)

DEFINE_VLOG_HELPER(trace, UNDERLORD_LOG_TRACE)
DEFINE_VLOG_HELPER(debug, UNDERLORD_LOG_DEBUG)
DEFINE_VLOG_HELPER(info, UNDERLORD_LOG_INFO)
DEFINE_VLOG_HELPER(warn, UNDERLORD_LOG_WARN)
DEFINE_VLOG_HELPER(error, UNDERLORD_LOG_ERROR)
