#pragma once

typedef enum {
    UNDERLORD_LOG_TRACE,
    UNDERLORD_LOG_DEBUG,
    UNDERLORD_LOG_INFO,
    UNDERLORD_LOG_WARN,
    UNDERLORD_LOG_ERROR,
} underlord_log_level_t;

/* Public VMM logging interface. The helper supplies the vmm[instance] name. */
void underlord_vlog(unsigned int instance_id, underlord_log_level_t level,
                    const char *format, ...)
    __attribute__((format(printf, 3, 4)));
