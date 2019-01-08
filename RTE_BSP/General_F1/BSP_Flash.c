#include "BSP_Flash.h"
/**
 * Read data from flash.
 * @note This operation's units is word.
 *
 * @param addr flash address
 * @param buf buffer to store read data
 * @param size read bytes size
 *
 * @return result
 */
EfErrCode ef_port_read(uint32_t addr, uint32_t *buf, size_t size) {
    EfErrCode result = EF_NO_ERR;
    RTE_AssertParam(size % 4 == 0);
    /*copy from flash to ram */
    for (; size > 0; size -= 4, addr += 4, buf++) {
		while(FLASH_GetStatus()==FLASH_BUSY){};
        *buf = *(uint32_t *) addr;
    }
    return result;
}
/**
 * Erase data on flash.
 * @note This operation is irreversible.
 * @note This operation's units is different which on many chips.
 *
 * @param addr flash address
 * @param size erase bytes size
 *
 * @return result
 */
EfErrCode ef_port_erase(uint32_t addr, size_t size) {
    EfErrCode result = EF_NO_ERR;
    FLASH_Status flash_status;
    size_t erase_pages, i;

    /* make sure the start address is a multiple of EF_ERASE_MIN_SIZE */
    RTE_AssertParam(addr % KVDB_ERASE_MIN_SIZE == 0);
    /* calculate pages */
    erase_pages = size / PAGE_SIZE;
    if (size % PAGE_SIZE != 0) {
        erase_pages++;
    }
    /* start erase */
    FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
    /* it will stop when erased size is greater than setting size */
    for (i = 0; i < erase_pages; i++) {
		while(FLASH_GetStatus()==FLASH_BUSY){};
        flash_status = FLASH_ErasePage(addr + (PAGE_SIZE * i));
        if (flash_status != FLASH_COMPLETE) {
            result = EF_ERASE_ERR;
            break;
        }
    }
    FLASH_Lock();

    return result;
}
/**
 * Write data to flash.
 * @note This operation's units is word.
 * @note This operation must after erase. @see flash_erase.
 *
 * @param addr flash address
 * @param buf the write data buffer
 * @param size write bytes size
 *
 * @return result
 */
EfErrCode ef_port_write(uint32_t addr, const uint32_t *buf, size_t size) {
    EfErrCode result = EF_NO_ERR;
    size_t i;
    uint32_t read_data;

    RTE_AssertParam(size % 4 == 0);

    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
    for (i = 0; i < size; i += 4, buf++, addr += 4) {
		while(FLASH_GetStatus()==FLASH_BUSY){};
        /* write data */
        FLASH_ProgramWord(addr, *buf);
        read_data = *(uint32_t *)addr;
        /* check data */
        if (read_data != *buf) {
            result = EF_WRITE_ERR;
            break;
        }
    }
    FLASH_Lock();

    return result;
}
/**
 * lock the ENV ram cache
 */
void ef_port_env_lock(void) {
#if RTE_USE_OS == 1
	osMutexAcquire(MutexIDKVDB,osWaitForever);
#endif
}

/**
 * unlock the ENV ram cache
 */
void ef_port_env_unlock(void) {
#if RTE_USE_OS == 1
	osMutexRelease(MutexIDKVDB);
#endif
}
