#ifndef __MV_FRAMEBUFFER_H
#define __MV_FRAMEBUFFER_H
#include "RTE_Include.h"
typedef struct framebuffer {
	int x,y;
	int w,h;
	int u,v;
	image_bpp_t bpp;
	uint8_t *pixels;
}framebuffer_t;
extern void MV_FB_Init(uint8_t *buffer);
extern framebuffer_t *MV_FB_Get(void);
extern uint32_t MV_FB_GetSize(void);
#endif
