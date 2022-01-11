/**
 * @file rte_shell.c
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 1.0.0
 * @date 2021-10-12
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "../../inc/middle_layer/rte_shell.h"
#include "../../inc/middle_layer/rte_log.h"
#include "../../inc/middle_layer/rte_timer.h"

#if RTE_SHELL_ENABLE == 1

#include <stdint.h>
#include <string.h>
#include <stdarg.h>

#define CTL_CH(ch) ((ch) - 'A' + 1)
#define U_SHELL_ALERT() shell_putc('\a')
#define THIS_MODULE LOG_STR(SHELL)
#define SHELL_LOGF(...) LOG_FATAL(THIS_MODULE, __VA_ARGS__)
#define SHELL_LOGE(...) LOG_ERROR(THIS_MODULE, __VA_ARGS__)
#define SHELL_LOGI(...) LOG_INFO(THIS_MODULE, __VA_ARGS__)
#define SHELL_LOGW(...) LOG_WARN(THIS_MODULE, __VA_ARGS__)
#define SHELL_LOGD(...) LOG_DEBUG(THIS_MODULE, __VA_ARGS__)
#define SHELL_LOGV(...) LOG_VERBOSE(THIS_MODULE, __VA_ARGS__)
#define SHELL_ASSERT(v) LOG_ASSERT(THIS_MODULE, v)

// erase sequence, used to erase one character on screen.
static const char _erase_seq[] = "\b \b";

typedef struct {
    // non-zero means readline completed.
    int done;
    /**
     * The number of characters present in `shell_handle.line_buffer`.
     * 0 <= `shell_handle.end` <= `CONFIG_SHELL_INPUT_BUFFSIZE`
     * When `shell_handle.cursor` is at the end of the line, `shell_handle.cursor` and `shell_handle.end` are equal.
     * Note that the value of `shell_handle.line_buffer[shell_handle.end]` should be `\0` in any case.
     */
    int end;
    // The offset of the current cursor position in `shell_handle.line_buffer`
    // 0 <= `shell_handle.cursor` <= `shell_handle.end`
    int cursor;
    /* uesed by @file{key_seq.c} to recognize key sequences */
    int key_seq_len;
    int home;
    uint16_t command_num;
    shell_cmd_t *command_table_base;
    // console input buffer
    char line_buffer[CONFIG_SHELL_INPUT_BUFFSIZE + 1];
} shell_handle_def_t;

static shell_handle_def_t shell_handle = {0};

#if CONFIG_SHELL_HIST_MIN_RECORD > 0

/**
memory view of the history buffer:
-------------------------------------------------------------------------------------------
|length| string |length| *** |length| string |length|                                     |
|<---- 1st record ---->| *** |<----   record   ---->|                                     |
^                            ^                      ^                                     ^
&buffer[0]                   cursor                 tail                                  buffer end
                                                    (new record will be saved here)
-------------------------------------------------------------------------------------------
Let the data type of `length` be `uint8_t`,
then the value of length: length = strlen(string) + 1 + 2*sizeof(uint8_t)
*/

#define TOTAL_BUFFER_SIZE \
  (CONFIG_SHELL_HIST_MIN_RECORD * (CONFIG_SHELL_INPUT_BUFFSIZE + 1 + 2 * sizeof(uint8_t)))

static char historyBuffer[TOTAL_BUFFER_SIZE];  // history buffer

#define HISTORY_BUFFER_BEGIN() (&historyBuffer[0])
#define HISTORY_BUFFER_END() (&historyBuffer[TOTAL_BUFFER_SIZE])

#define GET_RECORD_SIZE(pRecord) ((uint8_t)(*((uint8_t *)(pRecord))))

static char *historyCursor = HISTORY_BUFFER_BEGIN();
static char *historyTail = HISTORY_BUFFER_BEGIN();  // new record will be saved here

