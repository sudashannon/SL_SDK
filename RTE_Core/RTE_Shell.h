#ifndef __RTE_SHELL_H
#define __RTE_SHELL_H
#ifdef __cplusplus
extern "C" {
#endif
	#include "RTE_Config.h"
	#if RTE_USE_SHELL == 1
	#include <stdbool.h>
	#include "RTE_Vector.h"
	typedef enum
	{
		SHELL_NOERR = 0,
		SHELL_NOMODULE = 1,
		SHELL_NOSPACEFORNEW = 2,
		SHELL_ALREADYEXIST = 3,
		SHELL_NOSUCHCMD = 4,
		SHELL_TOOMANYARGS = 5,
		SHELL_NOVALIDCMD = 6,
		SHELL_ARGSERROR = 7,
	}shell_error_e;
	typedef struct
	{
		//! A pointer to a string containing the name of the command.
		const char *pcCmd;
		//! A function pointer to the implementation of the command.
		shell_error_e (*pfnCmdLine)(int argc, char *argv[]);
		//! A pointer to a string of brief help text for the command.
		const char *pcHelp;
	}shell_cmd_t;
	typedef struct
	{
        uint8_t ShellModuleID;
        const char *ShellModuleName;
        vector_t *ModuleFucTable;
	}shell_module_t;

	typedef struct
	{
	    uint8_t ShellModuleCnt;
		char *g_ppcArgv[SHELL_MAX_ARGS + 2];
		shell_module_t *ShellModule;
	}shell_t;

	extern shell_error_e Shell_CreateModule(const char *module);
	extern shell_error_e Shell_AddCommand(const char *module,const char *cmd,shell_error_e (*func)(int argc, char *argv[]),const char *help);
	extern shell_error_e Shell_DeleteCommand(const char *module,const char *cmd);
	extern void Shell_Init(void);
    extern void Shell_Poll(void *Params);
	extern void Shell_Input(uint8_t *Data,uint16_t Length);
	#endif
#ifdef __cplusplus
}
#endif
#endif
