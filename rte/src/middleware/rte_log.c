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

/**
 * @brief
 *
 */
typedef struct __log_handle_impl{
    bool enable;
    log_level_t level;
    log_format_t format;
    uint8_t filter_cnt;
    char *filter_tbl[LOG_MAX_FILTER_CNT];
    rte_mutex_t *mutex;
    log_output_f out_func;
    log_get_tick_f get_tick_func;
} log_handle_impl_t;

static log_handle_impl_t log_config_handle = {
    .enable = true,
    .level = LOG_LEVEL_WARN,
    .format = LOG_FMT_DEFAULT,
    .filter_cnt = 0,
    .filter_tbl = {
        NULL
    },
    .mutex = NULL,
    .out_func = NULL,
    .get_tick_func = NULL,
};

#define LOG_LOCK(bank)   RTE_LOCK(log_config_handle.mutex)
#define LOG_UNLOCK(bank) RTE_UNLOCK(log_config_handle.mutex)

rte_error_t log_init(rte_mutex_t *mutex, log_output_f out_func, log_get_tick_f get_tick_func)
{
    rte_error_t retval = RTE_ERR_UNDEFINE;
    log_config_handle.mutex = mutex;
    log_config_handle.out_func = out_func;
    log_config_handle.get_tick_func = get_tick_func;
    retval = RTE_SUCCESS;
    return retval;
}

