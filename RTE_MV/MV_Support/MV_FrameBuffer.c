#include "MV_FrameBuffer.h"
static framebuffer_t MVFramebuffer = {0};
void MV_FB_Init(uint8_t *buffer)
{
	MVFramebuffer.pixels = buffer;
}
framebuffer_t *MV_FB_Get(void)
{
	return (&MVFramebuffer);
}
uint32_t MV_FB_GetSize(void)
{
	switch (MVFramebuffer.bpp) {
		case IMAGE_BPP_BINARY: 
			return ((MVFramebuffer.w + UINT32_T_MASK) >> UINT32_T_SHIFT) * MVFramebuffer.h;
		case IMAGE_BPP_GRAYSCALE: 
			return (MVFramebuffer.w * MVFramebuffer.h) * sizeof(uint8_t);
		case IMAGE_BPP_RGB565: 
			return (MVFramebuffer.w * MVFramebuffer.h) * sizeof(uint16_t);
		case IMAGE_BPP_BAYER: 
			return MVFramebuffer.w * MVFramebuffer.h;
		default:
			return MVFramebuffer.bpp;
	}
}


