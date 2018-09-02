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
    size_t erased_size = 0;
		uint32_t cur_erase_addr;
    /* make sure the start address is a multiple of EF_ERASE_MIN_SIZE */
    RTE_AssertParam(addr % KVDB_ERASE_MIN_SIZE == 0);
    /* it will stop when erased size is greater than setting size */
    while(erased_size < size) {
			cur_erase_addr = addr + erased_size;
			// Erase this block of the flash.
			if(FlashErase(cur_erase_addr)==0)
				erased_size += 16 * 1024;
			else
				return EF_ERASE_ERR;
    }

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
    RTE_AssertParam(size % 4 == 0);
		if(FlashProgram((uint32_t *)buf,addr,size)!=0)
			result = EF_WRITE_ERR;
    return result;
}
/**
 * lock the ENV ram cache
 */
void ef_port_env_lock(void) {
    //rt_sem_take(&env_cache_lock, RT_WAITING_FOREVER);
}

/**
 * unlock the ENV ram cache
 */
void ef_port_env_unlock(void) {
    //rt_sem_release(&env_cache_lock);
}