static int log_vsnprintf(char * restrict s, size_t n,
                    const char * restrict format,
                    va_list arg)
{
    // A mapping from an integer between 0 and 15 to its ASCII character
    // equivalent.
    static const char * const g_pcHex = "0123456789abcdef";
    unsigned long ulIdx, ulValue, ulCount, ulBase, ulNeg;
    char *pcStr, cFill;
    int iConvertCount = 0;
    // Adjust buffer size limit to allow one space for null termination.
    if(n)
        n--;
    // Initialize the count of characters converted.
    iConvertCount = 0;
    // Loop while there are more characters in the format string.
    while(*format) {
        // Find the first non-% character, or the end of the string.
        for(ulIdx = 0; (format[ulIdx] != '%') && (format[ulIdx] != '\0');
            ulIdx++) {
        }
        // Write this portion of the string to the output buffer.  If there are
        // more characters to write than there is space in the buffer, then
        // only write as much as will fit in the buffer.
        if(ulIdx > n) {
            strncpy(s, format, n);
            s += n;
            n = 0;
        } else {
            strncpy(s, format, ulIdx);
            s += ulIdx;
            n -= ulIdx;
        }
        // Update the conversion count.  This will be the number of characters
        // that should have been written, even if there was not room in the
        // buffer.
        iConvertCount += ulIdx;
        // Skip the portion of the format string that was written.
        format += ulIdx;
        // See if the next character is a %.
        if(*format == '%') {
            // Skip the %.
            format++;
            // Set the digit count to zero, and the fill character to space
            // (that is, to the defaults).
            ulCount = 0;
            cFill = ' ';
            // It may be necessary to get back here to process more characters.
            // Goto's aren't pretty, but effective.  I feel extremely dirty for
            // using not one but two of the beasts.
again:
            // Determine how to handle the next character.
            switch(*format++) {
            // Handle the digit characters.
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': {
                // If this is a zero, and it is the first digit, then the
                // fill character is a zero instead of a space.
                if((format[-1] == '0') && (ulCount == 0))
                    cFill = '0';
                // Update the digit count.
                ulCount *= 10;
                ulCount += format[-1] - '0';
                // Get the next character.
                goto again;
            }
            // Handle the %c command.
            case 'c': {
                // Get the value from the varargs.
                ulValue = va_arg(arg, unsigned long);
                // Copy the character to the output buffer, if there is
                // room.  Update the buffer size remaining.
                if(n != 0) {
                    *s++ = (char)ulValue;
                    n--;
                }
                // Update the conversion count.
                iConvertCount++;
                // This command has been handled.
                break;
            }
            // Handle the %d and %i commands.
            case 'd':
            case 'i': {
                // Get the value from the varargs.
                ulValue = va_arg(arg, unsigned long);
                // If the value is negative, make it positive and indicate
                // that a minus sign is needed.
                if((long)ulValue < 0) {
                    // Make the value positive.
                    ulValue = -(long)ulValue;
                    // Indicate that the value is negative.
                    ulNeg = 1;
                } else {
                    // Indicate that the value is positive so that a
                    // negative sign isn't inserted.
                    ulNeg = 0;
                }
                // Set the base to 10.
                ulBase = 10;
                // Convert the value to ASCII.
                goto convert;
            }
            // Handle the %s command.
            case 's': {
                // Get the string pointer from the varargs.
                pcStr = va_arg(arg, char *);
                // Determine the length of the string.
                for(ulIdx = 0; pcStr[ulIdx] != '\0'; ulIdx++) {
                }
                // Update the convert count to include any padding that
                // should be necessary (regardless of whether we have space
                // to write it or not).
                if(ulCount > ulIdx)
                    iConvertCount += (ulCount - ulIdx);
                // Copy the string to the output buffer.  Only copy as much
                // as will fit in the buffer.  Update the output buffer
                // pointer and the space remaining.
                if(ulIdx > n) {
                    strncpy(s, pcStr, n);
                    s += n;
                    n = 0;
                } else {
                    strncpy(s, pcStr, ulIdx);
                    s += ulIdx;
                    n -= ulIdx;
                    // Write any required padding spaces assuming there is
                    // still space in the buffer.
                    if(ulCount > ulIdx) {
                        ulCount -= ulIdx;
                        if(ulCount > n)
                            ulCount = n;
                        n = -ulCount;
                        while(ulCount--)
                            *s++ = ' ';
                    }
                }
                // Update the conversion count.  This will be the number of
                // characters that should have been written, even if there
                // was not room in the buffer.
                iConvertCount += ulIdx;
                // This command has been handled.
                break;
            }
            // Handle the %u command.
            case 'u': {
                // Get the value from the varargs.
                ulValue = va_arg(arg, unsigned long);
                // Set the base to 10.
                ulBase = 10;
                // Indicate that the value is positive so that a minus sign
                // isn't inserted.
                ulNeg = 0;
                // Convert the value to ASCII.
                goto convert;
            }
            // Handle the %x and %X commands.  Note that they are treated
            // identically; that is, %X will use lower case letters for a-f
            // instead of the upper case letters is should use.  We also
            // alias %p to %x.
            case 'x':
            case 'X':
            case 'p': {
                // Get the value from the varargs.
                ulValue = va_arg(arg, unsigned long);
                // Set the base to 16.
                ulBase = 16;
                // Indicate that the value is positive so that a minus sign
                // isn't inserted.
                ulNeg = 0;
                // Determine the number of digits in the string version of
                // the value.
convert:
                for(ulIdx = 1;
                    (((ulIdx * ulBase) <= ulValue) &&
                        (((ulIdx * ulBase) / ulBase) == ulIdx));
                    ulIdx *= ulBase, ulCount--) {
                }
                // If the value is negative, reduce the count of padding
                // characters needed.
                if(ulNeg)
                    ulCount--;
                // If the value is negative and the value is padded with
                // zeros, then place the minus sign before the padding.
                if(ulNeg && (n != 0) && (cFill == '0')) {
                    // Place the minus sign in the output buffer.
                    *s++ = '-';
                    n--;
                    // Update the conversion count.
                    iConvertCount++;
                    // The minus sign has been placed, so turn off the
                    // negative flag.
                    ulNeg = 0;
                }
                // See if there are more characters in the specified field
                // width than there are in the conversion of this value.
                if((ulCount > 1) && (ulCount < 65536)) {
                    // Loop through the required padding characters.
                    for(ulCount--; ulCount; ulCount--) {
                        // Copy the character to the output buffer if there
                        // is room.
                        if(n != 0) {
                            *s++ = cFill;
                            n--;
                        }
                        // Update the conversion count.
                        iConvertCount++;
                    }
                }
                // If the value is negative, then place the minus sign
                // before the number.
                if(ulNeg && (n != 0)) {
                    // Place the minus sign in the output buffer.
                    *s++ = '-';
                    n--;
                    // Update the conversion count.
                    iConvertCount++;
                }
                // Convert the value into a string.
                for(; ulIdx; ulIdx /= ulBase) {
                    // Copy the character to the output buffer if there is
                    // room.
                    if(n != 0) {
                        *s++ = g_pcHex[(ulValue / ulIdx) % ulBase];
                        n--;
                    }
                    // Update the conversion count.
                    iConvertCount++;
                }
                // This command has been handled.
                break;
            }
            // Handle the %% command.
            case '%': {
                // Simply write a single %.
                if(n != 0) {
                    *s++ = format[-1];
                    n--;
                }
                // Update the conversion count.
                iConvertCount++;
                // This command has been handled.
                break;
            }
            // Handle all other commands.
            default: {
                // Indicate an error.
                if(n >= 5) {
                    strncpy(s, "ERRO", 5);
                    s += 5;
                    n -= 5;
                } else {
                    strncpy(s, "ERRO", n);
                    s += n;
                    n = 0;
                }
                // Update the conversion count.
                iConvertCount += 5;
                // This command has been handled.
                break;
            }
            }
        }
    }
    // Null terminate the string in the buffer.
    *s = 0;
    // Return the number of characters in the full converted string.
    return(iConvertCount);
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

