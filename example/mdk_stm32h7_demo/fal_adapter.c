
/**
 * @file fal_adapter.c
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 1.0.0
 * @date 2022-01-02
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "rte_include.h"
#include "fal.h"
#include "driver_w25qxx.h"
#include "cmsis_os2.h"

#define FLASH_ERASE_MIN_SIZE    (4 * 1024)
extern w25qxx_handle_t ex_flash_handle;
extern w25qxx_handle_t db_flash_handle;
static rte_mutex_t ex_flash_mutex_instance = {NULL};
static rte_mutex_t db_flash_mutex_instance = {NULL};

static int ex_fal_init(void)
{
    w25qxx_info_t info;
    uint8_t res = 0;
    /* get information */
    res = w25qxx_info(&info);
    if (res) {
        RTE_LOGE("w25qxx: get info failed.");
        return 1;
    } else {
        /* print chip information */
        RTE_LOGI("w25qxx: chip is %s.", info.chip_name);
        RTE_LOGI("w25qxx: manufacturer is %s.", info.manufacturer_name);
        RTE_LOGI("w25qxx: interface is %s.", info.interface);
        RTE_LOGI("w25qxx: driver version is %d.%d.", info.driver_version / 1000, (info.driver_version % 1000) / 100);
    }

    /* set chip type */
    res = w25qxx_set_type(&ex_flash_handle, W25Q64);
    if (res) {
        RTE_LOGE("w25qxx: set type failed.");
        return 1;
    }
    /* set chip interface */
    res = w25qxx_set_interface(&ex_flash_handle, W25QXX_INTERFACE_QSPI);
    if (res) {
        RTE_LOGE("w25qxx: set interface failed.");
        return 1;
    }
    /* set dual quad spi */
    res = w25qxx_set_dual_quad_spi(&ex_flash_handle, W25QXX_BOOL_FALSE);
    if (res) {
        RTE_LOGE("w25qxx: set dual quad spi failed.");
        w25qxx_deinit(&ex_flash_handle);
        return 1;
    }
    /* chip init */
    res = w25qxx_init(&ex_flash_handle);
    if (res) {
        RTE_LOGE("w25qxx: init failed.");
        return 1;
    }
    /* Init mutex */
    osMutexAttr_t flash_mutex_attr = {
        LOG_STR(EXE_FLASH),
        osMutexPrioInherit | osMutexRecursive,
        NULL,
        0U
    };
    ex_flash_mutex_instance.mutex = (void *)osMutexNew(&flash_mutex_attr);
    ex_flash_mutex_instance.lock = rte_mutex_lock;
    ex_flash_mutex_instance.unlock = rte_mutex_unlock;
    ex_flash_mutex_instance.trylock = NULL;
    return 0;
}

static int db_fal_init(void)
{
    w25qxx_info_t info;
    uint8_t res = 0;
    /* get information */
    res = w25qxx_info(&info);
    if (res) {
        RTE_LOGE("w25qxx: get info failed.");
        return 1;
    } else {
        /* print chip information */
        RTE_LOGI("w25qxx: chip is %s.", info.chip_name);
        RTE_LOGI("w25qxx: manufacturer is %s.", info.manufacturer_name);
        RTE_LOGI("w25qxx: interface is %s.", info.interface);
        RTE_LOGI("w25qxx: driver version is %d.%d.", info.driver_version / 1000, (info.driver_version % 1000) / 100);
    }

    /* set chip type */
    res = w25qxx_set_type(&db_flash_handle, W25Q64);
    if (res) {
        RTE_LOGE("w25qxx: set type failed.");
        return 1;
    }
    /* set chip interface */
    res = w25qxx_set_interface(&db_flash_handle, W25QXX_INTERFACE_SPI);
    if (res) {
        RTE_LOGE("w25qxx: set interface failed.");
        return 1;
    }
    /* set dual quad spi */
    res = w25qxx_set_dual_quad_spi(&db_flash_handle, W25QXX_BOOL_FALSE);
    if (res) {
        RTE_LOGE("w25qxx: set dual quad spi failed.");
        w25qxx_deinit(&db_flash_handle);
        return 1;
    }
    /* chip init */
    res = w25qxx_init(&db_flash_handle);
    if (res) {
        RTE_LOGE("w25qxx: init failed %d.", res);
        return 1;
    }
    /* Init mutex */
    osMutexAttr_t flash_mutex_attr = {
        LOG_STR(DB_FLASH),
        osMutexPrioInherit | osMutexRecursive,
        NULL,
        0U
    };
    db_flash_mutex_instance.mutex = (void *)osMutexNew(&flash_mutex_attr);
    db_flash_mutex_instance.lock = rte_mutex_lock;
    db_flash_mutex_instance.unlock = rte_mutex_unlock;
    db_flash_mutex_instance.trylock = NULL;
    return 0;
}

