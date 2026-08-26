#pragma once

/***
 * @file vlog.h
 * Public VMM logging interface with a caller-supplied instance identity.
 */

/***
 * @enum underlord_log_level_t
 * Supported record severities, ordered from least to most severe.
 */
typedef enum {
    UNDERLORD_LOG_TRACE,
    UNDERLORD_LOG_DEBUG,
    UNDERLORD_LOG_INFO,
    UNDERLORD_LOG_WARN,
    UNDERLORD_LOG_ERROR,
} underlord_log_level_t;

/***
 * @function underlord_vlog_trace(instance_id, format, ...)
 * Emit a TRACE record under the vmm[instance_id] module name.
 * @param {unsigned int} instance_id VMM identity included in the prefix.
 * @param {const char *} format Non-NULL printf-style format and arguments.
 * @pre The target printf backend is initialized.
 * @return None.
 * @sideeffect Emits one newline-terminated diagnostic record.
 * @error Formatting failures suppress the record.
 */
void underlord_vlog_trace(unsigned int instance_id, const char *format, ...)
    __attribute__((format(printf, 2, 3)));
/***
 * @function underlord_vlog_debug(instance_id, format, ...)
 * Emit a DEBUG VMM record.
 * @param {unsigned int} instance_id VMM identity.
 * @param {const char *} format Non-NULL format and arguments.
 * @pre The target printf backend is initialized.
 * @return None.
 * @sideeffect Emits one record.
 * @error Formatting failures suppress the record.
 */
void underlord_vlog_debug(unsigned int instance_id, const char *format, ...)
    __attribute__((format(printf, 2, 3)));
/***
 * @function underlord_vlog_info(instance_id, format, ...)
 * Emit an INFO VMM record.
 * @param {unsigned int} instance_id VMM identity.
 * @param {const char *} format Non-NULL format and arguments.
 * @pre The target printf backend is initialized.
 * @return None.
 * @sideeffect Emits one record.
 * @error Formatting failures suppress the record.
 */
void underlord_vlog_info(unsigned int instance_id, const char *format, ...)
    __attribute__((format(printf, 2, 3)));
/***
 * @function underlord_vlog_warn(instance_id, format, ...)
 * Emit a WARN VMM record.
 * @param {unsigned int} instance_id VMM identity.
 * @param {const char *} format Non-NULL format and arguments.
 * @pre The target printf backend is initialized.
 * @return None.
 * @sideeffect Emits one record.
 * @error Formatting failures suppress the record.
 */
void underlord_vlog_warn(unsigned int instance_id, const char *format, ...)
    __attribute__((format(printf, 2, 3)));
/***
 * @function underlord_vlog_error(instance_id, format, ...)
 * Emit an ERROR VMM record.
 * @param {unsigned int} instance_id VMM identity.
 * @param {const char *} format Non-NULL format and arguments.
 * @pre The target printf backend is initialized.
 * @return None.
 * @sideeffect Emits one record.
 * @error Formatting failures suppress the record.
 */
void underlord_vlog_error(unsigned int instance_id, const char *format, ...)
    __attribute__((format(printf, 2, 3)));
