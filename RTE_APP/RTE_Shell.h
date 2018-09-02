#ifndef __RTE_SHELL_H
#define __RTE_SHELL_H
#ifdef __cplusplus
extern "C" {
#endif
	#include "RTE_Include.h"
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
	
	typedef struct
	{
		char *CmdBuffer;
		uint8_t CmdCnt;
		RTE_Shell_t *g_psCmdTable;
		char *g_ppcArgv[HI_SHELL_MAX_ARGS + 1];
	}RTE_Shell_Control_t;
	extern RTE_Shell_Err_e RTE_Shell_AddCommand(const char *cmd,RTE_Shell_Err_e (*func)(int argc, char *argv[]),const char *help);
	extern RTE_Shell_Err_e RTE_Shell_DeleteCommand(const char *cmd);
	extern void RTE_Shell_Init(void);
  extern void RTE_Shell_Poll(void);
#ifdef __cplusplus
}
#endif
#endif