static char *shell_history_next(void)
{
    if (historyCursor >= historyTail ||                                // cursor point to the tail
        historyCursor + GET_RECORD_SIZE(historyCursor) >= historyTail  // cursor point to the last one
    ) {
        return NULL;
    }

    historyCursor += GET_RECORD_SIZE(historyCursor);
    return historyCursor + sizeof(uint8_t);
}

static char *shell_history_prev(void)
{
    if (historyTail != HISTORY_BUFFER_BEGIN() &&  // buffer is not empty
        historyCursor > HISTORY_BUFFER_BEGIN()    // cursor does not point to the first
    ) {
        historyCursor -= GET_RECORD_SIZE(historyCursor - sizeof(uint8_t));
        return historyCursor + sizeof(uint8_t);
    }

    return NULL;
}

static void shell_history_add(char *input)
{
    size_t freeSpace = HISTORY_BUFFER_END() - historyTail;

    uint8_t inputLength = strlen(input) + 1;
    uint8_t newRecordLength = inputLength + 2 * sizeof(uint8_t);

    if (freeSpace < newRecordLength) {
        uint8_t tmpLength;
        char *tmpRecord = HISTORY_BUFFER_BEGIN();
        do {
        tmpLength = GET_RECORD_SIZE(tmpRecord);
        freeSpace += tmpLength;
        tmpRecord += tmpLength;
        } while (freeSpace < newRecordLength);

        memmove(HISTORY_BUFFER_BEGIN(), tmpRecord, historyTail - tmpRecord);
        historyTail -= (tmpRecord - HISTORY_BUFFER_BEGIN());
    }

    /* put the new record in the history buffer */
    *((uint8_t *)historyTail) = newRecordLength;
    memcpy(historyTail + sizeof(uint8_t), input, inputLength);
    historyTail += newRecordLength;  // move tail to the end of the new record
    *((uint8_t *)(historyTail - sizeof(uint8_t))) = newRecordLength;

    /* set cursor point to the end */
    historyCursor = historyTail;
}

static void shell_process_history(const char *history)
{
    if (history) {
        shell_puts(&shell_handle.line_buffer[shell_handle.cursor]);  // move cursor to the end on screen.

        while (shell_handle.end > shell_handle.home) {       // erase all on the screen.
            shell_puts(_erase_seq);
            shell_handle.end--;
        }

        shell_handle.end = strlen(history) + shell_handle.home;  // update shell_handle.end.
        shell_handle.cursor = shell_handle.end;       // update shell_handle.cursor if LINE_EDITING is enabled.

        memcpy(shell_handle.line_buffer + shell_handle.home, history, shell_handle.end -shell_handle.home + 1);
        shell_puts(shell_handle.line_buffer + shell_handle.home);  // display new text and move cursor to the end on screen.
    } else {
        U_SHELL_ALERT();
    }
}

static void shell_get_pre_history(void)
{
    shell_process_history(shell_history_prev());
}

static void shell_get_next_history(void)
{
    shell_process_history(shell_history_next());
}

#endif /* CONFIG_SHELL_HIST_MIN_RECORD > 0 */

static void shell_get_pre_history(void);   // `up arrow` or `Ctrl P`
static void shell_get_next_history(void);  // `down arrow` or `Ctrl N`
static void shell_backward_cursor(void);   // `right arrow` or `Ctrl F`
static void shell_forward_cursor(void);    // `left arrow` or `Ctrl B`
static void shell_line_home(void);         // `Home`
static void shell_line_end(void);          // `End`
static void shell_delete(void);            // `Delete`
static void shell_end_input(void);

#define CONVERT_KEY_SEQ(c1, c2, c3, c4) \
    ((uint32_t)((((uint8_t)(c1)) << 24) | (((uint8_t)(c2)) << 16) | (((uint8_t)(c3)) << 8) | (((uint8_t)(c4)) << 0)))

typedef struct {
    uint32_t key_seq_val;
    void (*key_func)(void);
} key_seq_t;

