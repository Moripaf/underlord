#include <stdarg.h>
#include <stdio.h>

#include <underlord/log_format.h>

const char *underlord_log_level_name(underlord_log_level_t level)
{
    static const char *const names[] = {
        [UNDERLORD_LOG_TRACE] = "TRACE", [UNDERLORD_LOG_DEBUG] = "DEBUG",
        [UNDERLORD_LOG_INFO] = "INFO", [UNDERLORD_LOG_WARN] = "WARN",
        [UNDERLORD_LOG_ERROR] = "ERROR",
    };

    return level <= UNDERLORD_LOG_ERROR ? names[level] : "UNKNOWN";
}

int underlord_format_log_v(char *buffer, size_t buffer_size,
                           const char *module, underlord_log_level_t level,
                           const char *format, va_list args)
{
    int prefix_length;

    if (buffer == NULL || buffer_size == 0 || module == NULL || format == NULL) {
        return -1;
    }
    prefix_length = snprintf(buffer, buffer_size, "[%s] %s: ",
                             underlord_log_level_name(level), module);
    if (prefix_length < 0 || (size_t)prefix_length >= buffer_size) {
        buffer[buffer_size - 1] = '\0';
        return -1;
    }
    if (vsnprintf(buffer + prefix_length, buffer_size - (size_t)prefix_length,
                  format, args) < 0) {
        buffer[0] = '\0';
        return -1;
    }
    buffer[buffer_size - 1] = '\0';
    return 0;
}

int underlord_format_log(char *buffer, size_t buffer_size,
                         const char *module, underlord_log_level_t level,
                         const char *format, ...)
{
    va_list args;
    int result;

    va_start(args, format);
    result = underlord_format_log_v(buffer, buffer_size, module, level, format,
                                    args);
    va_end(args);
    return result;
}
