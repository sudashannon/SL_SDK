#include "RTE_KVDB.h"
/*****************************************************************************
*** Author: Shannon
*** Version: 1.1 2018.10.06
*** History: 1.0 创建，修改自easy_flash
*** History: 1.1 修复一个小warning
*****************************************************************************/
#if RTE_USE_KVDB == 1
#define KVDB_STR "[KVDB]"
#if RTE_USE_OS == 1
	osMutexId_t MutexIDKVDB;
#endif
/* flash ENV parameters index and size in system section */
enum {
    /* data section ENV end address index in system section */
    ENV_PARAM_INDEX_END_ADDR = 0,
#if KVDB_USE_PFS== 1
    /* saved count for ENV area */
    ENV_PARAM_INDEX_SAVED_COUNT,
#endif

#if KVDB_USE_AUTO_UPDATE== 1
    /* current version number for ENV */
    ENV_PARAM_INDEX_VER_NUM,
#endif
    /* data section CRC32 code index in system section */
    ENV_PARAM_INDEX_DATA_CRC,
    /* flash ENV parameters word size */
    ENV_PARAM_WORD_SIZE,
    /* flash ENV parameters byte size */
    ENV_PARAM_BYTE_SIZE = ENV_PARAM_WORD_SIZE * 4,
};
/* default ENV set, must be initialized by user */
static ef_env const *default_env_set;
/* default ENV set size, must be initialized by user */
static size_t default_env_set_size = 0;
/* ENV ram cache */
static uint32_t env_cache[KVDB_USER_SETTING_SIZE / 4] = { 0 };
/* ENV start address in flash */
static uint32_t env_start_addr = 0;
/* ENV ram cache has changed when ENV created, deleted and changed value. */
static bool env_cache_changed = false;
/* initialize OK flag */
static bool init_ok = false;

#if KVDB_USE_PFS == 1
/* current load ENV area address */
static uint32_t cur_load_area_addr = 0;
/* next save ENV area address */
static uint32_t next_save_area_addr = 0;
#endif

