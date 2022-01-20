/**
 * @file rte_log.c
 * @author Leon Shan (813475603@qq.com)
 * @brief NOTE: The mutex provided to log may not
 *             be recursive.
 * @version 1.0.0
 * @date 2021-07-27
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "../../inc/middle_layer/rte_log.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>
/**
 * @brief
 *
 */
typedef struct _log_level_tbl_t {
    log_level_t level;
    char *name;
    char *color;
} log_level_tbl_t;

static const log_level_tbl_t log_level_tbl[] = {
    {LOG_LEVEL_FATAL, "F", COL_FATAL},
    {LOG_LEVEL_ERROR, "E", COL_ERROR},
    {LOG_LEVEL_INFO, "I", COL_INFO},
    {LOG_LEVEL_WARN, "W", COL_WARN},
    {LOG_LEVEL_DEBUG, "D", COL_DEBUG},
    {LOG_LEVEL_VERBOSE, "V", COL_VERBOSE},
};

// A mapping from an integer between 0 and 15 to its ASCII character
// equivalent.
static const char * const ascii_map_table = "0123456789abcdef";

/**
 * @brief
 *
 */
typedef struct __log_handle_impl{
    char *filter_tbl[LOG_MAX_FILTER_CNT];
    rte_mutex_t *mutex;
    log_output_f ex_out_func;
    log_get_tick_f get_tick_func;
    uint8_t filter_cnt;
    bool enable;
    log_level_t level;
    log_format_t format;
} log_handle_impl_t;

typedef struct __log_buffer_output {
    uint8_t *buffer;
    uint32_t length;
    uint32_t index;
} log_buffer_output_t;

static log_handle_impl_t log_config_handle = {
    .enable = true,
    .level = LOG_LEVEL_WARN,
    .format = LOG_FMT_DEFAULT,
    .filter_cnt = 0,
    .filter_tbl = {
        NULL
    },
    .mutex = NULL,
    .ex_out_func = NULL,
    .get_tick_func = NULL,
};

#define LOG_LOCK(bank)   rte_lock(log_config_handle.mutex)
#define LOG_UNLOCK(bank) rte_unlock(log_config_handle.mutex)
#define CHAR_IS_NUM(x)   ((x) >= '0' && (x) <= '9')
#define CHAR_TO_NUM(x)   ((x) - '0')

rte_error_t log_init(rte_mutex_t *mutex, log_output_f out_func, log_get_tick_f get_tick_func)
{
    rte_error_t retval = RTE_ERR_UNDEFINE;
    log_config_handle.mutex = mutex;
    log_config_handle.ex_out_func = out_func;
    log_config_handle.get_tick_func = get_tick_func;
    retval = RTE_SUCCESS;
    return retval;
}

static inline size_t log_output_wrapper(log_buffer_output_t *buf_output, uint8_t *data, size_t length)
{
    if (buf_output == NULL) {
        return log_config_handle.ex_out_func(data, length);
    } else {
        if (buf_output->index + length < buf_output->length) {
            memcpy(buf_output->buffer + buf_output->index, data, length);
            buf_output->index += length;
            return length;
        } else {
            if (buf_output->length != buf_output->index) {
                memcpy(buf_output->buffer + buf_output->index, data, buf_output->length - buf_output->index);
                buf_output->buffer[buf_output->length - 1] = ' ';
                buf_output->index = buf_output->length;
                return buf_output->length - buf_output->index;
            }
        }
    }
    return 0;
}

static inline int log_format_parse_num(const char**format)
{
    const char* fmt = *format;
    int n = 0;
    for (; CHAR_IS_NUM(*fmt); ++fmt) {
        n = 10 * n + CHAR_TO_NUM(*fmt);
    }
    *format = fmt;
    return n;
}