const static key_seq_t key_seq_map[] = {
#if CONFIG_SHELL_HIST_MIN_RECORD > 0
    {CONVERT_KEY_SEQ('\033', '[', 'A', 0), shell_get_pre_history},   // up arrow
    {CONVERT_KEY_SEQ('\033', '[', 'B', 0), shell_get_next_history},  // down arrow
    // {CONVERT_KEY_SEQ('\033', 'O', 'A', 0), shell_get_pre_history},
    // {CONVERT_KEY_SEQ('\033', 'O', 'B', 0), shell_get_next_history},

#endif /* CONFIG_SHELL_HIST_MIN_RECORD */
    {CONVERT_KEY_SEQ('\033', '[', 'C', 0), shell_backward_cursor},  // right arrow
    {CONVERT_KEY_SEQ('\033', '[', 'D', 0), shell_forward_cursor},   // left arrow
    {CONVERT_KEY_SEQ('\033', '[', 'H', 0), shell_line_home},        // home
    {CONVERT_KEY_SEQ('\033', '[', 'F', 0), shell_line_end},         // end
    // {CONVERT_KEY_SEQ('\033', 'O', 'C', 0), shell_forward_cursor},
    // {CONVERT_KEY_SEQ('\033', 'O', 'D', 0), shell_backward_cursor},
    // {CONVERT_KEY_SEQ('\033', 'O', 'H', 0), shell_line_home},
    // {CONVERT_KEY_SEQ('\033', 'O', 'F', 0), shell_line_end},

    {CONVERT_KEY_SEQ('\033', '[', '3', '~'), shell_delete},  // delete
};

static void shell_dispatch_seq(char ch)
{
    static uint32_t key_seq_val, key_seq_mask;

    uint32_t offset, miss_match, tmp_val;

    shell_handle.key_seq_len++;

    offset = ((uint32_t)(sizeof(uint32_t) - shell_handle.key_seq_len)) << 3;  // (4-shell_handle.key_seq_len)*8
    key_seq_val |= (((uint32_t)ch) << offset);
    key_seq_mask |= (0xFF << offset);

    miss_match = 1;
    for (int i = 0; i < ARRAY_SIZE(key_seq_map); i++) {
            tmp_val = key_seq_map[i].key_seq_val;
            if ((tmp_val & key_seq_mask) == key_seq_val) {  // partial match
                if (key_seq_val == tmp_val) {                 // full match
                    key_seq_val = 0;
                    key_seq_mask = 0;
                    shell_handle.key_seq_len = 0;

                    key_seq_map[i].key_func();
                    return;
                }
                miss_match = 0;
        }
    }

    if (miss_match) {
        key_seq_val = 0;
        key_seq_mask = 0;
        shell_handle.key_seq_len = 0;
        shell_putc('\a');
    }
}

/**
 * @brief Judge whether the line should be continued
 *
 * @return int 1: continue.
 *             0: no continue, start a new line.
 */
static int shell_should_continue()
{
    // in version 1.0, only judged whether the last character is '\' or not
    return (shell_handle.end > shell_handle.home && shell_handle.line_buffer[shell_handle.end-1] == '\\');
}

static void shell_new_line()
{
    if (shell_should_continue()) {
        shell_handle.line_buffer[--shell_handle.end] = '\0'; // overwrite the backslash('\')
        shell_handle.home = shell_handle.end;
        shell_handle.cursor = shell_handle.end;  // update shell_handle.cursor if LINE_EDITING is enabled.
        shell_puts("\r\n> ");
    } else {
        shell_end_input();
    }
}


static void shell_end_input(void)
{
#if CONFIG_SHELL_HIST_MIN_RECORD > 0
    if (*shell_handle.line_buffer) {
        shell_history_add(shell_handle.line_buffer);
    }
#endif /*CONFIG_SHELL_HIST_MIN_RECORD */

    shell_handle.home = 0;
    shell_handle.cursor = 0;
    shell_handle.end = 0;
    shell_handle.done = 1;
    shell_puts("\r\n");
}

