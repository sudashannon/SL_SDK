/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-09-22     Warfalcon    first version
 */

#include "rte_include.h"
#include "cmsis_os2.h"

#include "fal.h"
#include "tinycrypt.h"
#include "fastlz.h"
#include "fota.h"
#include "flashdb.h"

#ifndef FOTA_CRYPTO_BUFF_SIZE
#define FOTA_CRYPTO_BUFF_SIZE				4096
#endif

#ifndef FOTA_DEPRESSED_BUFF_SIZE
#define FOTA_DEPRESSED_BUFF_SIZE			8192
#endif

/**
 * AES256 encryption algorithm option
 */
#ifndef FOTA_ALGO_AES_IV
#define FOTA_ALGO_AES_IV  				"e3r4y6p0e3r4y6p0"
#endif

#ifndef FOTA_ALGO_AES_KEY
#define FOTA_ALGO_AES_KEY 				"lziebg,z@fjebzdllziebg,z@fjebzdl"
#endif

#define FNV_SEED  0x811c9dc5

static uint32_t fnv1a_r(uint8_t oneByte, uint32_t hash)
{
    return ((oneByte ^ hash) * 0x1000193);
}

static uint32_t calc_fnv1a_r(uint8_t *data, uint32_t hash, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++) {
        hash = fnv1a_r(data[i], hash);
    }
    return hash;
}

int fota_part_fw_verify(const char *paname, fota_pahead_t *pa_info)
{
#define FOTA_CRC_BUFF_SIZE		4096
#define FOTA_CRC_INIT_VAL		0xffffffff

	int fota_res = FOTA_NO_ERR;
	const struct fal_partition *part;
	fota_pahead_t pahead;
	uint8_t *body_buf = NULL;
	uint32_t body_crc = FOTA_CRC_INIT_VAL;
	uint32_t hdr_crc;

	if (paname == NULL) {
		RTE_LOGE("Invaild paramenter input!");
		fota_res = FOTA_GENERAL_ERR;
		goto __exit_partition_verify;
	}

	part = fal_partition_find(paname);
	if (part == NULL) {
		RTE_LOGE("Partition[%s] not found.", paname);
		fota_res = FOTA_GENERAL_ERR;
		goto __exit_partition_verify;
	}

	/* read the head of RBL files */
	if (fal_partition_read(part, 0, (uint8_t *)&pahead, sizeof(fota_pahead_t)) < 0) {
		RTE_LOGE("Partition[%s] read error!", part->name);
		fota_res = FOTA_PAREAD_ERR;
		goto __exit_partition_verify;
	}

	RTE_LOGI("Partition[%s] type %s", part->name, pahead.type);
	RTE_LOGI("Partition[%s] fm_time %s", part->name, pahead.fm_time);
	RTE_LOGI("Partition[%s] app_paname %s", part->name, pahead.app_paname);
	RTE_LOGI("Partition[%s] firmware_version %s", part->name, pahead.download_version);
	RTE_LOGI("Partition[%s] Code raw size %d", part->name, pahead.raw_size);
	RTE_LOGI("Partition[%s] Code package size %d ", part->name, pahead.pkg_size);
	extern uint32_t fota_crc(uint8_t *buf, uint32_t len);
	hdr_crc = fota_crc((uint8_t *)&pahead, sizeof(fota_pahead_t) - 4);
	if (hdr_crc != pahead.head_crc) {
		RTE_LOGE("Partition[%s] head CRC32 error!", part->name);
		fota_res = FOTA_FW_VERIFY_FAILED;
		goto __exit_partition_verify;
	}

	if (strcmp(pahead.type, "RBL") != 0) {
		RTE_LOGE("Partition[%s] type[%s] not surport.", part->name, pahead.type);
		fota_res = FOTA_CHECK_FAILED;
		goto __exit_partition_verify;
	}

	if (fal_partition_find(pahead.app_paname) == NULL) {
		RTE_LOGE("Partition[%s] not found.", pahead.app_paname);
		fota_res = FOTA_FW_VERIFY_FAILED;
		goto __exit_partition_verify;
	}

	body_buf = rte_malloc(FOTA_CRC_BUFF_SIZE);
	if (body_buf == NULL) {
		RTE_LOGE("Not enough memory for body CRC32 verify.");
		fota_res = FOTA_NO_MEM_ERR;
		goto __exit_partition_verify;
	}

	for (int body_pos = 0; body_pos < pahead.pkg_size;) {
		int body_read_len = fal_partition_read(part, sizeof(fota_pahead_t) + body_pos, body_buf, FOTA_CRC_BUFF_SIZE);
		if (body_read_len > 0) {
            if ((body_pos + body_read_len) > pahead.pkg_size) {
                body_read_len = pahead.pkg_size - body_pos;
            }

			extern uint32_t fota_step_crc(uint32_t crc, uint8_t *buf, uint32_t len);
			body_crc = fota_step_crc(body_crc, body_buf, body_read_len);
			body_pos = body_pos + body_read_len;
		} else {
			RTE_LOGE("Partition[%s] read error!", part->name);
			fota_res = FOTA_PAREAD_ERR;
			goto __exit_partition_verify;
		}
	}
	body_crc = body_crc ^ FOTA_CRC_INIT_VAL;

	if (body_crc != pahead.code_crc) {
		RTE_LOGE("Partition[%s] firmware integrity verify failed.", part->name);
		fota_res = FOTA_FW_VERIFY_FAILED;
		goto __exit_partition_verify;
	}

__exit_partition_verify:
	if (fota_res == FOTA_NO_ERR)
	{
        if (pa_info)
            memcpy(pa_info, &pahead, sizeof(fota_pahead_t));
		RTE_LOGI("partition[%s] verify success!", part->name);
	} else {
        if (pa_info)
            memset(pa_info, 0x0, sizeof(fota_pahead_t));
		RTE_LOGE("Partition[%s] verify failed!", part->name);
	}

	if (body_buf)
		rte_free(body_buf);

	return fota_res;
}

