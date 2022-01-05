#include "rte_include.h"
#include "cmsis_os2.h"
#include "RTX_Config.h"
#include "fal.h"
#include "fota.h"
#include "ymodem.h"
#include "quadspi.h"
#include "driver_w25qxx.h"
#include "flashdb.h"

#define DEFAULT_DOWNLOAD_PART FOTA_FM_PART_NAME

static char* recv_partition = NULL;
static size_t update_file_total_size, update_file_cur_size;
static const struct fal_partition * dl_part = NULL;
static struct rym_ctx rctx = {0};

static enum rym_code ymodem_on_begin(struct rym_ctx *ctx, uint8_t *buf, size_t len)
{
    char *file_name, *file_size;

    /* calculate and store file size */
    file_name = (char *)&buf[0];
    file_size = (char *)&buf[strlen(file_name) + 1];
    update_file_total_size = atol(file_size);
    RTE_LOGI("Get Ymodem file_size:%d", update_file_total_size);

    update_file_cur_size = 0;
    /* Get download partition information and erase download partition data */
    if ((dl_part = fal_partition_find(recv_partition)) == NULL) {
        RTE_LOGE("Firmware download failed! Partition (%s) find error!", recv_partition);
        return RYM_CODE_CAN;
    }

    if (update_file_total_size > dl_part->len) {
        RTE_LOGE("Firmware is too large! File size (%d), '%s' partition size (%d)", update_file_total_size, recv_partition, dl_part->len);
        return RYM_CODE_CAN;
    }

    RTE_LOGI("Start erase. Size (%d)", update_file_total_size);

    /* erase DL section */
    if (fal_partition_erase(dl_part, 0, update_file_total_size) < 0) {
        RTE_LOGE("Firmware download failed! Partition (%s) erase error!", dl_part->name);
        return RYM_CODE_CAN;
    }

    return RYM_CODE_ACK;
}

static enum rym_code ymodem_on_data(struct rym_ctx *ctx, uint8_t *buf, size_t len)
{
    /* write data of application to DL partition  */
    if (fal_partition_write(dl_part, update_file_cur_size, buf, len) < 0) {
        RTE_LOGE("Firmware download failed! Partition (%s) write data error!", dl_part->name);
        return RYM_CODE_CAN;
    }
    update_file_cur_size += len;
    return RYM_CODE_ACK;
}

static void upgrade_status_set(bool upgrade_status)
{
    struct fdb_blob blob;
    extern struct fdb_kvdb kvdb;
    fdb_kv_set_blob(&kvdb, "upgrade_status", fdb_blob_make(&blob, &upgrade_status, sizeof(upgrade_status)));
}

static bool upgrade_status_get(void)
{
    struct fdb_blob blob;
    extern struct fdb_kvdb kvdb;
    bool upgrade_status = false;
    fdb_kv_get_blob(&kvdb, "upgrade_status", fdb_blob_make(&blob, &upgrade_status, sizeof(upgrade_status)));
    return upgrade_status;
}

int shell_upgrade_cmd(const shell_cmd_t *pcmd, int argc, char *const argv[])
{
    if (argc < 2) {
        shell_printf("You must input upgrade way!\r\n");
        return -1;
    }
    const char *way = argv[1];
    if (!strcmp(way, "ymodem")) {
        if (argc < 3)
        {
            recv_partition = DEFAULT_DOWNLOAD_PART;
            shell_printf("Default save firmware on download partition.\r\n");
        } else {
            const char *operator = argv[2];
            if (!strcmp(operator, "-p")) {
                if (argc < 3) {
                    shell_printf("Usage: upgrade ymodem -p <partiton name>.\r\n");
                    return -1;
                } else {
                    /* change default partition to save firmware */
                    recv_partition = argv[3];
                }
            } else {
                shell_printf("Usage: upgrade ymodem -p <partiton name>.\r\n");
                return -1;
            }
        }

        shell_printf("Warning: Ymodem has started! This operator will not recovery.\r\n");
        shell_printf("Please select the ota firmware file and use Ymodem to send.\r\n");
        upgrade_status_set(false);
        if (!rym_recv_on_device(&rctx, "com_0", ymodem_on_begin, ymodem_on_data, NULL, HAL_MAX_DELAY)) {
            shell_printf("Download firmware to flash success.\r\n");
            if (fota_part_fw_verify(recv_partition, NULL) >= 0) {
                shell_printf("Download firmware verify........[OK]\r\n");
                upgrade_status_set(true);
                shell_printf("Reset system and apply new firmware.\r\n");
                osDelay(1000);
                SCB->AIRCR = 0x05FA0004;
            } else {
                shell_printf("Download firmware verify........[FAILED]\r\n");
            }
        } else {
            shell_printf("Download firmware to flash fail.\r\n");
        }
    } else {
        shell_printf("Unknown upgrade way: %s!\r\n", way);
        return -1;
    }
    return 0;
}

SHELL_ADD_CMD(upgrade, shell_upgrade_cmd, "upgrade device in provided way, support ymodem.","\r\n");