// add one character to the buffer
static void shell_add_char(char ch)
{
    if (shell_handle.end < CONFIG_SHELL_INPUT_BUFFSIZE && ch >= ' ') {
        int len = shell_handle.end - shell_handle.cursor;

        shell_handle.end++;
        shell_handle.line_buffer[shell_handle.end] = '\0';

        memmove(&shell_handle.line_buffer[shell_handle.cursor + 1], &shell_handle.line_buffer[shell_handle.cursor], len);
        shell_handle.line_buffer[shell_handle.cursor] = ch;

        shell_puts(&shell_handle.line_buffer[shell_handle.cursor++]);
        while (len > 0) {
            shell_putc('\b');
            len--;
        }
    } else {
        U_SHELL_ALERT();
    }
}

// Rubout the character behind `shell_handle.cursor`(Backspace).
static void shell_rubout(void)
{
    if (shell_handle.cursor > shell_handle.home) {
        int len = shell_handle.end - (--shell_handle.cursor);
        shell_handle.end--;

        memmove(&shell_handle.line_buffer[shell_handle.cursor], &shell_handle.line_buffer[shell_handle.cursor + 1], len);
        shell_putc('\b');
        shell_puts(&shell_handle.line_buffer[shell_handle.cursor]);
        shell_putc(' ');
        do {
            shell_putc('\b');
            len--;
        } while (len > 0);
    } else {
        U_SHELL_ALERT();
    }
}


// Delete the character under the cursor (Delete).
static void shell_delete(void)
{
    if (shell_handle.cursor < shell_handle.end) {
        int len = shell_handle.end - shell_handle.cursor;
        shell_handle.end--;

        memmove(&shell_handle.line_buffer[shell_handle.cursor], &shell_handle.line_buffer[shell_handle.cursor + 1], len);
        shell_puts(&shell_handle.line_buffer[shell_handle.cursor]);
        shell_putc(' ');
        do {
            shell_putc('\b');
            len--;
        } while (len > 0);
    }
}

// Move curosr to the beginning of line.
static void shell_line_home(void)
{
    while (shell_handle.cursor > shell_handle.home) {
        shell_putc('\b');
        shell_handle.cursor--;
    }
}

// Move cursor to the end of line.
static void shell_line_end(void)
{
    shell_puts(shell_handle.line_buffer + shell_handle.cursor);
    shell_handle.cursor = shell_handle.end;
}

// Move forward (left).
static void shell_forward_cursor(void)
{
    if (shell_handle.cursor > shell_handle.home) {
        shell_putc('\b');
        shell_handle.cursor--;
    } else {
        U_SHELL_ALERT();
    }
}

// Move backward (right).
static void shell_backward_cursor(void)
{
    if (shell_handle.cursor < shell_handle.end) {
        shell_putc(shell_handle.line_buffer[shell_handle.cursor]);
        shell_handle.cursor++;
    } else {
        U_SHELL_ALERT();
    }
}

// Erase from beginning of line to cursor.
static void shell_erase_all_backward(void)
{
    if (shell_handle.cursor > shell_handle.home) {
        int len = shell_handle.end - shell_handle.cursor + 1;

        shell_puts(&shell_handle.line_buffer[shell_handle.cursor]);  // move cursor to the end on screen.
        while (shell_handle.end > shell_handle.home) {                       // erase all on the screen
            shell_puts(_erase_seq);
            shell_handle.end--;
        }

        memmove(shell_handle.line_buffer + shell_handle.home, &shell_handle.line_buffer[shell_handle.cursor], len--);  // new text.
        shell_puts(shell_handle.line_buffer + shell_handle.home);  // display new text and move cursor to the end on screen.

        shell_handle.cursor = shell_handle.home;
        shell_handle.end = len + shell_handle.home;
        while (len > 0) {  // move cursor to the begin on the screen.
            shell_putc('\b');
            len--;
        }
    }
}

// Erase from cursor to end of line.
static void shell_erase_all_forward(void)
{
    shell_puts(&shell_handle.line_buffer[shell_handle.cursor]);  // move cursor to the end on screen.
    while (shell_handle.end > shell_handle.cursor) {
        shell_puts(_erase_seq);  // erase all right to shell_handle.cursor, and move screen cursor to shell_handle.cursor.
        shell_handle.end--;
    }
    shell_handle.line_buffer[shell_handle.end] = '\0';
}

