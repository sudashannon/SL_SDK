#ifndef __RTE_SHELL_H
#define __RTE_SHELL_H
#ifdef __cplusplus
extern "C" {
#endif
	#include "RTE_Include.h"
	#if RTE_USE_SHELL
	typedef enum
	{
		SHELL_NOERR = 0,
		SHELL_UNINITHANDLE = 1,
		SHELL_NOSPACEFORNEW = 2,
		SHELL_ALREADYEXIST = 3,
		SHELL_NOSUCHCMD = 4,
		SHELL_TOOMANYARGS = 5,
		SHELL_NOVALIDCMD = 6,
	}RTE_Shell_Err_e;
	typedef struct
	{
		//! A pointer to a string containing the name of the command.
		const char *pcCmd;
		//! A function pointer to the implementation of the command.
		RTE_Shell_Err_e (*pfnCmdLine)(int argc, char *argv[]);
		//! A pointer to a string of brief help text for the command.
		const char *pcHelp;
	}RTE_Shell_t;
	
	typedef vec_t(RTE_Shell_t) sh_vec_t;
	
	typedef struct
	{
		char *g_ppcArgv[SHELL_MAX_ARGS + 1];
		sh_vec_t g_psCmdTable;
	}RTE_Shell_Control_t;
	
	extern RTE_Shell_Err_e RTE_Shell_AddCommand(const char *cmd,RTE_Shell_Err_e (*func)(int argc, char *argv[]),const char *help);
	extern RTE_Shell_Err_e RTE_Shell_DeleteCommand(const char *cmd);
	extern void RTE_Shell_Init(void);
  extern void RTE_Shell_Poll(char *ShellBuffer);
	#endif
#ifdef __cplusplus
}
#endif
#endif
