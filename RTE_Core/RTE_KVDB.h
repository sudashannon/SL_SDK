#ifndef __RTE_KVDB_H
#define __RTE_KVDB_H
#ifdef __cplusplus
extern "C" {
#endif
	#include "RTE_Config.h"
	#if RTE_USE_KVDB == 1
	#if RTE_USE_OS == 1
	#include "cmsis_os2.h"
	extern osMutexId_t MutexIDKVDB;
	#endif
	#include <stdbool.h>
	#include <stdint.h>
	#include <string.h>
	/* EasyFlash software version number */
	#define EF_SW_VERSION                  "4.0.0"
	#define EF_SW_VERSION_NUM              0x40000
	/* the ENV max name length must less then it */
	#define EF_ENV_NAME_MAX                          32
	typedef struct _ef_env {
		char *key;
		void *value;
		size_t value_len;
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
	/* the flash sector current status */
	typedef enum {
		EF_SECTOR_EMPTY,
		EF_SECTOR_USING,
		EF_SECTOR_FULL,
	} EfSecrorStatus;
	#if EF_USING_ENV
	extern EfErrCode ef_env_init(ef_env const *default_env, size_t default_env_size);
	/* only supported on ef_env.c */
	size_t ef_get_env_blob(const char *key, void *value_buf, size_t buf_len, size_t *value_len);
	EfErrCode ef_set_env_blob(const char *key, const void *value_buf, size_t buf_len);

	/* ef_env.c, ef_env_legacy_wl.c and ef_env_legacy.c */
	EfErrCode ef_load_env(void);
	uint8_t ef_print_env(char **keylist);
	char *ef_get_env(const char *key);
	EfErrCode ef_set_env(const char *key, const char *value);
	EfErrCode ef_del_env(const char *key);
	EfErrCode ef_save_env(void);
	EfErrCode ef_env_set_default(void);
	size_t ef_get_env_write_bytes(void);
	EfErrCode ef_set_and_save_env(const char *key, const char *value);
	EfErrCode ef_del_and_save_env(const char *key);
	#endif

	#if EF_USING_IAP
	extern EfErrCode ef_iap_init(void);
	/* ef_iap.c */
	EfErrCode ef_erase_bak_app(size_t app_size);
	EfErrCode ef_erase_user_app(uint32_t user_app_addr, size_t user_app_size);
	EfErrCode ef_erase_spec_user_app(uint32_t user_app_addr, size_t app_size,
									 EfErrCode (*app_erase)(uint32_t addr, size_t size));
	EfErrCode ef_erase_bl(uint32_t bl_addr, size_t bl_size);
	EfErrCode ef_write_data_to_bak(uint8_t *data, size_t size, size_t *cur_size,
								   size_t total_size);
	EfErrCode ef_copy_app_from_bak(uint32_t user_app_addr, size_t app_size);
	EfErrCode ef_copy_spec_app_from_bak(uint32_t user_app_addr, size_t app_size,
										EfErrCode (*app_write)(uint32_t addr, const uint32_t *buf, size_t size));
	EfErrCode ef_copy_bl_from_bak(uint32_t bl_addr, size_t bl_size);
	uint32_t ef_get_bak_app_start_addr(void);
	#endif
	
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