static int log_format_output(const char *format, va_list arg, log_buffer_output_t *buf_output)
{
    int converted_count = 0;
    double float_value = 0.0;
    bool if_neg = false, if_long = false, if_left_align = false,
        if_float = false, if_dot_output = false;
    uint32_t pos = 0, extra_pos = 0, width = 0, precision = 0, base = 0, ui32value = 0;
    uint64_t ui64value = 0, index = 0;
    char *str_ptr, temp_buf[32], fill_char = ' ';
    // Loop while there are more characters in the string.
    while(*format) {
        // Find the first non-% character, or the end of the string.
        for(index = 0;
            (format[index] != '%') && (format[index] != '\0');
            index++) {
        }
        // Write this portion of the string.
        converted_count += log_output_wrapper(buf_output, (uint8_t *)format, index);
        // Skip the portion of the string that was written.
        format += index;
        // See if the next character is a %.
        if(*format == '%') {
again:
            // Skip the % or the -.
            format++;
            /* %[flags][width][.precision][length]type */
            // Check [flags]
            switch (*format) {
                case '-':
                    if_left_align = true;
                    goto again;
                case ' ':
                    fill_char = ' ';
                    break;
                case '0':
                    fill_char = '0';
                    break;
                default:
                    format--;
                    break;
            }
            format++;

            // Check [width]
            width = 0;
            if (CHAR_IS_NUM(*format)) {
                width = log_format_parse_num(&format);
            } else if (*format == '*') {
                const int w = (int)va_arg(arg, int);
                if (w < 0) {
                    if_left_align = true;      /* Negative width means left aligned */
                    width = -w;
                } else {
                    width = w;
                }
                ++format;
            }
            if (width > sizeof(temp_buf))
                width = sizeof(temp_buf);
            // Check [.precision]
            precision = 0;
            if (*format == '.') {                      /* Precision flag is detected */
                format++;
                if (CHAR_IS_NUM(*format)) {       /* Directly in the string */
                    precision = log_format_parse_num(&format);
                } else if (*format == '*') {                /* Variable check */
                    const int pr = (int)va_arg(arg, int);
                    precision = pr > 0 ? pr : 0;
                    format++;
                }
            }
            // Check [length]
            switch (*format) {
                case 'l' :
                    if_long = true;
                    format++;
                    break;
                default:
                    break;
            }
            // Determine how to handle the next character.
            switch(*format++) {
                // Handle the %% command.
                case '%': {
                    // Simply write a single %.
                    converted_count += log_output_wrapper(buf_output, (uint8_t *)format - 1, 1);
                    // This command has been handled.
                    break;
                }
                // Handle the %c command.
                case 'c': {
                    // Get the value from the varargs.
                    ui32value = va_arg(arg, int);
                    // Print out the character.
                    converted_count += log_output_wrapper(buf_output, (uint8_t *)&ui32value, 1);
                    // This command has been handled.
                    break;
                }
                // Handle the %s command.
                case 's': {
                    // Get the string pointer from the varargs.
                    str_ptr = va_arg(arg, char *);
                    // Determine the length of the string.
                    for(index = 0; str_ptr[index] != '\0'; index++);
                    // Write any required padding spaces
                    if (width > index &&
                        if_left_align == true) {
                        width -= index;
                        while(width--) {
                            log_output_wrapper(buf_output, (uint8_t *)&fill_char, 1);
                        }
                    }
                    // Write the string.
                    log_output_wrapper(buf_output, (uint8_t *)str_ptr, index);
                    // Write any required padding spaces
                    if(width > index &&
                        if_left_align == false) {
                        width -= index;
                        while(width--) {
                            log_output_wrapper(buf_output, (uint8_t *)&fill_char, 1);
                        }
                    }
                    converted_count += index;
                    // This command has been handled.
                    break;
                }
                // Handle the %d and %i commands.
                case 'd':
                case 'i': {
                    if (if_long) {
                        // Get the value from the varargs.
                        ui64value = va_arg(arg, unsigned long int);
                        // If the value is negative, make it positive and indicate
                        // that a minus sign is needed.
                        if((int64_t)ui64value < 0) {
                            // Make the value positive.
                            ui64value = -(int64_t)ui64value;
                            // Indicate that the value is negative.
                            if_neg = 1;
                        } else {
                            // Indicate that the value is positive so that a minus
                            // sign isn't inserted.
                            if_neg = 0;
                        }
                    } else {
                        // Get the value from the varargs.
                        ui32value = va_arg(arg, unsigned int);
                        // If the value is negative, make it positive and indicate
                        // that a minus sign is needed.
                        if((int32_t)ui32value < 0) {
                            // Make the value positive.
                            ui32value = -(int32_t)ui32value;
                            // Indicate that the value is negative.
                            if_neg = 1;
                        } else {
                            // Indicate that the value is positive so that a minus
                            // sign isn't inserted.
                            if_neg = 0;
                        }
                    }
                    // Reset the buffer position.
                    pos = 0;
                    extra_pos = 0;
                    // Set the base to 10.
                    base = 10;
                    // Convert the value to ASCII.
                    goto convert;
                }
                // Handle the %u command.
                // Handle the %x and %X commands.  Note that they are treated
                // identically; in other words, %X will use lower case letters
                // for a-f instead of the upper case letters it should use.  We
                // also alias %p to %x.
                case 'u':
                case 'x':
                case 'X':
                case 'p': {
                    if (if_long) {
                        // Get the value from the varargs.
                        ui64value = va_arg(arg, unsigned long int);
                    } else {
                        // Get the value from the varargs.
                        ui32value = va_arg(arg, unsigned int);
                    }
                    // Reset the buffer position.
                    pos = 0;
                    extra_pos = 0;
                    // Set the base
                    if (*(format - 1) == 'u') {
                        base = 10;
                    } else if (*(format - 1) == 'x' || *(format - 1) == 'X' || *(format - 1) == 'p') {
                        base = 16;
                    }
                    // Indicate that the value is positive so that a minus sign
                    // isn't inserted.
                    if_neg = 0;
                    goto convert;
                }
                case 'f': {
                    if_long = true;
                    if_float = true;
                    if_dot_output = false;
                    base = 10;
                    float_value = va_arg(arg, double);
                    if (float_value < 0) {
                        if_neg = 1;
                        float_value = -float_value;
                    }
                    // Default precision
                    if (precision == 0) {
                        precision = 6;
                    }
float_again:
                    pos = 0;
                    extra_pos = 0;
                    if (if_dot_output == true) {
                        if_neg = 0;
                        float_value -= ui64value;
                        float_value *= pow(10, precision);
                    }
                    ui64value = (uint64_t)float_value;
convert:
                    memset(temp_buf, 0, sizeof(temp_buf));
                    // Determine the number of digits in the string version of
                    // the value.
                    if (if_long) {
                        for(index = 1;
                            (((index * base) <= ui64value) &&
                            (((index * base) / base) == index));
                            index *= base, width--) {
                        }
                    } else {
                        for(index = 1;
                            (((index * base) <= ui32value) &&
                            (((index * base) / base) == index));
                            index *= base, width--) {
                        }
                    }
                    // If the value is negative, reduce the count of padding
                    // characters needed.
                    if(if_neg)
                        if (width) width--;
                    // If the value is negative and the value is padded with
                    // zeros, then place the minus sign before the padding.
                    if(if_neg && (fill_char == '0')) {
                        // Place the minus sign in the output buffer.
                        temp_buf[pos++] = '-';
                        // The minus sign has been placed, so turn off the
                        // negative flag.
                        if_neg = false;
                    }
                    // Provide additional padding at the beginning of the
                    // string conversion if needed.
                    if ((width > 1) &&
                        (width < sizeof(temp_buf)) &&
                        if_dot_output == false) {
                        if (if_left_align) {
                            for(uint32_t temp_pos = width; width; width--) {
                                temp_buf[temp_pos--] = fill_char;
                                extra_pos++;
                            }
                        } else {
                            for(width--; width; width--) {
                                temp_buf[pos++] = fill_char;
                            }
                        }
                    }
                    // If the value is negative, then place the minus sign
                    // before the number.
                    if(if_neg) {
                        // Place the minus sign in the output buffer.
                        temp_buf[pos++] = '-';
                    }
                    // Convert the value into a string.
                    for(; index; index /= base) {
                        if (if_long) {
                            temp_buf[pos++] = ascii_map_table[(ui64value / index) % base];
                        } else {
                            temp_buf[pos++] = ascii_map_table[(ui32value / index) % base];
                        }
                    }
                    // Write the string.
                    converted_count += log_output_wrapper(buf_output, (uint8_t *)temp_buf, pos + extra_pos);
                    // Judge if it has other part to output when is float
                    if (if_float && if_dot_output == false) {
                        char dot = '.';
                        converted_count += log_output_wrapper(buf_output, (uint8_t *)&dot, 1);
                        if_dot_output = true;
                        goto float_again;
                    }
                    // This command has been handled.
                    break;
                }
                // Handle all other commands.
                default: {
                    // Indicate an error.
                    converted_count += log_output_wrapper(buf_output, (uint8_t *)"ERROR", 5);
                    // This command has been handled.
                    break;
                }
            }
        }
    }
    return converted_count;
}