static int fota_get_main_version(char *ver_str)
{
    char *main_ver_str = ver_str;
    char *dot_pos = strstr(ver_str, ".");
    if (dot_pos) {
        *(dot_pos) = 0;
        int retval = atoi(main_ver_str);
        *(dot_pos) = '.';
        return retval;
    }
    return 0;
}

static int fota_get_sub_version(char *ver_str)
{
    char *dot_pos = strstr(ver_str, ".");
    if (dot_pos) {
        int retval = atoi(dot_pos + 1);
        return retval;
    }
    return 0;
}

int fota_check_upgrade(fota_pahead_t *cur_pa_info)
{
	int is_upgrade = 0;
	extern struct fdb_kvdb kvdb;
	char *current_version = fdb_kv_get(&kvdb, "current_version");
    RTE_LOGI("current version %s download version %s", current_version ? current_version : "N/A", cur_pa_info->download_version);
	if (current_version == NULL) {
		is_upgrade = 1;
	} else {
        int cur_main_ver = fota_get_main_version(current_version);
        int cur_sub_ver = fota_get_sub_version(current_version);
        int down_main_ver = fota_get_main_version(cur_pa_info->download_version);
        int down_sub_ver = fota_get_sub_version(cur_pa_info->download_version);
        if (cur_main_ver < down_main_ver ||
            (cur_main_ver == down_main_ver &&
             cur_sub_ver < down_sub_ver)) {
            is_upgrade = 1;
        }
    }

	return is_upgrade;
}

int fota_update_current_version(fota_pahead_t *cur_pa_info)
{
	int fota_res = FOTA_NO_ERR;
	extern struct fdb_kvdb kvdb;
	if (fdb_kv_set(&kvdb, "current_version", cur_pa_info->download_version) != FDB_NO_ERR) {
		RTE_LOGE("Update partition[%p] version failed.", cur_pa_info);
		fota_res = FOTA_GENERAL_ERR;
	}

	return fota_res;
}


int fota_erase_part(const char *part_name, uint32_t size)
{
	int fota_res = FOTA_NO_ERR;
	const struct fal_partition *part;

	part = fal_partition_find(part_name);
	if (part == NULL) {
		RTE_LOGE("Erase partition[%s] not found.", part_name);
		fota_res = FOTA_FW_VERIFY_FAILED;
		goto __exit_partition_erase;
	}

    RTE_LOGI("Partition[%s] erase start:", part->name);
    if (size)
        fota_res = fal_partition_erase(part, 0, size);
    else
        fota_res = fal_partition_erase_all(part);
	if (fota_res < 0) {
		RTE_LOGE("Partition[%s] erase failed!", part->name);
		fota_res = FOTA_PAERASE_ERR;
		goto __exit_partition_erase;
	} else {
        fota_res = FOTA_NO_ERR;
    }

__exit_partition_erase:
	if (fota_res == FOTA_NO_ERR) {
		RTE_LOGD("Partition[%s] erase %d bytes success!", part->name, size);
	}
	return fota_res;
}