static uint32_t get_env_system_addr(void);
static uint32_t get_env_data_addr(void);
static uint32_t get_env_end_addr(void);
static void set_env_end_addr(uint32_t end_addr);
static EfErrCode write_env(const char *key, const char *value);
static char *find_env(const char *key);
static EfErrCode del_env(const char *key);
static size_t get_env_data_size(void);
static size_t get_env_user_used_size(void);
static EfErrCode create_env(const char *key, const char *value);
static uint32_t calc_env_crc(void);
static bool env_crc_is_ok(void);
#if KVDB_USE_AUTO_UPDATE== 1
static EfErrCode env_auto_update(void);
#endif
static const uint32_t crc32_table[] =
{
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
    0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
    0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
    0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
    0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
    0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
    0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
    0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
    0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
    0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
    0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
    0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
    0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
    0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
    0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
    0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
    0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
    0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
    0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
    0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
    0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
    0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
    0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
    0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
    0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
    0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
    0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
    0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
    0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
    0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
    0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
    0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

/**
 * Calculate the CRC32 value of a memory buffer.
 *
 * @param crc accumulated CRC32 value, must be 0 on first call
 * @param buf buffer to calculate CRC32 value for
 * @param size bytes in buffer
 *
 * @return calculated CRC32 value
 */
static uint32_t ef_calc_crc32(uint32_t crc, const void *buf, size_t size)
{
    const uint8_t *p;

    p = (const uint8_t *)buf;
    crc = crc ^ ~0U;

    while (size--) {
        crc = crc32_table[(crc ^ *p++) & 0xFF] ^ (crc >> 8);
    }

    return crc ^ ~0U;
}

/**
 * Flash ENV initialize.
 *
 * @param default_env default ENV set for user
 * @param default_env_size default ENV set size
 *
 * @note user_size must equal with total_size in normal mode
 *
 * @return result
 */
EfErrCode ef_env_init(ef_env const *default_env, size_t default_env_size) {
#if RTE_USE_OS == 1
	MutexIDKVDB = osMutexNew(NULL);
#endif
	EfErrCode result = EF_NO_ERR;
	RTE_AssertParam(KVDB_ERASE_MIN_SIZE);
	RTE_AssertParam(KVDB_SIZE);
	RTE_AssertParam(KVDB_USER_SETTING_SIZE);
	/* must be word alignment for ENV */
	RTE_AssertParam(KVDB_SIZE % 4 == 0);
	RTE_AssertParam(KVDB_SIZE % 4 == 0);
	RTE_AssertParam(default_env);
	RTE_AssertParam(default_env_size < KVDB_USER_SETTING_SIZE);
#if KVDB_USE_PFS == 0
	/* total_size must be aligned with erase_min_size */
	if (KVDB_USER_SETTING_SIZE % KVDB_ERASE_MIN_SIZE == 0) 
	{
		RTE_AssertParam(KVDB_USER_SETTING_SIZE == KVDB_SIZE);
	} 
	else 
	{
		RTE_AssertParam((KVDB_USER_SETTING_SIZE / KVDB_ERASE_MIN_SIZE + 1)*KVDB_ERASE_MIN_SIZE == KVDB_SIZE);
	}
#else
	/* total_size must be aligned with erase_min_size */
	if (KVDB_USER_SETTING_SIZE % KVDB_ERASE_MIN_SIZE == 0) 
	{
		/* it has double area when used power fail safeguard mode */
		RTE_AssertParam(2 * KVDB_USER_SETTING_SIZE == KVDB_SIZE);
	} 
	else 
	{
		/* it has double area when used power fail safeguard mode */
		RTE_AssertParam(2 * (KVDB_USER_SETTING_SIZE / KVDB_ERASE_MIN_SIZE + 1)*KVDB_ERASE_MIN_SIZE == KVDB_SIZE);
	}
#endif
	env_start_addr = KVDB_FLASH_BASE + KVDB_ADDR_OFFSET;
	default_env_set = default_env;
	default_env_set_size = default_env_size;

	RTE_Printf("%10s    KVDB start address is 0x%08X, size is %d bytes.\r\n",KVDB_STR, env_start_addr, KVDB_SIZE);

	result = ef_load_env();

#if KVDB_USE_AUTO_UPDATE== 1
	if (result == EF_NO_ERR) {
			env_auto_update();
	}
#endif

	if (result == EF_NO_ERR) {
			init_ok = true;
	}
	return result;
	
}
/**
 * ENV set default.
 *
 * @return result
 */
EfErrCode ef_env_set_default(void) {
    extern EfErrCode KVDB_FM_VER_NUM_set_default(void);

    EfErrCode result = EF_NO_ERR;
    size_t i;

    RTE_AssertParam(default_env_set);
    RTE_AssertParam(default_env_set_size);

    /* lock the ENV cache */
    ef_port_env_lock();

    /* set environment end address is at data section start address */
    set_env_end_addr(get_env_data_addr());

#if KVDB_USE_PFS== 1
    /* set saved count to default 0 */
    env_cache[ENV_PARAM_INDEX_SAVED_COUNT] = 0;
#endif

#if KVDB_USE_AUTO_UPDATE== 1
    /* initialize version number */
    env_cache[ENV_PARAM_INDEX_VER_NUM] = KVDB_FM_VER_NUM;
#endif

    /* create default ENV */
    for (i = 0; i < default_env_set_size; i++) {
        create_env(default_env_set[i].key, default_env_set[i].value);
    }

    /* unlock the ENV cache */
    ef_port_env_unlock();

    result = ef_save_env();

#if KVDB_USE_PFS== 1
    /* reset other PFS area's data */
    if (result == EF_NO_ERR) {
        env_cache_changed = true;
        result = ef_save_env();
    }
#endif
    return result;
}
/**
 * Get ENV system section start address.
 *
 * @return system section start address
 */
static uint32_t get_env_system_addr(void) {
#if KVDB_USE_PFS == 0
    return env_start_addr;
#else
    return cur_load_area_addr;
#endif
}

/**
 * Get ENV data section start address.
 *
 * @return data section start address
 */
static uint32_t get_env_data_addr(void) {
    return get_env_system_addr() + ENV_PARAM_BYTE_SIZE;
}

/**
 * Get ENV end address.
 * It's the first word in ENV.
 *
 * @return ENV end address
 */
static uint32_t get_env_end_addr(void) {
    /* it is the first word */
    return env_cache[ENV_PARAM_INDEX_END_ADDR];
}

/**
 * Set ENV end address.
 * It's the first word in ENV.
 *
 * @param end_addr ENV end address
 */
static void set_env_end_addr(uint32_t end_addr) {
    env_cache[ENV_PARAM_INDEX_END_ADDR] = end_addr;
}

/**
 * Get current ENV data section size.
 *
 * @return size
 */
static size_t get_env_data_size(void) {
    if (get_env_end_addr() > get_env_data_addr()) {
        return get_env_end_addr() - get_env_data_addr();
    } else {
        return 0;
    }
}

/**
 * Get current user used ENV size.
 *
 * @return bytes
 */
static size_t get_env_user_used_size(void) {
    if (get_env_end_addr() > get_env_system_addr()) {
        return get_env_end_addr() - get_env_system_addr();
    } else {
        return 0;
    }
}

/**
 * Get current ENV already write bytes.
 *
 * @return write bytes
 */
size_t ef_get_env_write_bytes(void) {
#if KVDB_USE_PFS == 0
    return get_env_user_used_size();
#else
    return get_env_user_used_size() * 2;
#endif
}
/**
 * Write an ENV at the end of cache.
 *
 * @param key ENV name
 * @param value ENV value
 *
 * @return result
 */
static EfErrCode write_env(const char *key, const char *value) {
    EfErrCode result = EF_NO_ERR;
    size_t key_len = strlen(key), value_len = strlen(value), env_str_len;
    char *env_cache_bak = (char *)env_cache;

    /* calculate ENV storage length, contain '=' and '\0'. */
    env_str_len = key_len + value_len + 2;
    if (env_str_len % 4 != 0) {
        env_str_len = (env_str_len / 4 + 1) * 4;
    }
    /* check capacity of ENV  */
    if (env_str_len + get_env_user_used_size() >= KVDB_USER_SETTING_SIZE) {
        return EF_ENV_FULL;
    }

    /* calculate current ENV ram cache end address */
    env_cache_bak += get_env_user_used_size();

    /* copy key name */
    memcpy(env_cache_bak, key, key_len);
    env_cache_bak += key_len;
    /* copy equal sign */
    *env_cache_bak = '=';
    env_cache_bak++;
    /* copy value */
    memcpy(env_cache_bak, value, value_len);
    env_cache_bak += value_len;
    /* fill '\0' for string end sign */
    *env_cache_bak = '\0';
    env_cache_bak ++;
    /* fill '\0' for word alignment */
    memset(env_cache_bak, 0, env_str_len - (key_len + value_len + 2));
    set_env_end_addr(get_env_end_addr() + env_str_len);
    /* ENV ram cache has changed */
    env_cache_changed = true;

    return result;
}

/**
 * Find ENV.
 *
 * @param key ENV name
 *
 * @return found ENV in ram cache
 */
static char *find_env(const char *key) {
    char *env_start, *env_end, *env, *found_env = NULL;
    size_t key_len = strlen(key), env_len;

    if ((key == NULL) || *key == '\0') {
        RTE_Printf("%10s    Flash ENV name must be not empty!\r\n",KVDB_STR);
        return NULL;
    }

    /* from data section start to data section end */
    env_start = (char *) ((char *) env_cache + ENV_PARAM_BYTE_SIZE);
    env_end = (char *) ((char *) env_cache + get_env_user_used_size());

    /* ENV is null */
    if (env_start == env_end) {
        return NULL;
    }

    env = env_start;
    while (env < env_end) {
        /* the key length must be equal */
        if (!strncmp(env, key, key_len) && (env[key_len] == '=')) {
            found_env = env;
            break;
        } else {
            /* calculate ENV length, contain '\0'. */
            env_len = strlen(env) + 1;
            /* next ENV and word alignment */
            if (env_len % 4 == 0) {
                env += env_len;
            } else {
                env += (env_len / 4 + 1) * 4;
            }
        }
    }
    return found_env;
}

/**
 * If the ENV is not exist, create it.
 * @see flash_write_env
 *
 * @param key ENV name
 * @param value ENV value
 *
 * @return result
 */
static EfErrCode create_env(const char *key, const char *value) {
    EfErrCode result = EF_NO_ERR;

    RTE_AssertParam(key);
    RTE_AssertParam(value);

    if ((key == NULL) || *key == '\0') {
        RTE_Printf("%10s    Flash ENV name must be not empty!\r\n",KVDB_STR);
        return EF_ENV_NAME_ERR;
    }

    if (strchr(key, '=')) {
        RTE_Printf("%10s    Flash ENV name can't contain '='.\r\n",KVDB_STR);
        return EF_ENV_NAME_ERR;
    }

    /* find ENV */
    if (find_env(key)) {
        RTE_Printf("%10s    The name of \"%s\" is already exist.\r\n",KVDB_STR, key);
        return EF_ENV_NAME_EXIST;
    }
    /* write ENV at the end of cache */
    result = write_env(key, value);

    return result;
}

/**
 * Delete an ENV in cache.
 *
 * @param key ENV name
 *
 * @return result
 */
static EfErrCode del_env(const char *key) {
    EfErrCode result = EF_NO_ERR;
    char *del_env = NULL;
    size_t del_env_length, remain_env_length;

    RTE_AssertParam(key);

    if ((key == NULL) || *key == '\0') {
        RTE_Printf("%10s    Flash ENV name must be not NULL!\r\n",KVDB_STR);
        return EF_ENV_NAME_ERR;
    }

    if (strchr(key, '=')) {
        RTE_Printf("%10s    Flash ENV name or value can't contain '='.\r\n",KVDB_STR);
        return EF_ENV_NAME_ERR;
    }

    /* find ENV */
    del_env = find_env(key);

    if (!del_env) {
        RTE_Printf("%10s    Not find \"%s\" in ENV.\r\n",KVDB_STR, key);
        return EF_ENV_NAME_ERR;
    }
    del_env_length = strlen(del_env);
    /* '\0' also must be as ENV length */
    del_env_length ++;
    /* the address must multiple of 4 */
    if (del_env_length % 4 != 0) {
        del_env_length = (del_env_length / 4 + 1) * 4;
    }
    /* calculate remain ENV length */
    remain_env_length = get_env_data_size()
                        - (((uint32_t) del_env + del_env_length) - ((uint32_t) env_cache + ENV_PARAM_BYTE_SIZE));
    /* remain ENV move forward */
    memcpy(del_env, del_env + del_env_length, remain_env_length);
    /* reset ENV end address */
    set_env_end_addr(get_env_end_addr() - del_env_length);
    /* ENV ram cache has changed */
    env_cache_changed = true;

    return result;
}

/**
 * Set an ENV.If it value is NULL, delete it.
 * If not find it in ENV table, then create it.
 *
 * @param key ENV name
 * @param value ENV value
 *
 * @return result
 */
EfErrCode ef_set_env(const char *key, const char *value) {
    EfErrCode result = EF_NO_ERR;
    char *old_env, *old_value;

    if (!init_ok) {
        RTE_Printf("%10s    ENV isn't initialize OK.\r\n",KVDB_STR);
        return EF_ENV_INIT_FAILED;
    }

    /* lock the ENV cache */
    ef_port_env_lock();

    /* if ENV value is NULL, delete it */
    if (value == NULL) {
        result = del_env(key);
    } else {
        old_env = find_env(key);
        /* If find this ENV, then compare the new value and old value. */
        if (old_env) {
            /* find the old value address */
            old_env = strchr(old_env, '=');
            old_value = old_env + 1;
            /* If it is changed then delete it and recreate it  */
            if (strcmp(old_value, value)) {
                result = del_env(key);
                if (result == EF_NO_ERR) {
                    result = create_env(key, value);
                }
            }
        } else {
            result = create_env(key, value);
        }
    }

    /* unlock the ENV cache */
    ef_port_env_unlock();

    return result;
}

/**
 * Del an ENV.
 *
 * @param key ENV name
 *
 * @return result
 */
EfErrCode ef_del_env(const char *key) {
    EfErrCode result = EF_NO_ERR;

    if (!init_ok) {
        RTE_Printf("%10s    ENV isn't initialize OK.\r\n",KVDB_STR);
        return EF_ENV_INIT_FAILED;
    }

    /* lock the ENV cache */
    ef_port_env_lock();

    result = del_env(key);

    /* unlock the ENV cache */
    ef_port_env_unlock();

    return result;
}

/**
 * Get an ENV value by key name.
 *
 * @param key ENV name
 *
 * @return value
 */
char *ef_get_env(const char *key) {
    char *env = NULL, *value = NULL;

    if (!init_ok) {
        RTE_Printf("%10s    ENV isn't initialize OK.\r\n",KVDB_STR);
        return NULL;
    }

    /* find ENV */
    env = find_env(key);

    if (env == NULL) {
        return NULL;
    }
    /* get value address */
    value = strchr(env, '=');
    if (value != NULL) {
        /* the equal sign next character is value */
        value++;
    }
    return value;
}
/**
 * Print ENV.
 */
void ef_print_env(void) {
    uint32_t *env_cache_data_addr = env_cache + ENV_PARAM_WORD_SIZE,
              *env_cache_end_addr =
                  (uint32_t *) (env_cache + ENV_PARAM_WORD_SIZE + get_env_data_size() / 4);
    uint8_t j;
    char c;

    if (!init_ok) {
        RTE_Printf("%10s    ENV isn't initialize OK.\r\n",KVDB_STR);
        return;
    }

    for (; env_cache_data_addr < env_cache_end_addr; env_cache_data_addr += 1) {
				RTE_Printf("%10s    ",KVDB_STR);
        for (j = 0; j < 4; j++) {
            c = (*env_cache_data_addr) >> (8 * j);
            RTE_Printf(" %c", c);
            if (c == '\0') {
                RTE_Printf("\r\n");
                break;
            }
        }
    }

#if KVDB_USE_PFS == 0
    RTE_Printf("%10s    \nmode: normal\r\n",KVDB_STR);
    RTE_Printf("%10s    size: %ld/%ld bytes.\r\n",KVDB_STR, get_env_user_used_size(), KVDB_USER_SETTING_SIZE);
#else
    RTE_Printf("%10s    \nmode: power fail safeguard\r\n",KVDB_STR);
    RTE_Printf("%10s    size: %ld/%ld bytes, write bytes %ld/%ld.\r\n",KVDB_STR, get_env_user_used_size(),
             KVDB_USER_SETTING_SIZE, ef_get_env_write_bytes(), KVDB_SIZE);
    RTE_Printf("%10s    saved count: %ld\r\n",KVDB_STR, env_cache[ENV_PARAM_INDEX_SAVED_COUNT]);
#endif

#if KVDB_USE_AUTO_UPDATE== 1
    RTE_Printf("%10s    ver num: %d\r\n", env_cache[ENV_PARAM_INDEX_VER_NUM]);
#endif
}
/**
 * Load flash ENV to ram.
 *
 * @return result
 */
#if KVDB_USE_PFS == 0
EfErrCode ef_load_env(void) {
    EfErrCode result = EF_NO_ERR;
    uint32_t *env_cache_bak, env_end_addr;

    /* read ENV end address from flash */
    ef_port_read(get_env_system_addr() + ENV_PARAM_INDEX_END_ADDR * 4, &env_end_addr, 4);
    /* if ENV is not initialize or flash has dirty data, set default for it */
    if ((env_end_addr == 0xFFFFFFFF) || (env_end_addr < env_start_addr)
            || (env_end_addr > env_start_addr + KVDB_USER_SETTING_SIZE)) {
        result = ef_env_set_default();
    } else {
        /* set ENV end address */
        set_env_end_addr(env_end_addr);

        env_cache_bak = env_cache + ENV_PARAM_WORD_SIZE;
        /* read all ENV from flash */
        ef_port_read(get_env_data_addr(), env_cache_bak, get_env_data_size());
        /* read ENV CRC code from flash */
        ef_port_read(get_env_system_addr() + ENV_PARAM_INDEX_DATA_CRC * 4,
                     &env_cache[ENV_PARAM_INDEX_DATA_CRC] , 4);
        /* if ENV CRC32 check is fault, set default for it */
        if (!env_crc_is_ok()) {
            RTE_Printf("%10s    Warning: ENV CRC check failed. Set it to default.\r\n",KVDB_STR);
            result = ef_env_set_default();
        }
    }
    return result;
}
#else
EfErrCode ef_load_env(void) {
    EfErrCode result = EF_NO_ERR;
    uint32_t area0_start_address = env_start_addr, area1_start_address = env_start_addr
                                   + KVDB_SIZE / 2;
    uint32_t area0_end_addr, area1_end_addr, area0_crc, area1_crc, area0_saved_count, area1_saved_count;
    bool area0_is_valid = true, area1_is_valid = true;
    /* read ENV area end address from flash */
    ef_port_read(area0_start_address + ENV_PARAM_INDEX_END_ADDR * 4, &area0_end_addr, 4);
    ef_port_read(area1_start_address + ENV_PARAM_INDEX_END_ADDR * 4, &area1_end_addr, 4);
    if ((area0_end_addr == 0xFFFFFFFF) || (area0_end_addr < area0_start_address)
            || (area0_end_addr > area0_start_address + KVDB_USER_SETTING_SIZE)) {
        area0_is_valid = false;
    }
    if ((area1_end_addr == 0xFFFFFFFF) || (area1_end_addr < area1_start_address)
            || (area1_end_addr > area1_start_address + KVDB_USER_SETTING_SIZE)) {
        area1_is_valid = false;
    }
    /* check area0 CRC when it is valid */
    if (area0_is_valid) {
        /* read ENV area0 crc32 code from flash */
        ef_port_read(area0_start_address + ENV_PARAM_INDEX_DATA_CRC * 4, &area0_crc, 4);
        /* read ENV from ENV area0 */
        ef_port_read(area0_start_address, env_cache, area0_end_addr - area0_start_address);
        /* current load ENV area address is area0 start address */
        cur_load_area_addr = area0_start_address;
        if (!env_crc_is_ok()) {
            area0_is_valid = false;
        }
    }
    /* check area1 CRC when it is valid */
    if (area1_is_valid) {
        /* read ENV area1 crc32 code from flash */
        ef_port_read(area1_start_address + ENV_PARAM_INDEX_DATA_CRC * 4, &area1_crc, 4);
        /* read ENV from ENV area1 */
        ef_port_read(area1_start_address, env_cache, area1_end_addr - area1_start_address);
        /* current load ENV area address is area1 start address */
        cur_load_area_addr = area1_start_address;
        if (!env_crc_is_ok()) {
            area1_is_valid = false;
        }
    }
    /* all ENV area CRC is OK then compare saved count */
    if (area0_is_valid && area1_is_valid) {
        /* read ENV area saved count from flash */
        ef_port_read(area0_start_address + ENV_PARAM_INDEX_SAVED_COUNT * 4,
                     &area0_saved_count, 4);
        ef_port_read(area1_start_address + ENV_PARAM_INDEX_SAVED_COUNT * 4,
                     &area1_saved_count, 4);
        /* the bigger saved count area is valid */
        if ((area0_saved_count > area1_saved_count) || ((area0_saved_count == 0) && (area1_saved_count == 0xFFFFFFFF))) {
            area1_is_valid = false;
        } else {
            area0_is_valid = false;
        }
    }
    if (area0_is_valid) {
        /* current load ENV area address is area0 start address */
        cur_load_area_addr = area0_start_address;
        /* next save ENV area address is area1 start address */
        next_save_area_addr = area1_start_address;
        /* read all ENV from area0 */
        ef_port_read(area0_start_address, env_cache, area0_end_addr - area0_start_address);
    } else if (area1_is_valid) {
        /* next save ENV area address is area0 start address */
        next_save_area_addr = area0_start_address;
    } else {
        /* current load ENV area address is area1 start address */
        cur_load_area_addr = area1_start_address;
        /* next save ENV area address is area0 start address */
        next_save_area_addr = area0_start_address;
        /* set the ENV to default */
        result = ef_env_set_default();
    }
    return result;
}
#endif

/**
 * Save ENV to flash.
 */
EfErrCode ef_save_env(void) {
    EfErrCode result = EF_NO_ERR;
    uint32_t write_addr, write_size;

    /* ENV ram cache has not changed don't need to save */
    if (!env_cache_changed) {
        return result;
    }

#if KVDB_USE_PFS == 0
    write_addr = get_env_system_addr();
    write_size = get_env_user_used_size();
    /* calculate and cache CRC32 code */
    env_cache[ENV_PARAM_INDEX_DATA_CRC] = calc_env_crc();
#else
    write_addr = next_save_area_addr;
    write_size = get_env_user_used_size();
    /* replace next_save_area_addr with cur_load_area_addr */
    next_save_area_addr = cur_load_area_addr;
    cur_load_area_addr = write_addr;
    /* change the ENV end address to next save area address */
    set_env_end_addr(write_addr + write_size);
    /* ENV area saved count +1 */
    env_cache[ENV_PARAM_INDEX_SAVED_COUNT]++;
    /* calculate and cache CRC32 code */
    env_cache[ENV_PARAM_INDEX_DATA_CRC] = calc_env_crc();
#endif

    /* erase ENV */
    result = ef_port_erase(write_addr, write_size);
    switch (result) {
    case EF_NO_ERR: {
        RTE_Printf("%10s    Erased ENV OK.\r\n",KVDB_STR);
        break;
    }
    case EF_ERASE_ERR: {
        RTE_Printf("%10s    Error: Erased ENV fault! Start address is 0x%08X, size is %ld.\r\n",KVDB_STR, write_addr, write_size);
        /* will return when erase fault */
        return result;
		default:
			break;
    }
    }

    /* write ENV to flash */
    result = ef_port_write(write_addr, env_cache, write_size);
    switch (result) {
    case EF_NO_ERR: {
        RTE_Printf("%10s    Saved ENV OK.\r\n",KVDB_STR);
        break;
    }
    case EF_WRITE_ERR: {
        RTE_Printf("%10s    Error: Saved ENV fault! Start address is 0x%08X, size is %ld.\r\n",KVDB_STR, write_addr, write_size);
        break;
		default:
			break;
    }
    }

    env_cache_changed = false;

    return result;
}

/**
 * Calculate the cached ENV CRC32 value.
 *
 * @return CRC32 value
 */
static uint32_t calc_env_crc(void) {
    uint32_t crc32 = 0;

    /* Calculate the ENV end address CRC32. The 4 is ENV end address bytes size. */
    crc32 = ef_calc_crc32(crc32, &env_cache[ENV_PARAM_INDEX_END_ADDR], 4);

#if KVDB_USE_PFS== 1
    /* Calculate the ENV area saved count CRC32. */
    crc32 = ef_calc_crc32(crc32, &env_cache[ENV_PARAM_INDEX_SAVED_COUNT], 4);
#endif

    /* Calculate the all ENV data CRC32. */
    crc32 = ef_calc_crc32(crc32, &env_cache[ENV_PARAM_WORD_SIZE], get_env_data_size());

    RTE_Printf("%10s    Calculate ENV CRC32 number is 0x%08X.\r\n",KVDB_STR, crc32);

    return crc32;
}

/**
 * Check the ENV CRC32
 *
 * @return true is ok
 */
static bool env_crc_is_ok(void) {
    if (calc_env_crc() == env_cache[ENV_PARAM_INDEX_DATA_CRC]) {
        RTE_Printf("%10s    Verify ENV CRC32 result is OK.\r\n",KVDB_STR);
        return true;
    } else {
        return false;
    }
}

/**
 * Set and save an ENV. If set ENV is success then will save it.
 *
 * @param key ENV name
 * @param value ENV value
 *
 * @return result
 */
EfErrCode ef_set_and_save_env(const char *key, const char *value) {
    EfErrCode result = EF_NO_ERR;

    result = ef_set_env(key, value);

    if (result == EF_NO_ERR) {
        result = ef_save_env();
    }

    return result;
}

/**
 * Del and save an ENV. If del ENV is success then will save it.
 *
 * @param key ENV name
 *
 * @return result
 */
EfErrCode ef_del_and_save_env(const char *key) {
    EfErrCode result = EF_NO_ERR;

    result = ef_del_env(key);

    if (result == EF_NO_ERR) {
        result = ef_save_env();
    }

    return result;
}

#if KVDB_USE_AUTO_UPDATE== 1
/**
 * Auto update ENV to latest default when current EF_ENV_VER is changed.
 *
 * @return result
 */
static EfErrCode env_auto_update(void)
{
    size_t i;

    /* lock the ENV cache */
    ef_port_env_lock();

    /* read ENV version number from flash*/
    ef_port_read(get_env_system_addr() + ENV_PARAM_INDEX_VER_NUM * 4,
                 &env_cache[ENV_PARAM_INDEX_VER_NUM] , 4);

    /* check version number */
    if (env_cache[ENV_PARAM_INDEX_VER_NUM] != KVDB_FM_VER_NUM) {
        env_cache_changed = true;
        /* update version number */
        env_cache[ENV_PARAM_INDEX_VER_NUM] = KVDB_FM_VER_NUM;
        /* add a new ENV when it's not found */
        for (i = 0; i < default_env_set_size; i++) {
            if (find_env(default_env_set[i].key) == NULL) {
                create_env(default_env_set[i].key, default_env_set[i].value);
            }
        }
    }

    /* unlock the ENV cache */
    ef_port_env_unlock();

    return ef_save_env();
}
#endif /* KVDB_USE_AUTO_UPDATE */

#endif
