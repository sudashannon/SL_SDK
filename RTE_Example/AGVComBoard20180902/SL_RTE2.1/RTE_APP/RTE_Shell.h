#ifndef __RTE_SHELL_H
#define __RTE_SHELL_H
#ifdef __cplusplus
extern "C" {
#endif
	#include "RTE_Include.h"
	typedef enum
	{
		SHELL_NO_ERR = 0,
		CMDLINE_BAD_CMD = -1,
		CMDLINE_TOO_MANY_ARGS = -2,
		CMDLINE_TOO_FEW_ARGS = -3,
		CMDLINE_INVALID_ARG = -4,
		CMDLINE_NO_CMDS = -5,
		CDMLINE_ADD_FAIL = -6,
		CDMLINE_DELETE_FAIL = -7,
	}RTE_Shell_Err_e;
	typedef struct
	{
		//! A pointer to a string containing the name of the command.
		const char *pcCmd;
		//! A function pointer to the implementation of the command.
		int (*pfnCmdLine)(int argc, char *argv[]);
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
	extern RTE_Shell_Err_e RTE_Shell_AddCommand(const char *cmd,int (*func)(int argc, char *argv[]),const char *help);
	extern RTE_Shell_Err_e RTE_Shell_DeleteCommand(const char *cmd);
	extern void RTE_Shell_Init(void);
  extern void RTE_Shell_Poll(void);
#ifdef __cplusplus
}
#endif
#endif