static int db_fal_read(long offset, uint8_t *buf, size_t size)
{
    /* You can add your code under here. */
    int32_t ret;
    uint32_t addr = nor_flash1.addr + offset;


    rte_lock(&db_flash_mutex_instance);
    ret = w25qxx_fast_read(&db_flash_handle, addr, buf, size);
    rte_unlock(&db_flash_mutex_instance);

    return (ret == 0) ? size : ret;
}

static int db_fal_write(long offset, const uint8_t *buf, size_t size)
{
    int32_t ret;
    uint32_t addr = nor_flash1.addr + offset;

    rte_lock(&db_flash_mutex_instance);
    ret = w25qxx_write(&db_flash_handle, addr, (uint8_t *)buf, size);
    rte_unlock(&db_flash_mutex_instance);

    return (ret == 0) ? size : ret;
}

static int db_fal_erase(long offset, size_t size)
{
    int32_t ret;
    uint32_t addr = nor_flash1.addr + offset;

    int32_t erase_size = ((size - 1) / FLASH_ERASE_MIN_SIZE) + 1;

    rte_lock(&db_flash_mutex_instance);
    for (int32_t i = 0; i < erase_size; i++)
        ret = w25qxx_sector_erase_4k(&db_flash_handle, addr);
    rte_unlock(&db_flash_mutex_instance);

    return (ret == 0) ? erase_size : ret;
}

static int ex_fal_read(long offset, uint8_t *buf, size_t size)
{
    /* You can add your code under here. */
    int32_t ret;
    uint32_t addr = nor_flash0.addr + offset;


    rte_lock(&ex_flash_mutex_instance);
    ret = w25qxx_fast_read(&ex_flash_handle, addr, buf, size);
    rte_unlock(&ex_flash_mutex_instance);

    return (ret == 0) ? size : ret;
}

static int ex_fal_write(long offset, const uint8_t *buf, size_t size)
{
    int32_t ret;
    uint32_t addr = nor_flash0.addr + offset;

    rte_lock(&ex_flash_mutex_instance);
    ret = w25qxx_write(&ex_flash_handle, addr, (uint8_t *)buf, size);
    rte_unlock(&ex_flash_mutex_instance);

    return (ret == 0) ? size : ret;
}

static int ex_fal_erase(long offset, size_t size)
{
    int32_t ret;
    uint32_t addr = nor_flash0.addr + offset;

    int32_t erase_size = ((size - 1) / FLASH_ERASE_MIN_SIZE) + 1;

    rte_lock(&ex_flash_mutex_instance);
    for (int32_t i = 0; i < erase_size; i++)
        ret = w25qxx_sector_erase_4k(&ex_flash_handle, addr);
    rte_unlock(&ex_flash_mutex_instance);

    return (ret == 0) ? erase_size : ret;
}

/*
    "norflash0" : Flash 设备的名字。
    0x08000000: 对 Flash 操作的起始地址。
    1024*1024：可操作 Flash 的总大小（1MB）。
    128*1024：Flash 块/扇区大小（因为 STM32F2 各块大小不均匀，所以擦除粒度为最大块的大小：128K）。
    {init, read, write, erase} ：Flash 的操作函数。 如果没有 init 初始化过程，第一个操作函数位置可以置空。
    8 : 设置写粒度，单位 bit， 0 表示未生效（默认值为 0 ），该成员是 fal 版本大于 0.4.0 的新增成员。各个 flash 写入粒度不尽相同，可通过该成员进行设置，以下列举几种常见 Flash 写粒度：
    nor flash:  1 bit
    stm32f2/f4: 8 bit
    stm32f1:    32 bit
    stm32l4:    64 bit
 */

//1.定义 flash 设备

struct fal_flash_dev nor_flash0 =
{
    .name       = "norflash0",
    .addr       = 0x0,
    .len        = 8 * 1024 * 1024,
    .blk_size   = FLASH_ERASE_MIN_SIZE,
    .ops        = {ex_fal_init, ex_fal_read, ex_fal_write, ex_fal_erase},
    .write_gran = 1
};


struct fal_flash_dev nor_flash1 =
{
    .name       = "norflash1",
    .addr       = 0x0,
    .len        = 8 * 1024 * 1024,
    .blk_size   = FLASH_ERASE_MIN_SIZE,
    .ops        = {db_fal_init, db_fal_read, db_fal_write, db_fal_erase},
    .write_gran = 1
};
