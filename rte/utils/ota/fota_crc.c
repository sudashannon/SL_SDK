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

static uint32_t crc_tab[256];

static uint32_t bitrev(uint32_t poly, uint32_t width)
{
	uint32_t i;
	uint32_t var = 0;

	for (i = 0; i < width; i++)
	{
		if (poly & 0x00000001)
			var |= 1 << (width - 1 - i);
		
		poly >>= 1;
	}
	return var;
}

static void crc32_init(uint32_t poly)
{
	uint32_t i, j;
	uint32_t c;
	
	poly = bitrev(poly, 32);
	
	for (i = 0; i < 256; i++)
	{
	    c = i;
		
		for (j = 0; j < 8; j++)
		{
			if(c & 0x00000001)
				c = poly ^ (c >> 1);
			else
				c >>= 1;
		}
		
		crc_tab[i] = c;
    }
}

static uint32_t crc32(uint32_t crc_init, uint8_t *buf, uint32_t len)
{
	uint32_t i;
	uint8_t index;

	for (i = 0; i < len; i++)
	{
		index = (uint8_t)(crc_init ^ buf[i]);
		crc_init = (crc_init >> 8) ^ crc_tab[index];
	}
	
	return crc_init;
}

uint32_t fota_crc(uint8_t *buf, uint32_t len)
{
	uint32_t crc_init = 0xffffffff;

	crc_init = crc32(crc_init, buf, len) ^ 0xffffffff;
	
	return crc_init;
}

uint32_t fota_step_crc(uint32_t crc, uint8_t *buf, uint32_t len)
{
	return crc32(crc, buf, len);
}

int fota_crc_init(void)
{
    #define POLYNOMIAL		0x04c11db7
	crc32_init(POLYNOMIAL);
	return 0;
}
