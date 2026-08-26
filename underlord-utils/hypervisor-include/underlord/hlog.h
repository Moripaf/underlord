#pragma once

#include <underlord/vlog.h>

/* Hypervisor-only logging interface; this header is not exposed to VMMs. */
void underlord_hlog_trace(const char *format, ...)
    __attribute__((format(printf, 1, 2)));
void underlord_hlog_debug(const char *format, ...)
    __attribute__((format(printf, 1, 2)));
void underlord_hlog_info(const char *format, ...)
    __attribute__((format(printf, 1, 2)));
void underlord_hlog_warn(const char *format, ...)
    __attribute__((format(printf, 1, 2)));
void underlord_hlog_error(const char *format, ...)
    __attribute__((format(printf, 1, 2)));