static void shell_dispatch(char ch)
{
    if (shell_handle.key_seq_len) {
        shell_dispatch_seq(ch);
        return;
    }

    switch (ch) {
        case '\r':  // CTL_CH('M')
        case '\n':  // CTL_CH('J')
            shell_new_line();
            break;

        case CTL_CH('C'):
            shell_puts("^C\r\n");
            *shell_handle.line_buffer = '\0';
            shell_end_input();
            break;

        case 127:
        case 8:  // backspace, CTL_CH('H')
            shell_rubout();
            break;

        case '\033':  // ESC(\033)
            shell_dispatch_seq(ch);
            break;

        case CTL_CH('A'):  // HOME
            shell_line_home();
            break;
        case CTL_CH('E'):  // END
            shell_line_end();
            break;

        case CTL_CH('B'):  // <-- (left arrow)
            shell_forward_cursor();
            break;
        case CTL_CH('F'):  // --> (right arrow)
            shell_backward_cursor();
            break;


        case CTL_CH('K'):  // Delete all characters on the right side.
            shell_erase_all_forward();
            break;
        case CTL_CH('U'):  // Delete all characters one the left side.
            shell_erase_all_backward();
            break;

        case CTL_CH('D'):  // DELETE
            shell_delete();
            break;

    // case CTL_CH('X'):
    // case CTL_CH('O'):
    //   break;

#if CONFIG_SHELL_HIST_MIN_RECORD > 0
        case CTL_CH('P'):  // up arrow
            shell_get_pre_history();
            break;
        case CTL_CH('N'):  // down arrow
            shell_get_next_history();
            break;
#endif /* CONFIG_SHELL_HIST_MIN_RECORD > 0 */

        default:  // add current character to the buffer
            shell_add_char(ch);
        break;
    }
}

static const shell_cmd_t *shell_find_cmd(const char *cmd_name)
{
    if (cmd_name == NULL) {
        return NULL;
    }
    for (uint16_t i = 0; i < shell_handle.command_num; i++) {
        const shell_cmd_t *tmp = shell_handle.command_table_base + i;
        if (strcmp(cmd_name, tmp->name) == 0) {
            return tmp;
        }
    }
    return NULL;
}

static const shell_cmd_t *shell_find_sub_cmd(const char *cmd_name, const shell_cmd_t *start, uint16_t count)
{
    if (cmd_name == NULL) {
        return NULL;
    }
    for (uint16_t i = 0; i < count; i++) {
        const shell_cmd_t *tmp = start + i;
        if (strcmp(cmd_name, tmp->name) == 0) {
            return tmp;
        }
    }
    return NULL;
}

static void shell_print_cmd_list(void)
{
    for (uint16_t i = 0; i < shell_handle.command_num; i++) {
        const shell_cmd_t *tmp = shell_handle.command_table_base + i;
        shell_printf("  %s: %s\r\n", tmp->name, tmp->brief_usage);
    }
}

static void shell_print_cmd_help(const char *cmd_name)
{
    const shell_cmd_t *tmp = shell_find_cmd(cmd_name);

    if (tmp) {
        shell_printf("%s: %s\r\n", cmd_name, tmp->brief_usage);
        shell_puts(tmp->help);
    } else {
        shell_printf("%s: command not found.\r\n", cmd_name);
    }
}

static int shell_help_generic(int argc, char *const argv[],
            const char* preamble)
{
    if (argc == 1) {
        shell_puts(preamble);
        shell_puts("\r\n"
                "Type `help name' to find out more about the function `name'.\r\n"
                "\r\n");
        shell_print_cmd_list();
        shell_puts("\r\n");
    } else {
        for (int i = 1; i < argc; i++) {
            shell_print_cmd_help(argv[i]);
        }
    }
    return 0;
}