static int fota_write_part(const struct fal_partition *part, int fw_pos, uint8_t *fw_buf, int fw_len)
{
	int fota_res = FOTA_NO_ERR;

	if (fal_partition_write(part, fw_pos, fw_buf, fw_len) < 0) {
		RTE_LOGE("Partition[%s] write failed!", part->name);
		fota_res = FOTA_PAWRITE_ERR;
	}
	return fota_res;
}

static int fota_read_part(const struct fal_partition *part, int read_pos, uint8_t *read_buf, uint32_t read_len)
{
	int fota_err = FOTA_NO_ERR;

	if ((part == NULL) || (read_buf == NULL)) {
		fota_err = FOTA_GENERAL_ERR;
		return fota_err;
	}

	memset(read_buf, 0x0, read_len);
	/* Read partion data to decrypt buffer */
	read_len = fal_partition_read(part, read_pos, read_buf, read_len);
	if (read_len <= 0) {
		fota_err = FOTA_PAREAD_ERR;
		return fota_err;
	}

	return read_len;
}

int fota_upgrade_firmware(const char *firmware_part_name, fota_pahead_t *firmware_package_info)
{
	int fota_err = FOTA_NO_ERR;
	/* data for fal */
	const struct fal_partition *firmware_part = NULL, *app_part = NULL, *decrypt_part = NULL;
    uint8_t *read_buf = rte_malloc(FOTA_CRYPTO_BUFF_SIZE);
	if (read_buf == NULL) {
		RTE_LOGE("Not enough memory for firmware read buffer.");
		fota_err = FOTA_NO_MEM_ERR;
		goto __exit_upgrade;
	}
	/* data for crypt */
	tiny_aes_context aes_ctx = {0};
	uint8_t aes_iv[16 + 1] = {0};
	uint8_t *decrypt_buf = NULL;
	/* data for depressed */
	uint8_t *depressed_buf = NULL;
	/* data for full firmware verify */
	uint32_t hashvalue = FNV_SEED;

	if (firmware_part_name == NULL || firmware_package_info == NULL) {
		RTE_LOGE("Invaild paramenter input!");
		fota_err = FOTA_GENERAL_ERR;
		goto __exit_upgrade;
	}

	firmware_part = fal_partition_find(firmware_part_name);
	if (firmware_part == NULL) {
		RTE_LOGE("Firmware partition[%s] not found.", firmware_part_name);
		fota_err = FOTA_GENERAL_ERR;
		goto __exit_upgrade;
	}

	app_part = fal_partition_find(firmware_package_info->app_paname);
	if (app_part == NULL) {
		RTE_LOGE("App partition[%s] not found.", firmware_package_info->app_paname);
		fota_err = FOTA_GENERAL_ERR;
		goto __exit_upgrade;
	}

	/* Application partition erase */
	fota_err = fota_erase_part(firmware_package_info->app_paname, firmware_package_info->raw_size);
	if (fota_err != FOTA_NO_ERR) {
		RTE_LOGE("Erase partition[%s] failed.", firmware_package_info->app_paname);
		goto __exit_upgrade;
	}

	/* First, check if the partition is cryptographic */
	/* We only support aes_256 algorithm */
	if ((firmware_package_info->fota_algo & FOTA_CRYPT_STAT_MASK) == FOTA_CRYPT_ALGO_AES256) {
		decrypt_buf = rte_malloc(FOTA_CRYPTO_BUFF_SIZE);
		if (decrypt_buf == NULL) {
			RTE_LOGE("Not enough memory for decrypt buffer.");
			fota_err = FOTA_NO_MEM_ERR;
			goto __exit_upgrade;
		}
		memset(aes_iv, 0x0, strlen(FOTA_ALGO_AES_IV) + 1);
		memcpy(aes_iv, FOTA_ALGO_AES_IV, strlen(FOTA_ALGO_AES_IV));
		tiny_aes_setkey_dec(&aes_ctx, (uint8_t *)FOTA_ALGO_AES_KEY, 256);
	} else if ((firmware_package_info->fota_algo & FOTA_CRYPT_STAT_MASK) != 0) {
		RTE_LOGE("not support %d crypto method", firmware_package_info->fota_algo & FOTA_CRYPT_STAT_MASK);
		fota_err = FOTA_GENERAL_ERR;
		goto __exit_upgrade;
	} else {
		/* Disable depress */
		decrypt_buf = read_buf;
	}
	/* Second, check if the partition is compressed */
	if ((firmware_package_info->fota_algo & FOTA_CMPRS_STAT_MASK) == FOTA_CMPRS_ALGO_FASTLZ) {
		depressed_buf = rte_malloc(FOTA_DEPRESSED_BUFF_SIZE);
		if (depressed_buf == NULL) {
			RTE_LOGE("Not enough memory for depressed buffer.");
			fota_err = FOTA_NO_MEM_ERR;
			goto __exit_upgrade;
		}
		if ((firmware_package_info->fota_algo & FOTA_CMPRS_STAT_MASK)) {
			decrypt_part = fal_partition_find("dc_area");
			if (decrypt_part == NULL) {
				RTE_LOGE("decrypt partition[%s] not found.", decrypt_part);
				fota_err = FOTA_GENERAL_ERR;
				goto __exit_upgrade;
			}
		}
	} else if ((firmware_package_info->fota_algo & FOTA_CMPRS_STAT_MASK) != 0) {
		RTE_LOGE("not support %d compress method", firmware_package_info->fota_algo & FOTA_CMPRS_STAT_MASK);
		fota_err = FOTA_GENERAL_ERR;
		goto __exit_upgrade;
	}

	RTE_LOGI("Start to copy firmware from %s to %s partition", firmware_part->name, firmware_package_info->app_paname);
	RTE_LOGI("initial bin size: %d, package data size: %d", firmware_package_info->raw_size, firmware_package_info->pkg_size);

	/* Pass through the header */
	uint32_t firmware_read_pos = 96;
	uint32_t firmware_write_pos  = 0;
	uint32_t read_len = 0, write_len = 0, add_len = 0, depressed_len = 0;
	int32_t left_firmware_size = firmware_package_info->pkg_size;
	while (left_firmware_size > 0) {
		read_len = fota_read_part(
						firmware_part, firmware_read_pos, read_buf,
						(FOTA_CRYPTO_BUFF_SIZE < left_firmware_size) ? FOTA_CRYPTO_BUFF_SIZE : left_firmware_size);
		if (read_len <= 0) {
			RTE_LOGE("read firmware failed!");
			fota_err = FOTA_PAREAD_ERR;
			goto __exit_upgrade;
		}
		if ((firmware_package_info->fota_algo & FOTA_CRYPT_STAT_MASK)) {
			/* Start decrypt */
			tiny_aes_crypt_cbc(&aes_ctx, AES_DECRYPT, read_len, aes_iv, read_buf, decrypt_buf);
		}
		write_len = read_len;
		if ((firmware_package_info->fota_algo & FOTA_CMPRS_STAT_MASK)) {
			if (fota_write_part(decrypt_part, firmware_write_pos, decrypt_buf, write_len) < 0) {
				RTE_LOGE("write decrypt firmware failed!");
				fota_err = FOTA_PAREAD_ERR;
				goto __exit_upgrade;
			}
			firmware_write_pos  += write_len;
			left_firmware_size -= read_len;
			firmware_read_pos += read_len;
		} else {
			if (fota_write_part(app_part, firmware_write_pos, decrypt_buf, write_len) < 0) {
				RTE_LOGE("write app firmware failed!");
				fota_err = FOTA_PAREAD_ERR;
				goto __exit_upgrade;
			}
			firmware_write_pos  += write_len;
			left_firmware_size -= read_len;
			firmware_read_pos += read_len;
			if ((firmware_package_info->fota_algo & FOTA_CRYPT_STAT_MASK)) {
				if (left_firmware_size <= 0) {
					add_len = firmware_package_info->raw_size % 16;
				}
			}
			hashvalue = calc_fnv1a_r(decrypt_buf, hashvalue, write_len - add_len);
		}
		shell_printf(">");
	}

	if ((firmware_package_info->fota_algo & FOTA_CMPRS_STAT_MASK)) {
		firmware_read_pos = 0;
		firmware_write_pos  = 0;
		read_len = 0, write_len = 0, add_len = 0;
		left_firmware_size = firmware_package_info->raw_size;
		shell_printf("\r\n");
		while (left_firmware_size > 0) {
			/* Read chunk size */
			read_len = fota_read_part(decrypt_part, firmware_read_pos, read_buf, 4);
			if (read_len <= 0) {
				RTE_LOGE("read firmware failed!");
				fota_err = FOTA_PAREAD_ERR;
				goto __exit_upgrade;
			}
			/* Calculate chunk size */
			uint32_t chunk_size = read_buf[0] * (1 << 24) + read_buf[1] * (1 << 16) + read_buf[2] * (1 << 8) + read_buf[3];
			/* Read full compressed chunk */
			read_len = fota_read_part(decrypt_part, firmware_read_pos + 4, read_buf, chunk_size);
			if (read_len <= 0) {
				RTE_LOGE("read firmware failed!");
				fota_err = FOTA_PAREAD_ERR;
				goto __exit_upgrade;
			}
			/* Depress this chunk */
			depressed_len = fastlz_decompress(read_buf, chunk_size, depressed_buf, FOTA_DEPRESSED_BUFF_SIZE);
			if (depressed_len == 0) {
				RTE_LOGE("depress firmware failed!");
				fota_err = FOTA_PAREAD_ERR;
				goto __exit_upgrade;
			}
			/* Write depressed data to app partition */
			if (fota_write_part(app_part, firmware_write_pos, depressed_buf, depressed_len) < 0) {
				RTE_LOGE("write app firmware failed!");
				fota_err = FOTA_PAREAD_ERR;
				goto __exit_upgrade;
			}
			firmware_read_pos += (read_len + 4);
			firmware_write_pos  += depressed_len;
			left_firmware_size -= depressed_len;
			hashvalue = calc_fnv1a_r(depressed_buf, hashvalue, depressed_len);
			shell_printf(">");
		}
	}
__end_upgrade:
    shell_printf("\r\n");
	RTE_LOGI("caculated hash value %x, expected %x, write size %d",
				hashvalue, firmware_package_info->hash_val, firmware_write_pos);
	if (hashvalue != firmware_package_info->hash_val) {
		RTE_LOGE("hash of image check failed.");
		fota_err = FOTA_GENERAL_ERR;
	}

__exit_upgrade:
	if (decrypt_buf && (firmware_package_info->fota_algo & FOTA_CRYPT_STAT_MASK)) {
		rte_free(decrypt_buf);
	}
	if (depressed_buf && (firmware_package_info->fota_algo & FOTA_CMPRS_STAT_MASK)) {
		rte_free(depressed_buf);
	}
    if (read_buf) {
        rte_free(read_buf);
	}
	if (fota_err == FOTA_NO_ERR) {
		RTE_LOGI("Upgrade success!");
	}
	return fota_err;
}


