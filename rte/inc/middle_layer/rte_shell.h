#ifndef __RTE_SHELL_H
#define __RTE_SHELL_H

#include "rte.h"

#if RTE_SHELL_ENABLE == 1

/******************************* readline configuration ****************************/

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

/* shell provides a built-in help command, set 0 to disable it */
#define CONFIG_SHELL_CMD_BUILTIN_HELP 1

/* config the max number of arguments, must be no less than 1. */
#define CONFIG_SHELL_CMD_MAX_ARGC (10U)

#define CONFIG_SHELL_PROMPT "SL-SDK >> "


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
    __attribute__((used)) __attribute__((section("shell")))             \
        const _type shell_##_type##_##_name

// Forward delecation of shell command structure
struct _shell_cmd_s;

/**
 * @brief this is the implementation function of the command.
 *
 * @param pCmdt: pointer of the structure.
 * @param argc: the count of arguments.
 * @param argv: argument vector.
 * @return 0 if succeed, else non-zero. (return value is not used in ver1.0)
 *
 * @note the command name is the first argument, argv[0], so argc is always at least 1.
 */
typedef  int (*shell_cmd_cb_t)(const struct _shell_cmd_s *cmd, int argc, char *const argv[]);

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

#if CONFIG_SHELL_CMD_BUILTIN_HELP
    #define _shell_help_subcmd_entry(_name) \
        SHELL_SUBCMD_ENTRY(help, _name ## _subcmd_help, \
                        "help [pattern ...]", \
                        "    Print information about subcommands of " # _name ".\r\n" \
                        "\r\n" \
                        "    If PATTERN is specified, gives detailed help on all commands\r\n" \
                        "    matching PATTERN, otherwise print the list of all available commands.\r\n" \
                        "\r\n" \
                        "    Arguments:\r\n" \
                        "        PATTERN: specifiying the help topic\r\n"),
    #define _shell_help_subcmd_declare(_name) \
        static int _name ## _subcmd_help(const shell_cmd_t* pCmd, int argc, char* const argv[]);
    #define _shell_help_subcmd_define(_name) \
        static int _name ## _subcmd_help(const shell_cmd_t* pCmd, int argc, char* const argv[]) \
        { \
        const unsigned int subcommands_count = sizeof(_name ## _subcommands)/sizeof(shell_cmd_t); \
        return shell_help_generic( \
                        argc, argv, \
                        "Help for " #_name, \
                        _name ## _subcommands, subcommands_count); \
        }
#else
    #define _shell_help_subcmd_entry(_name)
    #define _shell_help_subcmd_declare(_name)
    #define _shell_help_subcmd_define(_name)
#endif  /* CONFIG_SHELL_CMD_BUILTIN_HELP */

/**
 * @brief Add a sub command in a group of sub-command
 *
 * To be used as the last arguments of @ref SHELL_DEFINE_SUBCMDS()
 * The syntax is similar to @ref SHELL_ADD_CMD()
 *
 * @param _name: name of the command. Note: THIS IS NOT a string.
 * @param _func: function pointer: (*cmd)(const shell_cmd_t *, int, int, char *const[]).
 * @param _brief: brief summaries of the command. This is a string.
 * @param _help: detailed help information of the command. This is a string.
 */
#define SHELL_SUBCMD_ENTRY(_name, _func, _brief, _help) _shell_cmd_complete(_name, _func, _brief, _help)

/**
 * @brief Get the name of the function implementing a sub-command group in shell
 *
 * @param _name name of the group of sub-commands
 *
 * @note this macro is to be used for the @c _func parameter of @ref SHELL_ADD_CMD() or @c _func parameter of @ref SHELL_SUBCMD_ENTRY()
 */
#define SHELL_SUBCMDS_FCT(_name) _name ## _shell_cmd

/**
 * @brief Define a group of sub-commands in shell
 *
 * @param _name name of the group of sub-commands
 * @param fallback_fct: function that will be run if no subcommand can be found (either @c argc is 1 or argv[1] is not found in @c subcommand)
 * @param ... A list of @ref SHELL_SUBCMD_ENTRY() that define the list of sub-commands
 */
#define SHELL_DEFINE_SUBCMDS(_name, fallback_fct, ...) \
    _shell_help_subcmd_declare(_name) \
    static  const shell_cmd_t _name ## _subcommands[] = { \
            _shell_help_subcmd_entry(_name) \
            __VA_ARGS__ }; \
    _shell_help_subcmd_define(_name) \
    int SHELL_SUBCMDS_FCT(_name)(const shell_cmd_t* pCmd, int argc, char* const argv[]) \
    { \
        const unsigned int subcommands_count = sizeof(_name ## _subcommands)/sizeof(shell_cmd_t); \
        return shell_run_subcmd_implem(pCmd, argc, argv, \
        fallback_fct, _name ## _subcommands, subcommands_count); \
    }

extern int shell_run_subcmd_implem(const shell_cmd_t* pCmdt,
                int argc, char* const argv[],
                shell_cmd_cb_t fallback_fct,
                const shell_cmd_t* subcommands, unsigned int  subcommands_count);

extern void shell_init(void);

#endif

#endif
