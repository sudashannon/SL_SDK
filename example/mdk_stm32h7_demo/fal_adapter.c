
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
extern w25qxx_handle_t gs_handle;
static rte_mutex_t flash_mutex_instance = {NULL};

#define LOCK()                                  \
    do {                                        \
        RTE_LOCK(&flash_mutex_instance);        \
    } while(0)

#define UNLOCK()                                \
    do {                                        \
        RTE_UNLOCK(&flash_mutex_instance);      \
    } while(0)

static int init(void)
{
    extern void driver_w25qxx_init(void);
    driver_w25qxx_init();
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
    res = w25qxx_set_type(&gs_handle, W25Q64);
    if (res) {
        RTE_LOGE("w25qxx: set type failed.");
        return 1;
    }
    /* set chip interface */
    res = w25qxx_set_interface(&gs_handle, W25QXX_INTERFACE_QSPI);
    if (res) {
        RTE_LOGE("w25qxx: set interface failed.");
        return 1;
    }
    /* set dual quad spi */
    res = w25qxx_set_dual_quad_spi(&gs_handle, W25QXX_BOOL_FALSE);
    if (res) {
        RTE_LOGE("w25qxx: set dual quad spi failed.");
        w25qxx_deinit(&gs_handle);
        return 1;
    }
    /* chip init */
    res = w25qxx_init(&gs_handle);
    if (res) {
        RTE_LOGE("w25qxx: init failed.");
        return 1;
    }
    /* Init mutex */
    osMutexAttr_t flash_mutex_attr = {
        LOG_STR(FLASH),
        osMutexPrioInherit | osMutexRecursive,
        NULL,
        0U
    };
    flash_mutex_instance.mutex = (void *)osMutexNew(&flash_mutex_attr);
    flash_mutex_instance.lock = rte_mutex_lock;
    flash_mutex_instance.unlock = rte_mutex_unlock;
    flash_mutex_instance.trylock = NULL;
    return 0;
}

static int read(long offset, uint8_t *buf, size_t size)
{
    //Xassert(size % 4 == 0);

    /* You can add your code under here. */
    int32_t ret;
    uint32_t addr = nor_flash0.addr + offset;


    LOCK();
    ret = w25qxx_fast_read(&gs_handle, addr, buf, size);
    UNLOCK();

    return ret;
}

static int write(long offset, const uint8_t *buf, size_t size)
{
    int32_t ret;
    uint32_t addr = nor_flash0.addr + offset;

    LOCK();
    ret = w25qxx_write(&gs_handle, addr, (uint8_t *)buf, size);
    UNLOCK();

    return ret;
}

static int erase(long offset, size_t size)
{
    int32_t ret;
    uint32_t addr = nor_flash0.addr + offset;

    int32_t erase_size = ((size - 1) / FLASH_ERASE_MIN_SIZE) + 1;

    LOCK();
    for (int32_t i = 0; i < erase_size; i++)
        ret = w25qxx_sector_erase_4k(&gs_handle, addr);
    UNLOCK();

    return ret;
}

/*
  "esp8266_onchip" : Flash 设备的名字。
  0x08000000: 对 Flash 操作的起始地址。
  1024*1024：Flash 的总大小（1MB）。
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
    .addr       = 0x700000,
    .len        = 1024*1024,
    .blk_size   = FLASH_ERASE_MIN_SIZE,
    .ops        = {init, read, write, erase},
    .write_gran = 1
};