int log_vprintf(const char *format, va_list arg)
{
    return log_format_output(format, arg, NULL);
}

int log_vsnprintf(char * restrict s, size_t n,
                        const char * restrict format,
                        va_list arg)
{
    log_buffer_output_t output_buffer = {
        .buffer = (uint8_t *)s,
        .length = n,
        .index = 0,
    };
    return log_format_output(format, arg, &output_buffer);
}

int log_sprintf(char * restrict s, const char *format, ...)
{
    va_list arg;
    int ret;
    // Start the varargs processing.
    va_start(arg, format);
    // Call vsnprintf to perform the conversion.  Use a large number for the
    // buffer size.
    ret = log_vsnprintf(s, 0xffff, format, arg);
    // End the varargs processing.
    va_end(arg);
    // Return the conversion count.
    return(ret);
}

int log_snprintf(char * restrict s, size_t n, const char * restrict format, ...)
{
    va_list arg;
    int ret;
    // Start the varargs processing.
    va_start(arg, format);
    // Call vsnprintf to perform the conversion.
    ret = log_vsnprintf(s, n, format, arg);
    // End the varargs processing.
    va_end(arg);
    // Return the conversion count.
    return(ret);
}

void log_sec2time(time_t timetick, struct tm *tm)
{
    // This array contains the number of days in a year at the beginning of each
    // month of the year, in a non-leap year.
    static const time_t g_psDaysToMonth[12] = {
        0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334
    };
    time_t temp, months;
    // Extract the number of seconds, converting time to the number of minutes.
    temp = timetick / 60;
    tm->tm_sec = timetick - (temp * 60);
    timetick = temp;
    // Extract the number of minutes, converting time to the number of hours.
    temp = timetick / 60;
    tm->tm_min = timetick - (temp * 60);
    timetick = temp;
    // Extract the number of hours, converting time to the number of days.
    temp = timetick / 24;
    tm->tm_hour = timetick - (temp * 24);
    timetick = temp;
    // Compute the day of the week.
    tm->tm_wday = (timetick + 4) % 7;
    // Compute the number of leap years that have occurred since 1968, the
    // first leap year before 1970.  For the beginning of a leap year, cut the
    // month loop below at March so that the leap day is classified as February
    // 29 followed by March 1, instead of March 1 followed by another March 1.
    timetick += 366 + 365;
    temp = timetick / ((4 * 365) + 1);
    if((timetick - (temp * ((4 * 365) + 1))) > (31 + 28)) {
        temp++;
        months = 12;
    } else {
        months = 2;
    }
    // Extract the year.
    tm->tm_year = ((timetick - temp) / 365) + 68;
    timetick -= ((tm->tm_year - 68) * 365) + temp;
    // Extract the month.
    for(temp = 0; temp < months; temp++) {
        if(g_psDaysToMonth[temp] > timetick)
            break;
    }
    tm->tm_mon = temp - 1;
    // Extract the day of the month.
    tm->tm_mday = timetick - g_psDaysToMonth[temp - 1] + 1;
}