#if RTE_SHELL_ENABLE == 1

int shell_cmd_fota(int argc, char *const argv[])
{
	char put_buf[24];
	char paname[2][FAL_DEV_NAME_MAX] = {
        {FOTA_FM_PART_NAME},
        {FOTA_DF_PART_NAME}
    };

	const char* help_info[] = {
            [0]     = "fota probe                       - probe RBL file of partiton",
            [1]     = "fota show partition addr size    - show 'size' bytes starting at 'addr'",
            [2]     = "fota clone des_part src_part     - clone src partition to des partiton",
            [3]     = "fota exec                        - execute application program",
    };

	if (argc < 2) {
        shell_printf("Usage:\r\n");
        for (int i = 0; i < sizeof(help_info) / sizeof(char*); i++)
        {
            shell_printf("%s\r\n", help_info[i]);
        }
        shell_printf("\r\n");
    } else {
    	const char *operator = argv[1];
		if (!strcmp(operator, "probe")) {
            for (int i = 0; i < 2; i++) {
                fota_pahead_t pa_info = {0};
                if (fota_part_fw_verify(&paname[i][0], &pa_info) == FOTA_NO_ERR) {
                    RTE_LOGI("===== RBL of %s partition =====", &paname[i][0]);
                    RTE_LOGI("| App partition name | %s |", pa_info.app_paname);

					memset(put_buf, 0x0, sizeof(put_buf));
					if ((pa_info.fota_algo & FOTA_CRYPT_STAT_MASK) == FOTA_CRYPT_ALGO_AES256) {
						strncpy(put_buf, " AES", 4);
					} else if ((pa_info.fota_algo & FOTA_CRYPT_STAT_MASK) == FOTA_CRYPT_ALGO_XOR) {
						strncpy(put_buf, " XOR", 4);
					} else {
                        strncpy(put_buf, "NONE", 4);
                    }

					if ((pa_info.fota_algo & FOTA_CMPRS_STAT_MASK) == FOTA_CMPRS_ALGO_GZIP) {
						strncpy(&put_buf[strlen(put_buf)], " && GLZ", 7);
					} else if ((pa_info.fota_algo & FOTA_CMPRS_STAT_MASK) == FOTA_CMPRS_ALGO_FASTLZ) {
						strncpy(&put_buf[strlen(put_buf)], " && FLZ", 7);
					}

					if (strlen(put_buf) <= 0) {
						strncpy(put_buf, "None", 4);
					}
					RTE_LOGI("| Algorithm mode     | %s |", put_buf);
					RTE_LOGI("| Firmware version   | %s |", pa_info.download_version);
					RTE_LOGI("| Code raw size      | %11d |", pa_info.raw_size);
                    RTE_LOGI("| Code package size  | %11d |", pa_info.pkg_size);
                    RTE_LOGI("| Build Timestamp    | %11d |", *((uint32_t *)(&pa_info.fm_time[2])));
                }
            }
        } else if (!strcmp(operator, "show")) {
       		const struct fal_partition *part;
       		const char *paname = argv[2];

			uint32_t addr = strtol(argv[3], NULL, 0);
			uint32_t size = strtol(argv[4], NULL, 0);
			uint8_t buf[16];


			part = fal_partition_find(paname);
			if (part != NULL) {
				while (size > 16) {
					fal_partition_read(part, addr, buf, 16);

					shell_printf("%08X: ", addr);
					for (int i = 0; i < 16; i++) {
						shell_printf("%02X ", buf[i]);
					}
					shell_printf("\r\n");

					size -= 16;
					addr += 16;
				}

				fal_partition_read(part, addr, buf, size);
				shell_printf("%08X: ", addr);
				for (int i = 0; i < size; i++) {
					shell_printf("%02X ", buf[i]);
				}
				shell_printf("\r\n");
			} else {
				shell_printf("%s partition is not exist!\r\n", paname);
			}
        } else if (!strcmp(operator, "clone")) {
       		const char *dst_paname = argv[2];
			const char *src_paname = argv[3];
			const struct fal_partition *dst_part;
			const struct fal_partition *src_part;

			dst_part = fal_partition_find(dst_paname);
			src_part = fal_partition_find(src_paname);
			if (dst_part == NULL || src_part == NULL) {
				if (dst_part == NULL)
					shell_printf("%s partition is not exist!\r\n", dst_paname);

				if (src_part == NULL)
					shell_printf("%s partition is not exist!\r\n", src_paname);
			} else {
				shell_printf("Clone %s partition to %s partition:\r\n", src_paname, dst_paname);
				if (fal_partition_erase(dst_part, 0, dst_part->len) >= 0) {
					int clone_pos = 0;
					int clone_len = 0, clone_tol_len;
					uint8_t *buf = rte_malloc(4096);

					if (dst_part->len < src_part->len)
						clone_tol_len = dst_part->len;
					else
						clone_tol_len = src_part->len;

					while ((clone_pos < clone_tol_len) && (buf != NULL)) {
						clone_len = fal_partition_read(src_part, clone_pos, buf, 4096);
						if (clone_len < 0) {
							shell_printf("\nread %s partition failed, clone stop!\r\n", src_paname);
							break;
						}

						if (fal_partition_write(dst_part, clone_pos, buf, clone_len) < 0) {
							shell_printf("\nwrite %s partition failed, clone stop!\r\n", dst_paname);
							break;
						}

						shell_printf("#");
						clone_pos += clone_len;
					}

					if (clone_pos >= clone_tol_len)
						shell_printf("\r\nClone partition success, total %d bytes!\r\n", clone_tol_len);
					else
						shell_printf("\r\nClone partition failed!\r\n");

					if (buf)
						rte_free(buf);
				}
			}
		} else {
            shell_printf("Usage:\r\n");
            for (int i = 0; i < sizeof(help_info) / sizeof(char*); i++) {
                shell_printf("%s\r\n", help_info[i]);
            }
            shell_printf("\r\n");
		}
    }
    return 0;
}

SHELL_ADD_CMD(fota, shell_cmd_fota,
                    "fota function.","\r\n");
#endif
