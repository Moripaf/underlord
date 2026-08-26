#pragma once

/***
 * @file log_format.h
 * Bounded, target-independent formatting for Underlord log records.
 */

#include <stdarg.h>
#include <stddef.h>

#include <underlord/vlog.h>

/***
 * @function underlord_log_level_name(level)
 * Obtain the stable printable name for a logging level.
 * @param {underlord_log_level_t} level Requested severity.
 * @pre None.
 * @return Static name for a known level, otherwise "UNKNOWN".
 * @sideeffect None.
 * @error Invalid levels are represented as "UNKNOWN".
 */
const char *underlord_log_level_name(underlord_log_level_t level);
/***
 * @function underlord_format_log_v(buffer, buffer_size, module, level, format, args)
 * Format one bounded log record using an existing varargs list.
 * @param {char *} buffer Destination storage; must not be NULL when size is non-zero.
 * @param {size_t} buffer_size Destination capacity including the terminator.
 * @param {const char *} module Non-NULL module prefix.
 * @param {underlord_log_level_t} level Record severity.
 * @param {const char *} format Non-NULL printf-style format.
 * @param {va_list} args Arguments consumed by format.
 * @pre buffer_size is non-zero and all pointer inputs are valid.
 * @return 0 on formatting success; -1 for invalid inputs or formatting failure.
 * @sideeffect Writes a NUL-terminated, possibly truncated record to buffer.
 * @error On error, buffer is empty or safely terminated when possible.
 */
int underlord_format_log_v(char *buffer, size_t buffer_size,
                           const char *module, underlord_log_level_t level,
                           const char *format, va_list args);
/***
 * @function underlord_format_log(buffer, buffer_size, module, level, format, ...)
 * Varargs wrapper for underlord_format_log_v().
 * @param {char *} buffer Destination record storage.
 * @param {size_t} buffer_size Destination capacity including the terminator.
 * @param {const char *} module Non-NULL module prefix.
 * @param {underlord_log_level_t} level Record severity.
 * @param {const char *} format Non-NULL printf-style format and arguments.
 * @pre Same validity requirements as underlord_format_log_v().
 * @return 0 on success; -1 on invalid input or formatting failure.
 * @sideeffect Writes a bounded NUL-terminated record to buffer.
 * @error See underlord_format_log_v().
 */
int underlord_format_log(char *buffer, size_t buffer_size,
                         const char *module, underlord_log_level_t level,
                         const char *format, ...)
    __attribute__((format(printf, 5, 6)));
