#pragma once

typedef enum {
    UNDERLORD_LOG_TRACE,
    UNDERLORD_LOG_DEBUG,
    UNDERLORD_LOG_INFO,
    UNDERLORD_LOG_WARN,
    UNDERLORD_LOG_ERROR,
} underlord_log_level_t;

/* Public VMM logging interface. Each helper supplies the vmm[instance] name. */
void underlord_vlog_trace(unsigned int instance_id, const char *format, ...)
    __attribute__((format(printf, 2, 3)));
void underlord_vlog_debug(unsigned int instance_id, const char *format, ...)
    __attribute__((format(printf, 2, 3)));
void underlord_vlog_info(unsigned int instance_id, const char *format, ...)
    __attribute__((format(printf, 2, 3)));
void underlord_vlog_warn(unsigned int instance_id, const char *format, ...)
    __attribute__((format(printf, 2, 3)));
void underlord_vlog_error(unsigned int instance_id, const char *format, ...)
    __attribute__((format(printf, 2, 3)));