static void ota_start_application(void)
{
	const struct fal_partition *part;
	uint32_t app_addr = 0;

	part = fal_partition_find(FOTA_APP_PART_NAME);
	if (part == NULL) {
		RTE_LOGE("Partition[%s] not found.", FOTA_APP_PART_NAME);
        return;
	}

    extern w25qxx_handle_t gs_handle;
    w25qxx_exit_qspi_mode(&gs_handle);
    app_addr = part->offset + 0x90000000;
    RTE_LOGI("app_addr: %x", app_addr);
    QSPI_CommandTypeDef s_command = {0};
    QSPI_MemoryMappedTypeDef s_mem_mapped_cfg = {0};

    s_command.InstructionMode          = QSPI_INSTRUCTION_1_LINE;
    s_command.AddressSize              = QSPI_ADDRESS_24_BITS;
    s_command.AlternateByteMode        = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DdrMode                  = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle         = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode                 = QSPI_SIOO_INST_EVERY_CMD;

    s_command.Instruction              = 0xEB;
    s_command.AddressMode              = QSPI_ADDRESS_4_LINES;
    s_command.DataMode                 = QSPI_DATA_4_LINES;
    s_command.DummyCycles              = 6;

    s_mem_mapped_cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;
    s_mem_mapped_cfg.TimeOutPeriod = 0;

    if(HAL_QSPI_MemoryMapped(&hqspi, &s_command, &s_mem_mapped_cfg) != HAL_OK) {
        RTE_LOGE("exe flash memory mapped failed!");
        return;
    }

	void (*app_jump)(void);
	__set_PRIMASK(1);
	HAL_RCC_DeInit();
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;
	for (uint8_t i = 0; i < 8; i++) {
		NVIC->ICER[i]=0xFFFFFFFF;
		NVIC->ICPR[i]=0xFFFFFFFF;
	}
	app_jump = (void (*)(void)) (*((uint32_t *) (app_addr + 4)));
	__set_MSP(*(uint32_t *)app_addr);
	__set_CONTROL(0);
	__set_PRIMASK(0);
	app_jump();
}

__NO_RETURN void ota_thread(void *argument)
{
    fota_pahead_t firmware_pa_info = {0};
    fota_pahead_t default_pa_info = {0};
    RTE_LOGI("enter OTA mode success!");
    /* Wait for long press key to enter recovery mode */

    /* Check if last upgrade is over */
    if (upgrade_status_get() != true) {
        RTE_LOGE("The devices doesn't upgrade success!");
        goto recover_default_image;
    }
	/* Firmware partition verify */
	if (fota_part_fw_verify(FOTA_FM_PART_NAME, &firmware_pa_info) != FOTA_NO_ERR) {
        RTE_LOGE("The devices firmware verify failed!");
		goto recover_default_image;
    }
	/* Check upgrade status */
	if (fota_check_upgrade(&firmware_pa_info) == 0) {
        RTE_LOGI("The devices doesn't need to be upgraded!");
        goto jump_user_app;
    }
	RTE_LOGI("Application need upgrade.");
    /* The firmware is newer, copy it to user_app region */
	/* Implement upgrade, copy firmware partition to app partition */
	if (fota_upgrade_firmware(FOTA_FM_PART_NAME, &firmware_pa_info) != FOTA_NO_ERR) {
        RTE_LOGE("upgrade failed!");
		goto jump_user_app;
    }

	/* Update new application verison in RBL file of firmware partition */
	if (fota_update_current_version(&firmware_pa_info) != FOTA_NO_ERR)
		goto jump_user_app;
jump_user_app:
    ota_start_application();
    /* User_app jump failed, use default app */
recover_default_image:
    /* Implement upgrade, copy default partition to app partition */
    if (fota_part_fw_verify(FOTA_DF_PART_NAME, &default_pa_info) == FOTA_NO_ERR) {
        /* Copy default partition to firmware partition */
        const struct fal_partition *dst_part = fal_partition_find(FOTA_FM_PART_NAME); 
        const struct fal_partition *src_part = fal_partition_find(FOTA_DF_PART_NAME);
        if (fal_partition_erase(dst_part, 0, dst_part->len) >= 0) {
            int clone_pos = 0;
            int clone_len = 0, clone_tol_len;
            uint8_t *buf = rte_malloc(4096);
            if (dst_part->len < src_part->len)
                clone_tol_len = dst_part->len;
            else
                clone_tol_len = src_part->len;
            shell_printf("\r\n");
            while ((clone_pos < clone_tol_len) && (buf != NULL)) {
                clone_len = fal_partition_read(src_part, clone_pos, buf, 4096);
                if (clone_len < 0) {
                    RTE_LOGE("read default partition failed, recovery stop!");
                    break;
                }

                if (fal_partition_write(dst_part, clone_pos, buf, clone_len) < 0) {
                    RTE_LOGE("write firmware partition failed, recovery stop!");
                    break;
                }

                shell_printf("#");
                clone_pos += clone_len;
            }
            shell_printf("\r\n");
            if (clone_pos >= clone_tol_len) {
                RTE_LOGE("recovery firmware partition success, total %d bytes!", clone_tol_len);
                upgrade_status_set(true);
                osDelay(1000);
                SCB->AIRCR = 0x05FA0004;
            } else {
                RTE_LOGE("recovery firmware partition failed!");
            }
            if (buf)
                rte_free(buf);
        }
    }
    RTE_LOGI("Boot application failed, entry shell mode.");
    osThreadExit();
}