size_t log_out(log_level_t level, const char *file, const char *function, int line,
                const char *module, const char *msg, ...)
{
    size_t retval = 0;
    char log_head[LOG_MAX_HEAD_LENGTH] = {0};
    size_t offset = 0;
    uint8_t i = 0;
    LOG_LOCK(&log_config_handle);
    if(!log_config_handle.enable || !log_config_handle.ex_out_func)
        goto end;
    /* Check filter */
    for(i = 0; i < log_config_handle.filter_cnt; i++) {
        if(log_config_handle.filter_tbl[i] && strstr(msg, log_config_handle.filter_tbl[i])) {
            goto end;
        }
    }
    if(level <= log_config_handle.level) {
        if(log_config_handle.format & LOG_FMT_TIME) {
            tick_unit_t tick = 0;
            struct tm time = {0};
            if(log_config_handle.get_tick_func) {
                tick = log_config_handle.get_tick_func();
            }
            log_sec2time(tick/1000, &time);
            offset += log_snprintf(log_head + offset, LOG_MAX_HEAD_LENGTH - offset, "%02d:%02d:%02d:%03d",
                                    time.tm_hour, time.tm_min, time.tm_sec, tick % 1000);
        }
        offset += log_snprintf(log_head + offset, LOG_MAX_HEAD_LENGTH - offset, " %s ",
                                log_level_tbl[level].name);
        log_head[offset] = '[';
        offset++;
        if(log_config_handle.format & LOG_FMT_FILE) {
            offset += log_snprintf(log_head + offset, LOG_MAX_HEAD_LENGTH - offset, "%s", file);
            log_head[offset] = ' ';
            offset++;
        }
        if(log_config_handle.format & LOG_FMT_FUNCTION) {
            offset += log_snprintf(log_head + offset, LOG_MAX_HEAD_LENGTH - offset, "%16s", function);
            log_head[offset] = ':';
            offset++;
        }
        if(log_config_handle.format & LOG_FMT_LINE) {
            offset += log_snprintf(log_head + offset, LOG_MAX_HEAD_LENGTH - offset, "%4d", line);
        }
        log_head[offset] = ']';
        offset++;
        offset += log_snprintf(log_head + offset, LOG_MAX_HEAD_LENGTH - offset, " %10s: ", module);
        log_config_handle.ex_out_func((uint8_t *)log_head, offset);
        va_list arg;
        va_start(arg, msg);
        offset += log_vprintf(msg, arg);
        va_end(arg);
        log_config_handle.ex_out_func((uint8_t *)"\r\n", 2);
        offset += 2;
        retval += offset;
    }
end:
    LOG_UNLOCK(&log_config_handle);
    return retval;
}

