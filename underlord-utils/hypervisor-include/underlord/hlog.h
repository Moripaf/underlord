#pragma once

/***
 * @file hlog.h
 * Private hypervisor logging facade with the fixed "hypervisor" identity.
 */

#include <underlord/vlog.h>

/***
 * @function underlord_hlog_trace(format, ...)
 * Emit a TRACE record for the hypervisor.
 * @param {const char *} format Non-NULL printf-style format and arguments.
 * @pre The target printf backend is initialized.
 * @return None.
 * @sideeffect Emits one newline-terminated diagnostic record.
 * @error Formatting failures suppress the record.
 */
void underlord_hlog_trace(const char *format, ...)
    __attribute__((format(printf, 1, 2)));
/***
 * @function underlord_hlog_debug(format, ...)
 * Emit a DEBUG hypervisor record.
 * @param {const char *} format Non-NULL printf-style format and arguments.
 * @pre The target printf backend is initialized.
 * @return None.
 * @sideeffect Emits one newline-terminated diagnostic record.
 * @error Formatting failures suppress the record.
 */
void underlord_hlog_debug(const char *format, ...)
    __attribute__((format(printf, 1, 2)));
/***
 * @function underlord_hlog_info(format, ...)
 * Emit an INFO hypervisor record.
 * @param {const char *} format Non-NULL printf-style format and arguments.
 * @pre The target printf backend is initialized.
 * @return None.
 * @sideeffect Emits one newline-terminated diagnostic record.
 * @error Formatting failures suppress the record.
 */
void underlord_hlog_info(const char *format, ...)
    __attribute__((format(printf, 1, 2)));
/***
 * @function underlord_hlog_warn(format, ...)
 * Emit a WARN hypervisor record.
 * @param {const char *} format Non-NULL printf-style format and arguments.
 * @pre The target printf backend is initialized.
 * @return None.
 * @sideeffect Emits one newline-terminated diagnostic record.
 * @error Formatting failures suppress the record.
 */
void underlord_hlog_warn(const char *format, ...)
    __attribute__((format(printf, 1, 2)));
/***
 * @function underlord_hlog_error(format, ...)
 * Emit an ERROR hypervisor record.
 * @param {const char *} format Non-NULL printf-style format and arguments.
 * @pre The target printf backend is initialized.
 * @return None.
 * @sideeffect Emits one newline-terminated diagnostic record.
 * @error Formatting failures suppress the record.
 */
void underlord_hlog_error(const char *format, ...)
    __attribute__((format(printf, 1, 2)));