int log_vprintf(const char *pcString, va_list vaArgP)
{
    int iConvertCount = 0;
    bool if_neg = false, if_long = false;
    uint32_t pos = 0, count = 0, base = 0, ui32value = 0;
    uint64_t ui64value = 0, index = 0;
    static const char * const g_pcHex = "0123456789abcdef";
    char *pcStr, pcBuf[16], cFill;
    // Loop while there are more characters in the string.
    while(*pcString) {
        // Find the first non-% character, or the end of the string.
        for(index = 0;
            (pcString[index] != '%') && (pcString[index] != '\0');
            index++) {
        }
        // Write this portion of the string.
        log_config_handle.out_func((uint8_t *)pcString, index);
        iConvertCount += index;
        // Skip the portion of the string that was written.
        pcString += index;
        // See if the next character is a %.
        if(*pcString == '%') {
            // Skip the %.
            pcString++;
            // Set the digit count to zero, and the fill character to space
            // (in other words, to the defaults).
            count = 0;
            cFill = ' ';
            // It may be necessary to get back here to process more characters.
            // Goto's aren't pretty, but effective.  I feel extremely dirty for
            // using not one but two of the beasts.
again:
            // Determine how to handle the next character.
            switch(*pcString++) {
                // Handle the %% command.
                case '%': {
                    // Simply write a single %.
                    log_config_handle.out_func((uint8_t *)pcString - 1, 1);
                    iConvertCount += 1;
                    // This command has been handled.
                    break;
                }
                // Handle the %c command.
                case 'c': {
                    // Get the value from the varargs.
                    ui32value = va_arg(vaArgP, uint32_t);
                    // Print out the character.
                    log_config_handle.out_func((uint8_t *)&ui32value, 1);
                    iConvertCount += 1;
                    // This command has been handled.
                    break;
                }
                // Handle the %s command.
                case 's': {
                    // Get the string pointer from the varargs.
                    pcStr = va_arg(vaArgP, char *);
                    // Determine the length of the string.
                    for(index = 0; pcStr[index] != '\0'; index++) {
                    }
                    // Write the string.
                    log_config_handle.out_func((uint8_t *)pcStr, index);
                    // Write any required padding spaces
                    if(count > index) {
                        count -= index;
                        while(count--) {
                            log_config_handle.out_func((uint8_t *)" ", 1);
                        }
                    }
                    iConvertCount += index;
                    // This command has been handled.
                    break;
                }
                // Handle the digit characters.
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9': {
                    // If this is a zero, and it is the first digit, then the
                    // fill character is a zero instead of a space.
                    if((pcString[-1] == '0') && (count == 0)) {
                        cFill = '0';
                    }
                    // Update the digit count.
                    count *= 10;
                    count += pcString[-1] - '0';
                    // Get the next character.
                    goto again;
                }
                // Handle the %l[] commands.
                case 'l': {
                    if_long = true;
                    goto again;
                }
                // Handle the %d and %i commands.
                case 'd':
                case 'i': {
                    if (if_long) {
                        // Get the value from the varargs.
                        ui64value = va_arg(vaArgP, uint64_t);
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
                        ui32value = va_arg(vaArgP, uint32_t);
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
                    // Set the base to 10.
                    base = 10;
                    // Convert the value to ASCII.
                    goto convert;
                }
                // Handle the %u command.
                case 'u': {
                    if (if_long) {
                        // Get the value from the varargs.
                        ui64value = va_arg(vaArgP, uint64_t);
                    } else {
                        // Get the value from the varargs.
                        ui32value = va_arg(vaArgP, uint32_t);
                    }
                    // Reset the buffer position.
                    pos = 0;
                    // Set the base to 10.
                    base = 10;
                    // Indicate that the value is positive so that a minus sign
                    // isn't inserted.
                    if_neg = 0;
                    // Convert the value to ASCII.
                    goto convert;
                }
                // Handle the %x and %X commands.  Note that they are treated
                // identically; in other words, %X will use lower case letters
                // for a-f instead of the upper case letters it should use.  We
                // also alias %p to %x.
                case 'x':
                case 'X':
                case 'p': {
                    if (if_long) {
                        // Get the value from the varargs.
                        ui64value = va_arg(vaArgP, uint64_t);
                    } else {
                        // Get the value from the varargs.
                        ui32value = va_arg(vaArgP, uint32_t);
                    }
                    // Reset the buffer position.
                    pos = 0;
                    // Set the base to 16.
                    base = 16;
                    // Indicate that the value is positive so that a minus sign
                    // isn't inserted.
                    if_neg = 0;
                    // Determine the number of digits in the string version of
                    // the value.
convert:
                    if (if_long) {
                        for(index = 1;
                            (((index * base) <= ui64value) &&
                            (((index * base) / base) == index));
                            index *= base, count--) {
                        }
                    } else {
                        for(index = 1;
                            (((index * base) <= ui32value) &&
                            (((index * base) / base) == index));
                            index *= base, count--) {
                        }
                    }
                    // If the value is negative, reduce the count of padding
                    // characters needed.
                    if(if_neg)
                        count--;
                    // If the value is negative and the value is padded with
                    // zeros, then place the minus sign before the padding.
                    if(if_neg && (cFill == '0')) {
                        // Place the minus sign in the output buffer.
                        pcBuf[pos++] = '-';
                        // The minus sign has been placed, so turn off the
                        // negative flag.
                        if_neg = 0;
                    }
                    // Provide additional padding at the beginning of the
                    // string conversion if needed.
                    if((count > 1) && (count < 16)) {
                        for(count--; count; count--) {
                            pcBuf[pos++] = cFill;
                        }
                    }
                    // If the value is negative, then place the minus sign
                    // before the number.
                    if(if_neg) {
                        // Place the minus sign in the output buffer.
                        pcBuf[pos++] = '-';
                    }
                    // Convert the value into a string.
                    for(; index; index /= base) {
                        if (if_long) {
                            pcBuf[pos++] = g_pcHex[(ui64value / index) % base];
                        } else {
                            pcBuf[pos++] = g_pcHex[(ui32value / index) % base];
                        }
                    }
                    // Write the string.
                    log_config_handle.out_func((uint8_t *)pcBuf, pos);
                    iConvertCount += pos;
                    // This command has been handled.
                    break;
                }
                // Handle all other commands.
                default: {
                    // Indicate an error.
                    log_config_handle.out_func((uint8_t *)"ERROR", 5);
                    iConvertCount += 5;
                    // This command has been handled.
                    break;
                }
            }
        }
    }
    return iConvertCount;
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
    if(!log_config_handle.enable || !log_config_handle.out_func)
        goto end;
    /* Check filter */
    for(i = 0; i < log_config_handle.filter_cnt; i++) {
        if(log_config_handle.filter_tbl[i] && strstr(msg, log_config_handle.filter_tbl[i])) {
            goto end;
        }
    }
    if(level <= log_config_handle.level) {
        if(log_config_handle.format & LOG_FMT_TIME) {
            uint64_t tick = 0;
            struct tm time = {0};
            if(log_config_handle.get_tick_func) {
                tick = log_config_handle.get_tick_func();
            }
            log_sec2time(tick/1000, &time);
            offset += log_snprintf(log_head + offset, LOG_MAX_HEAD_LENGTH - offset, "%02d:%02d:%02d:%03d",
                                    time.tm_hour, time.tm_min, time.tm_sec, tick%1000);
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
        log_config_handle.out_func((uint8_t *)log_head, offset);
        va_list arg;
        va_start(arg, msg);
        offset += log_vprintf(msg, arg);
        va_end(arg);
        log_config_handle.out_func((uint8_t *)"\r\n", 2);
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
        log_config_handle.out_func = param;
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
    log_config_handle.out_func((uint8_t *)buffer, length);
}