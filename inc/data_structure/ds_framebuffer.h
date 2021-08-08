/**
 * @file ds_framebuffer.h
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 1.0.0
 * @date 2021-08-08
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef __DS_FRAMEBUFFER_H
#define __DS_FRAMEBUFFER_H

#include "../middle_layer/rte.h"

typedef enum {
    FB_NO_FLAGS =   (0 << 0),
    FB_PEEK     =   (1 << 0),   // If set, will not move the head/tail.
} framebuffer_flags_t;

// DMA Buffers need to be aligned by cache lines or 16 bytes.
#ifndef __DCACHE_PRESENT
#define FRAMEBUFFER_ALIGNMENT 16
#else
#define FRAMEBUFFER_ALIGNMENT __SCB_DCACHE_LINE_SIZE
#endif

typedef struct framebuffer_impl {
    int32_t x,y;
    int32_t w,h;
    int32_t u,v;
    int32_t bpp;
    uint8_t *end_ptr;
    uint32_t raw_buffer_size;
    int32_t n_buffers;
    int32_t head;
    volatile int32_t tail;
    bool check_head;
    int32_t sampled_head;
    MEM_ALIGN_NBYTES(uint8_t data[], FRAMEBUFFER_ALIGNMENT);
} framebuffer_t;

typedef struct burstbuffer_impl {
    // Used by snapshot code to figure out the jpeg size (bpp).
    int32_t offset;
    bool jpeg_buffer_overflow;
    // Used internally by frame buffer code.
    volatile bool waiting_for_data;
    bool reset_state;
    // Image data array.
    MEM_ALIGN_NBYTES(uint8_t data[], FRAMEBUFFER_ALIGNMENT);
} burstbuffer_t;

ds_framebuffer_t framebuffer_init(uint32_t size);

int32_t framebuffer_get_x(ds_framebuffer_t pframebuffer);
int32_t framebuffer_get_y(ds_framebuffer_t pframebuffer);
int32_t framebuffer_get_u(ds_framebuffer_t pframebuffer);
int32_t framebuffer_get_v(ds_framebuffer_t pframebuffer);

int32_t framebuffer_get_width(ds_framebuffer_t pframebuffer);
int32_t framebuffer_get_height(ds_framebuffer_t pframebuffer);
int32_t framebuffer_get_depth(ds_framebuffer_t pframebuffer);

// Return the number of bytes in the current buffer.
uint32_t framebuffer_get_buffer_size(ds_framebuffer_t pframebuffer);

// Return the state of a buffer.
ds_burstbuffer_t framebuffer_get_buffer(ds_framebuffer_t pframebuffer, int32_t index);

// Initializes an image_t struct with the frame buffer.
void framebuffer_initialize_image(ds_framebuffer_t pframebuffer, ds_image_t pimg);

// Set the framebuffer w, h and bpp.
void framebuffer_set(ds_framebuffer_t pframebuffer, int32_t w, int32_t h, int32_t bpp);

// Clears out all old captures frames in the framebuffer.
void framebuffer_flush_buffers(ds_framebuffer_t pframebuffer);

// Resets all buffers (for use after aborting)
void framebuffer_reset_buffers(ds_framebuffer_t pframebuffer);

// Controls the number of virtual buffers in the frame buffer.
int framebuffer_set_buffers(ds_framebuffer_t pframebuffer, int32_t n_buffers);

// Automatically finds the best buffering size given RAM.
void framebuffer_auto_adjust_buffers(ds_framebuffer_t pframebuffer);

// Call when done with the current vbuffer to mark it as free.
void framebuffer_free_current_buffer(ds_framebuffer_t pframebuffer);

// Sets the current frame buffer to the latest virtual frame buffer.
// Returns the buffer if it is ready or NULL if not...
// Pass FB_PEEK to get the next buffer but not take it.
ds_burstbuffer_t framebuffer_get_head(ds_framebuffer_t pframebuffer, framebuffer_flags_t flags);

// Return the next vbuffer to store image data to or NULL if none.
// Pass FB_PEEK to get the next buffer but not commit it.
ds_burstbuffer_t framebuffer_get_tail(ds_framebuffer_t pframebuffer, framebuffer_flags_t flags);

// Returns a pointer to the end of the framebuffer(s).
char *framebuffer_get_buffers_end(ds_framebuffer_t pframebuffer);

#endif