static int shell_cmd_help(const shell_cmd_t *pcmd, int argc, char *const argv[])
{
    return shell_help_generic(argc, argv, "");
}

SHELL_ADD_CMD(help, shell_cmd_help,
            "help [pattern ...]",
            "    Print information about builtin commands.\r\n"
            "\r\n"
            "    If PATTERN is specified, gives detailed help on all commands\r\n"
            "    matching PATTERN, otherwise print the list of all available commands.\r\n"
            "\r\n"
            "    Arguments:\r\n"
            "        PATTERN: specifiying the help topic\r\n");

#define isblank(c) ((c) == ' ' || (c) == '\t')

static int shell_parse_line(char *input, char *argv[], const int maxArgc)
{
    char tmp;
    int nargc = 0;

    while (nargc < maxArgc) {
        while (isblank(*input)) {
            input++;
        }
        if (*input == '\0') {  // end of input
            argv[nargc] = NULL;
            break;
        }

        tmp = *input;
        // single quotes ('') and double quotes ("")
        if (tmp == '\'' || tmp == '"') {
            argv[nargc] = ++input;
            while (*input && (*input != tmp)) {
                input++;
            }
        } else {  // normal character
            argv[nargc] = input++;
            while (*input && !isblank(*input)) {
                input++;
            }
        }

        nargc++;
        if (*input) {
            *input++ = '\0'; /* terminate current arg */
        }
    }

    return nargc;
}

#if defined(__GNUC__)
#ifndef __weak
#define __weak __attribute__((weak))
#endif /* __weak */
#endif /* __GNUC__ */

__weak int shell_getc(char *ch)
{
    return 1;
}

void shell_printf(const char * restrict format, ...)
{
    va_list arg;
    va_start(arg, format);
    log_vprintf(format, arg);
    va_end(arg);
}

void shell_puts(const char *str)
{
    log_output(str, strlen(str));
}

void shell_putc(char ch)
{
    log_output(&ch, 1);
}

#if (CONFIG_SHELL_CMD_MAX_ARGC < 1)
#error "CONFIG_SHELL_CMD_MAX_ARGC must be no less than 1."
#endif

void shell_task(void *argument)
{
    char *argv[CONFIG_SHELL_CMD_MAX_ARGC + 1] = {NULL};
    int argc = 0;
    const shell_cmd_t *last_cmd = NULL;
    shell_puts("\r\n"
                "    _____ __        _____ ____  __ __\r\n"
                "   / ___// /       / ___// __ \\/ //_/\r\n"
                "   \\__ \\/ /  ______\\__ \\/ / / / ,<\r\n"
                "  ___/ / /__/_____/__/ / /_/ / /| |\r\n"
                " /____/_____/    /____/_____/_/ |_|\r\n"
                "\r\n"
                "Welcome to SL_SDK "RTE_VERSION"\r\n"
                "\r\n"
                " * Build:     "__DATE__" - "__TIME__"\r\n"
                " * Source:    https://github.com/sudashannon/SL_SDK\r\n"
                " * Copyright: (c) 2021 @Leon Shan\r\n"
                "\r\n");
    for (;;) {
        if (last_cmd == NULL)
            shell_puts(CONFIG_SHELL_PROMPT);
        // clean last line.
        shell_handle.done = 0;
        *shell_handle.line_buffer = '\0';
        // start read the new line.
        while (shell_handle.done == 0) {
            char input = 0;
            if(!shell_getc(&input))
                continue;
            shell_dispatch(input);
        }
        if (last_cmd == NULL) {
            argc = shell_parse_line(shell_handle.line_buffer, argv, CONFIG_SHELL_CMD_MAX_ARGC + 1);
            if (argc > CONFIG_SHELL_CMD_MAX_ARGC) {
                argc--;
                shell_printf("** WARNING: too many args (max: %d)! ", CONFIG_SHELL_CMD_MAX_ARGC);
                shell_printf("arguments after \"%s\" will be ignored. **\r\n", argv[argc - 1]);
            } else if (argc > 0) {
                const shell_cmd_t *cmd = shell_find_cmd(argv[0]);
                if (cmd) {
                    int ret = cmd->callback(cmd, argc, argv);
                    if (ret != 0) {
                        shell_printf("  %s: command exits with %d.\r\n", cmd->name, ret);
                    }
                    if (cmd->exe_mode)
                        last_cmd = cmd;
                } else {
                    shell_printf("  %s: command not found.\r\n", argv[0]);
                }
            }
        } else {
            int ret = last_cmd->exit(last_cmd, argc, argv);
            if (ret != 0) {
                shell_printf("  %s: command exits with %d.\r\n", last_cmd->name, ret);
            }
            last_cmd = NULL;
        }
    }
}

