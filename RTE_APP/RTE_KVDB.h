#ifndef __RTE_KVDB_H
#define __RTE_KVDB_H
#ifdef __cplusplus
extern "C" {
#endif
	#include "RTE_Include.h"
	#if RTE_USE_KVDB
	#if RTE_USE_OS
		extern osMutexId_t MutexIDKVDB;
	#endif
	// <o> KVDB_SIZE
	// <i> KVDB环境变量总大小(以KVDB_ERASE_MIN_SIZE为最小单位 单位：K)
	#ifndef KVDB_SIZE
	#if KVDB_USE_PFS == 0
		/* ENV area total bytes size in normal mode. */
		#define KVDB_SIZE          (1 * KVDB_ERASE_MIN_SIZE)      /* 128K */
	#else
		/* ENV area total bytes size in power fail safeguard mode. */
		#define KVDB_SIZE          (2 * KVDB_ERASE_MIN_SIZE)      /* 256K */
	#endif
	#endif
	typedef struct _ef_env {
    char *key;
    char *value;
	} ef_env, *ef_env_t;
	/* EasyFlash error code */
	typedef enum {
			EF_NO_ERR,
			EF_ERASE_ERR,
			EF_READ_ERR,
			EF_WRITE_ERR,
			EF_ENV_NAME_ERR,
			EF_ENV_NAME_EXIST,
			EF_ENV_FULL,
			EF_ENV_INIT_FAILED,
	} EfErrCode;
	
	
	/* ef_env.c ef_env_wl.c */
	EfErrCode ef_load_env(void);
	void ef_print_env(void);
	char *ef_get_env(const char *key);
	EfErrCode ef_set_env(const char *key, const char *value);
	EfErrCode ef_del_env(const char *key);
	EfErrCode ef_save_env(void);
	EfErrCode ef_env_set_default(void);
	size_t ef_get_env_write_bytes(void);
	EfErrCode ef_set_and_save_env(const char *key, const char *value);
	EfErrCode ef_del_and_save_env(const char *key);
	EfErrCode ef_env_init(ef_env const *default_env, size_t default_env_size);
	/* ef_port.c */
	extern EfErrCode ef_port_read(uint32_t addr, uint32_t *buf, size_t size);
	extern EfErrCode ef_port_erase(uint32_t addr, size_t size);
	extern EfErrCode ef_port_write(uint32_t addr, const uint32_t *buf, size_t size);
	extern void ef_port_env_lock(void);
	extern void ef_port_env_unlock(void);
	#endif
#ifdef __cplusplus
}
#endif
#endif