rte_error_t log_control(log_command_t command, void *param)
{
    rte_error_t retval = RTE_ERR_UNDEFINE;
    uint8_t i = 0;
    LOG_LOCK(&log_config_handle);
    switch(command) {
    case LOG_CMD_ENABLE:
        RTE_UNUSED(param);
        log_config_handle.enable = true;
        break;
    case LOG_CMD_DISABLE:
        RTE_UNUSED(param);
        log_config_handle.enable = false;
        break;
    case LOG_CMD_LEVEL_UP:
        if(!param) {
            retval = RTE_ERR_PARAM;
            break;
        }
        log_config_handle.level++;
        if(log_config_handle.level > LOG_LEVEL_VERBOSE)
            log_config_handle.level = LOG_LEVEL_VERBOSE;
        *((log_level_t *)param) = log_config_handle.level;
        break;
    case LOG_CMD_LEVEL_DOWN:
        if(!param) {
            retval = RTE_ERR_PARAM;
            break;
        }
        if(log_config_handle.level >= LOG_LEVEL_ERROR)
            log_config_handle.level--;
        *((log_level_t *)param) = log_config_handle.level;
        break;
    case LOG_CMD_SET_LEVEL:
        if(!param) {
            retval = RTE_ERR_PARAM;
            break;
        }
        log_config_handle.level = *((log_level_t *)param);
        break;
    case LOG_CMD_GET_LEVEL:
        if(!param) {
            retval = RTE_ERR_PARAM;
            break;
        }
        *((log_level_t *)param) = log_config_handle.level;
        break;
    case LOG_CMD_SET_FORMAT:
        if(!param) {
            retval = RTE_ERR_PARAM;
            break;
        }
        log_config_handle.format = *((log_format_t *)param);
        break;
    case LOG_CMD_GET_FORMAT:
        if(!param) {
            retval = RTE_ERR_PARAM;
            break;
        }
        *((log_format_t *)param) = log_config_handle.format;
        break;
    case LOG_CMD_ADD_FILTER:
        if(!param) {
            retval = RTE_ERR_PARAM;
            break;
        }
        if(log_config_handle.filter_cnt >= LOG_MAX_FILTER_CNT) {
            retval = RTE_ERR_NO_RSRC;
            break;
        }
        log_config_handle.filter_tbl[log_config_handle.filter_cnt++] = param;
        break;
    case LOG_CMD_RMV_FILTER:
        if(!param) {
            retval = RTE_ERR_PARAM;
            break;
        }
        for(i = 0; i < log_config_handle.filter_cnt; i++) {
            if(!strcmp(log_config_handle.filter_tbl[i], (char *)param)) {
                log_config_handle.filter_tbl[i] = NULL;
                memmove(&log_config_handle.filter_tbl[i], &log_config_handle.filter_tbl[i + 1],
                        (log_config_handle.filter_cnt - i - 1));
                log_config_handle.filter_cnt--;
                break;
            }
        }
        break;
    case LOG_CMD_SET_OUTPUT:
        if(!param)
            break;
        log_config_handle.ex_out_func = param;
        break;
    default:
        break;
    }
    LOG_UNLOCK(&log_config_handle);
    return retval;
}
/**
 * @brief Output some text to the log output.
 *
 * @param buffer
 */
void log_output(const char *buffer, size_t length)
{
    log_config_handle.ex_out_func((uint8_t *)buffer, length);
}
