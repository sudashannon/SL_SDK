#ifndef __MV_IMAGE_H
#define __MV_IMAGE_H
#include "RTE_Include.h"
typedef enum image_bpp
{
	IMAGE_BPP_BINARY,       // BPP = 0
	IMAGE_BPP_GRAYSCALE,    // BPP = 1
	IMAGE_BPP_RGB565,       // BPP = 2
	IMAGE_BPP_BAYER,        // BPP = 3
	IMAGE_BPP_JPEG          // BPP > 3
}image_bpp_t;
typedef struct image{
	int w;
	int h;
	int bpp;
	union {
		uint8_t *pixels;
		uint8_t *data;
	};
}image_t;
#define IM_R565(p) \
    ({ __typeof__ (p) _p = (p); \
       ((_p)>>3)&0x1F; })

#define IM_G565(p) \
    ({ __typeof__ (p) _p = (p); \
       (((_p)&0x07)<<3)|((_p)>>13); })

#define IM_B565(p) \
    ({ __typeof__ (p) _p = (p); \
       ((_p)>>8)&0x1F; })
		
// RGB888 to RGB565 conversion.
extern const uint8_t rb825_table[256];
extern const uint8_t g826_table[256];

#define IM_R825(p) \
    ({ __typeof__ (p) _p = (p); \
       rb825_table[_p]; })

#define IM_G826(p) \
    ({ __typeof__ (p) _p = (p); \
       g826_table[_p]; })

#define IM_B825(p) \
    ({ __typeof__ (p) _p = (p); \
       rb825_table[_p]; })
#endif
