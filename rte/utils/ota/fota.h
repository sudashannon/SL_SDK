/*
 * File      : fota.h
 * COPYRIGHT (C) 2012-2018, Shanghai Real-Thread Technology Co., Ltd
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-09-22     warfalcon    the first version
 */

#ifndef _FOTA_H_
#define _FOTA_H_

#include <stdint.h>

/* FOTA application partition name */
#ifndef FOTA_APP_PART_NAME
#define FOTA_APP_PART_NAME   "user_app"
#endif

/* FOTA download partition name */
#ifndef FOTA_FM_PART_NAME
#define FOTA_FM_PART_NAME    "fm_area"
#endif

/* FOTA default partition name */
#ifndef RT_FOTA_DF_PART_NAME
#define FOTA_DF_PART_NAME    "df_area"
#endif

typedef struct {
	char type[4];
	uint16_t fota_algo;
	uint8_t fm_time[6];
	char app_paname[16];
	char download_version[24];
	char current_version[24];
	uint32_t code_crc;
	uint32_t hash_val;
	uint32_t raw_size;
	uint32_t pkg_size;
	uint32_t head_crc;
} fota_pahead_t;

/**
 * FOTA firmware encryption algorithm and compression algorithm
 */
enum fota_algo
{
    FOTA_CRYPT_ALGO_NONE    = 0x0L,               /**< no encryption algorithm and no compression algorithm */
    FOTA_CRYPT_ALGO_XOR     = 0x1L,               /**< XOR encryption */
    FOTA_CRYPT_ALGO_AES256  = 0x2L,               /**< AES256 encryption */
    FOTA_CMPRS_ALGO_GZIP    = 0x1L << 8,          /**< Gzip: zh.wikipedia.org/wiki/Gzip */
    FOTA_CMPRS_ALGO_QUICKLZ = 0x2L << 8,          /**< QuickLZ: www.quicklz.com */
    FOTA_CMPRS_ALGO_FASTLZ  = 0x3L << 8,          /**< FastLZ: fastlz.org/ */

    FOTA_CRYPT_STAT_MASK    = 0xFL,
    FOTA_CMPRS_STAT_MASK    = 0xFL << 8,
};
typedef enum fota_algo fota_algo_t;

/* FOTA error code */
typedef enum {
    FOTA_NO_ERR             =  0,
    FOTA_GENERAL_ERR        = -1,    /* general error */
    FOTA_CHECK_FAILED       = -2,    /* check failed */
    FOTA_ALGO_NOT_SUPPORTED = -3,    /* firmware algorithm not supported */
    FOTA_COPY_FAILED        = -4,    /* copy firmware to destination partition failed */
    FOTA_FW_VERIFY_FAILED   = -5,    /* firmware verify failed */
    FOTA_NO_MEM_ERR         = -6,    /* no memory */
    FOTA_PAREAD_ERR      = -7,    /* partition read error */
    FOTA_PAWRITE_ERR     = -8,    /* partition write error */
    FOTA_PAERASE_ERR     = -9,    /* partition erase error */
} fota_err_t;

int fota_part_fw_verify(const char *part_name, fota_pahead_t *package_info);
int fota_check_upgrade(fota_pahead_t *cur_package_info);
int fota_update_current_version(fota_pahead_t *cur_package_info);
int fota_erase_part(const char *part_name, uint32_t size);
int fota_upgrade_firmware(const char *firmware_part_name, fota_pahead_t *firmware_package_info);

#endif /* _FOTA_H_ */

