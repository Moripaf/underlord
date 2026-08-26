#pragma once

#include <underlord/vlog.h>

/* Hypervisor-only logging interface; this header is not exposed to VMMs. */
void underlord_hlog(underlord_log_level_t level, const char *format, ...)
    __attribute__((format(printf, 2, 3)));