void shell_react(char ch)
{
    static char *argv[CONFIG_SHELL_CMD_MAX_ARGC + 1];
    int argc;

    if (shell_handle.done) { // clean last line.
        shell_handle.done = 0;
        *shell_handle.line_buffer = '\0';
    }

    shell_dispatch(ch);

    if (shell_handle.done) {
        /**
         * in react mode, use if (* line) to avoid unnecessary process
         * to improve speed.
         */
        if (*shell_handle.line_buffer) {
            argc = shell_parse_line(shell_handle.line_buffer, argv, CONFIG_SHELL_CMD_MAX_ARGC + 1);

            if (argc > CONFIG_SHELL_CMD_MAX_ARGC) {
                argc--;
                shell_printf("** WARNING: too many args (max: %d)! ", CONFIG_SHELL_CMD_MAX_ARGC);
                shell_printf("arguments after \"%s\" will be ignored. **\r\n", argv[argc - 1]);
            }

            if (argc > 0) {
                const shell_cmd_t *cmd = shell_find_cmd(argv[0]);
                if (cmd) {
                    int ret = cmd->callback(cmd, argc, argv);
                    if (ret != 0) {
                        shell_printf("  %s: command exits with %d.\r\n", cmd->name, ret);
                    }
                } else {
                    shell_printf("  %s: command not found.\r\n", argv[0]);
                }
            }
        }

        if (CONFIG_SHELL_PROMPT) {
            shell_puts(CONFIG_SHELL_PROMPT);
        }
    }
}

int shell_run_subcmd_implem(const shell_cmd_t* pCmdt,
                int argc, char* const argv[],
                shell_cmd_cb_t fallback_fct,
                const shell_cmd_t* subcommands, unsigned int subcommands_count)
{
    if (argc > 1) {
        const shell_cmd_t* sub_cmd = shell_find_sub_cmd(argv[1], subcommands, subcommands_count);

        if (sub_cmd) {
            return sub_cmd->callback(sub_cmd, argc - 1, argv + 1);
        } else if(fallback_fct) {
            return fallback_fct(pCmdt, argc, argv);
        } else {
            shell_printf("  %s: sub-command not found.\r\n", argv[1]);
        }
    } else if(fallback_fct) {
        return fallback_fct(pCmdt, argc, argv);
    }

    return -1;
}

void shell_init(void)
{
    shell_cmd_t *start = NULL;
    shell_cmd_t *end = NULL;
#if defined (__CC_ARM) || defined (__ARMCC_VERSION)
    extern const uintptr_t shell_command$$Base;
    extern const uintptr_t shell_command$$Limit;
    start = (shell_cmd_t *)&shell_command$$Base;
    end = (shell_cmd_t *)&shell_command$$Limit;
#elif defined(__GNUC__)
    extern const uintptr_t __shell_command_start;
    extern const uintptr_t __shell_command_end;
    start = (shell_cmd_t *)&__shell_command_start;
    end = (shell_cmd_t *)&__shell_command_end;
#else
#error "Doesn't support this platform!"
#endif
    shell_handle.command_num = (end - start);
    shell_handle.command_table_base = start;
    SHELL_LOGI("command num %d start %p end %p", shell_handle.command_num, start, end);
}

#endif
