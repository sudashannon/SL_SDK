#ifndef __RTE_SHELL_H
#define __RTE_SHELL_H

#include "rte.h"

#if RTE_SHELL_ENABLE == 1

/* command line input buffer size(byte) */
#define CONFIG_SHELL_INPUT_BUFFSIZE (127U)

/**
 * set 0 to disable history record.
 *
 * shell will take `CONFIG_SHELL_HIST_MIN_RECORD*(2+CONFIG_SHELL_INPUT_BUFFSIZE)` bytes to
 *   record **at least** `CONFIG_SHELL_HIST_MIN_RECORD` histroys.
 * the maximum number of history records depends on the average length of the input.
 */
#define CONFIG_SHELL_HIST_MIN_RECORD (5U)

/* config the max number of arguments, must be no less than 1. */
#define CONFIG_SHELL_CMD_MAX_ARGC (10U)

#define CONFIG_SHELL_PROMPT "SL-SDK >> "

#define CONFIG_SHELL_BOOT_INFO  "\r\n"                                                          \
                                "    _____ __        _____ ____  __ __\r\n"                     \
                                "   / ___// /       / ___// __ \\/ //_/\r\n"                    \
                                "   \\__ \\/ /  ______\\__ \\/ / / / ,<\r\n"                    \
                                "  ___/ / /__/_____/__/ / /_/ / /| |\r\n"                       \
                                " /____/_____/    /____/_____/_/ |_|\r\n"                       \
                                "\r\n"                                                          \
                                "Welcome to SL_SDK "RTE_VERSION"\r\n"                           \
                                "\r\n"                                                          \
                                " * Build:     "__DATE__" - "__TIME__"\r\n"                     \
                                " * Source:    https://github.com/sudashannon/SL_SDK\r\n"       \
                                " * Copyright: (c) 2021 @Leon Shan\r\n"                         \
                                "\r\n"

/*********************************************************************
    shell uses these functions to get/send character from/to stream.
    You should implement these functions in your project.
 *********************************************************************/

/**
 * @brief send a chararcter...
 *
 */
extern void shell_putc(char ch);

/**
 * @brief send string...
 *
 */
extern void shell_puts(const char *str);


/**
 * @brief Shell printf
 *
 */
extern void shell_printf(const char * restrict format, ...);
/**
 * @brief Get next character available from stream.
 *        not used in <interrupt mode>.
 *
 * @param ch Return the character in `ch` if there was...
 * @return Result is non-zero if there was a character, or 0 if there wasn't.
 *
 * @note if you run shell in <main loop mode>, to avoid losing characters, you'd better use a
 * low layer receive fifo. Take uart for example, you can detect incoming data using interrupts and
 * then store each received character in a first-in-first out (FIFO) buffer.
 *
 * then `shell_getc(char *ch)` may be like this:
 *
 * int shell_getc(char *ch)
 * {
 *   if (fifo_empty()) {  // if no character in fifo,
 *     return 0;          // return false
 *   }
 *
 *   *ch = fifo_pop();  // fifo is not empty, get it.
 *   return 1;          // return true
 * }
 *
 */
extern int shell_getc(char *ch);
/**
 * @brief shell infinite loop
 *
 * @param argument not used in ver1.0
 */
extern void shell_task(void *argument);

/**
 * @brief shell non-block interface, just react to the input character.
 * It is non-blocked (unless there is an infinite loop in your command function)
 * you can call it when get a new character.
 *
 * @param ch input character
 */
extern void shell_react(char ch);
/**
 * @brief array entry declare.
 * @_type: data type of the entry.
 * @_name: name of the entry.
 */
#define _shell_entry_declare(_type, _name)                              \
    __attribute__((used)) __attribute__((section("shell_command")))     \
        const _type shell_##_type##_##_name

// Forward delecation of shell command structure
struct _shell_cmd_s;

/**
 * @brief this is the implementation function of the command.
 *
 * @param argc: the count of arguments.
 * @param argv: argument vector.
 * @return 0 if succeed, else non-zero. (return value is not used in ver1.0)
 *
 * @note the command name is the first argument, argv[0], so argc is always at least 1.
 */
typedef  int (*shell_cmd_cb_t)(int argc, char *const argv[]);

// shell command structure
typedef struct _shell_cmd_s {
    int exe_mode;
    const char *name; // command name
    shell_cmd_cb_t exit;
    shell_cmd_cb_t callback; // Callback function to run the shell command
    const char *brief_usage; // brief usage of the command.
    const char *help;  // detailed help information of the command.
} shell_cmd_t;

#define _shell_cmd_initializar(_mode, _name, _exit, _func, _brief, _help) \
    { _mode, #_name, _exit, _func, _brief, _help }

/**
 * @brief add a command to shell, NOTE: exe mode will pause the shell input.
 *
 * @_name: name of the command. Note: THIS IS NOT a string.
 * @_func: function pointer: (*cmd)(const shell_cmd_t *, int, int, char *const[]).
 * @_brief: brief summaries of the command. This is a string.
 * @_help: detailed help information of the command. This is a string.
 */
#define SHELL_ADD_CMD(_name, _func, _brief, _help) \
    _shell_entry_declare(shell_cmd_t, _name) = _shell_cmd_initializar(0, _name, NULL, _func, _brief, _help)

#define SHELL_ADD_EXE_CMD(_name, _exitf, _func, _brief, _help) \
    _shell_entry_declare(shell_cmd_t, _name) = _shell_cmd_initializar(1, _name, _exitf, _func, _brief, _help)

extern void shell_init(void);

#endif

#endif
