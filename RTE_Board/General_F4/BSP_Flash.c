#include "BSP_Flash.h"
static uint32_t stm32_get_sector(uint32_t address);
static uint32_t stm32_get_sector_size(uint32_t sector);
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
    FLASH_Status flash_status;
    size_t erased_size = 0;
    uint32_t cur_erase_sector;

    /* make sure the start address is a multiple of EF_ERASE_MIN_SIZE */
    RTE_AssertParam(addr % KVDB_ERASE_MIN_SIZE == 0);

    /* start erase */
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR
                    | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
    /* it will stop when erased size is greater than setting size */
    while(erased_size < size) {
        cur_erase_sector = stm32_get_sector(addr + erased_size);
        flash_status = FLASH_EraseSector(cur_erase_sector, VoltageRange_3);
        if (flash_status != FLASH_COMPLETE) {
            result = EF_ERASE_ERR;
            break;
        }
        erased_size += stm32_get_sector_size(cur_erase_sector);
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
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR
                    | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
    for (i = 0; i < size; i += 4, buf++, addr += 4) {
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
    //rt_sem_take(&env_cache_lock, RT_WAITING_FOREVER);
}

/**
 * unlock the ENV ram cache
 */
void ef_port_env_unlock(void) {
    //rt_sem_release(&env_cache_lock);
}
/**
 * Get the sector of a given address
 *
 * @param address flash address
 *
 * @return The sector of a given address
 */
static uint32_t stm32_get_sector(uint32_t address) {
    uint32_t sector = 0;

    if ((address < ADDR_FLASH_SECTOR_1) && (address >= ADDR_FLASH_SECTOR_0)) {
        sector = FLASH_Sector_0;
    } else if ((address < ADDR_FLASH_SECTOR_2) && (address >= ADDR_FLASH_SECTOR_1)) {
        sector = FLASH_Sector_1;
    } else if ((address < ADDR_FLASH_SECTOR_3) && (address >= ADDR_FLASH_SECTOR_2)) {
        sector = FLASH_Sector_2;
    } else if ((address < ADDR_FLASH_SECTOR_4) && (address >= ADDR_FLASH_SECTOR_3)) {
        sector = FLASH_Sector_3;
    } else if ((address < ADDR_FLASH_SECTOR_5) && (address >= ADDR_FLASH_SECTOR_4)) {
        sector = FLASH_Sector_4;
    } else if ((address < ADDR_FLASH_SECTOR_6) && (address >= ADDR_FLASH_SECTOR_5)) {
        sector = FLASH_Sector_5;
    } else if ((address < ADDR_FLASH_SECTOR_7) && (address >= ADDR_FLASH_SECTOR_6)) {
        sector = FLASH_Sector_6;
    } else if ((address < ADDR_FLASH_SECTOR_8) && (address >= ADDR_FLASH_SECTOR_7)) {
        sector = FLASH_Sector_7;
    } else if ((address < ADDR_FLASH_SECTOR_9) && (address >= ADDR_FLASH_SECTOR_8)) {
        sector = FLASH_Sector_8;
    } else if ((address < ADDR_FLASH_SECTOR_10) && (address >= ADDR_FLASH_SECTOR_9)) {
        sector = FLASH_Sector_9;
    } else if ((address < ADDR_FLASH_SECTOR_11) && (address >= ADDR_FLASH_SECTOR_10)) {
        sector = FLASH_Sector_10;
    } else if ((address < ADDR_FLASH_SECTOR_12) && (address >= ADDR_FLASH_SECTOR_11)) {
        sector = FLASH_Sector_11;
    } else if ((address < ADDR_FLASH_SECTOR_13) && (address >= ADDR_FLASH_SECTOR_12)) {
        sector = FLASH_Sector_12;
    } else if ((address < ADDR_FLASH_SECTOR_14) && (address >= ADDR_FLASH_SECTOR_13)) {
        sector = FLASH_Sector_13;
    } else if ((address < ADDR_FLASH_SECTOR_15) && (address >= ADDR_FLASH_SECTOR_14)) {
        sector = FLASH_Sector_14;
    } else if ((address < ADDR_FLASH_SECTOR_16) && (address >= ADDR_FLASH_SECTOR_15)) {
        sector = FLASH_Sector_15;
    } else if ((address < ADDR_FLASH_SECTOR_17) && (address >= ADDR_FLASH_SECTOR_16)) {
        sector = FLASH_Sector_16;
    } else if ((address < ADDR_FLASH_SECTOR_18) && (address >= ADDR_FLASH_SECTOR_17)) {
        sector = FLASH_Sector_17;
    } else if ((address < ADDR_FLASH_SECTOR_19) && (address >= ADDR_FLASH_SECTOR_18)) {
        sector = FLASH_Sector_18;
    } else if ((address < ADDR_FLASH_SECTOR_20) && (address >= ADDR_FLASH_SECTOR_19)) {
        sector = FLASH_Sector_19;
    } else if ((address < ADDR_FLASH_SECTOR_21) && (address >= ADDR_FLASH_SECTOR_20)) {
        sector = FLASH_Sector_20;
    } else if ((address < ADDR_FLASH_SECTOR_22) && (address >= ADDR_FLASH_SECTOR_21)) {
        sector = FLASH_Sector_21;
    } else if ((address < ADDR_FLASH_SECTOR_23) && (address >= ADDR_FLASH_SECTOR_22)) {
        sector = FLASH_Sector_22;
    } else /*(address < FLASH_END_ADDR) && (address >= ADDR_FLASH_SECTOR_23))*/
    {
        sector = FLASH_Sector_23;
    }

    return sector;
}

/**
 * Get the sector size
 *
 * @param sector sector
 *
 * @return sector size
 */
static uint32_t stm32_get_sector_size(uint32_t sector) {
    RTE_AssertParam(IS_FLASH_SECTOR(sector));

    switch (sector) {
    case FLASH_Sector_0: return 16 * 1024;
    case FLASH_Sector_1: return 16 * 1024;
    case FLASH_Sector_2: return 16 * 1024;
    case FLASH_Sector_3: return 16 * 1024;
    case FLASH_Sector_4: return 64 * 1024;
    case FLASH_Sector_5: return 128 * 1024;
    case FLASH_Sector_6: return 128 * 1024;
    case FLASH_Sector_7: return 128 * 1024;
    case FLASH_Sector_8: return 128 * 1024;
    case FLASH_Sector_9: return 128 * 1024;
    case FLASH_Sector_10: return 128 * 1024;
    case FLASH_Sector_11: return 128 * 1024;
    case FLASH_Sector_12: return 16 * 1024;
    case FLASH_Sector_13: return 16 * 1024;
    case FLASH_Sector_14: return 16 * 1024;
    case FLASH_Sector_15: return 16 * 1024;
    case FLASH_Sector_16: return 64 * 1024;
    case FLASH_Sector_17: return 128 * 1024;
    case FLASH_Sector_18: return 128 * 1024;
    case FLASH_Sector_19: return 128 * 1024;
    case FLASH_Sector_20: return 128 * 1024;
    case FLASH_Sector_21: return 128 * 1024;
    case FLASH_Sector_22: return 128 * 1024;
    case FLASH_Sector_23: return 128 * 1024;
    default : return 128 * 1024;
    }
}
