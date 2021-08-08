/**
 * @file ds_framebuffer.c
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 1.0.0
 * @date 2021-08-08
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "../../inc/data_structure/ds_framebuffer.h"
#include "../../inc/data_structure/ds_image.h"
#include "../../inc/middle_layer/rte_memory.h"

#define FB_ALIGN_SIZE_ROUND_DOWN(x) (((x) / FRAMEBUFFER_ALIGNMENT) * FRAMEBUFFER_ALIGNMENT)
#define FB_ALIGN_SIZE_ROUND_UP(x)   FB_ALIGN_SIZE_ROUND_DOWN(((x) + FRAMEBUFFER_ALIGNMENT - 1))

// RAW buffer size
#define OMV_RAW_BUF_SIZE        (409600)

ds_framebuffer_t framebuffer_init(uint32_t size)
{
    uint8_t *buffer = memory_calloc(BANK_FB, size);
    if (buffer == NULL)
        return buffer;
    framebuffer_t *framebuffer = (framebuffer_t *)buffer;
    framebuffer->end_ptr = size + buffer;
    // Setup buffering.
    framebuffer_set_buffers(framebuffer, 1);
    return framebuffer;
}

void framebuffer_initialize_image(ds_framebuffer_t pframebuffer, ds_image_t pimg)
{
    framebuffer_t *framebuffer = (framebuffer_t *)pframebuffer;
    image_t *img = (image_t *)pimg;
    img->w = framebuffer->w;
    img->h = framebuffer->h;
    img->bpp = framebuffer->bpp;
    img->data = ((framebuffer_t *)framebuffer_get_buffer(pframebuffer, framebuffer->head))->data;
}

int32_t framebuffer_get_x(ds_framebuffer_t pframebuffer)
{
    framebuffer_t *framebuffer = (framebuffer_t *)pframebuffer;
    return framebuffer->x;
}

int32_t framebuffer_get_y(ds_framebuffer_t pframebuffer)
{
    framebuffer_t *framebuffer = (framebuffer_t *)pframebuffer;
    return framebuffer->y;
}

int32_t framebuffer_get_u(ds_framebuffer_t pframebuffer)
{
    framebuffer_t *framebuffer = (framebuffer_t *)pframebuffer;
    return framebuffer->u;
}

int32_t framebuffer_get_v(ds_framebuffer_t pframebuffer)
{
    framebuffer_t *framebuffer = (framebuffer_t *)pframebuffer;
    return framebuffer->v;
}

int32_t framebuffer_get_width(ds_framebuffer_t pframebuffer)
{
    framebuffer_t *framebuffer = (framebuffer_t *)pframebuffer;
    return framebuffer->w;
}

int32_t framebuffer_get_height(ds_framebuffer_t pframebuffer)
{
    framebuffer_t *framebuffer = (framebuffer_t *)pframebuffer;
    return framebuffer->h;
}

int32_t framebuffer_get_depth(ds_framebuffer_t pframebuffer)
{
    framebuffer_t *framebuffer = (framebuffer_t *)pframebuffer;
    return framebuffer->bpp;
}

// Returns the number of bytes the frame buffer could be at the current moment it time.
static uint32_t framebuffer_raw_buffer_size(ds_framebuffer_t pframebuffer)
{
    framebuffer_t *framebuffer = (framebuffer_t *)pframebuffer;
    uint32_t size = (uint32_t) (framebuffer->end_ptr - ((uint8_t *) framebuffer->data));
    // We don't want to give all of the frame buffer RAM to the frame buffer. So, we will limit
    // the maximum amount of RAM we return.
    return RTE_MIN(size, OMV_RAW_BUF_SIZE);
}

uint32_t framebuffer_get_buffer_size(ds_framebuffer_t pframebuffer)
{
    uint32_t size;
    framebuffer_t *framebuffer = (framebuffer_t *)pframebuffer;
    if (framebuffer->n_buffers == 1) {
        // With only 1 vbuffer it's fine to allow the frame buffer size to change given fb_alloc().
        size = framebuffer_raw_buffer_size(pframebuffer);
    } else {
        // Whatever the raw size was when the number of buffers were set is locked in...
        size = framebuffer->raw_buffer_size;
    }

    // Remove the size of the state header plus alignment padding.
    size -= sizeof(burstbuffer_t);

    // Do we have an estimate on the frame size with mutliple buffers? If so, we can reduce the
    // RAM each buffer takes up giving some space back to fb_alloc().
    if ((framebuffer->n_buffers != 1) && framebuffer->u && framebuffer->v) {
        // Typically a framebuffer will not need more than u*v*2 bytes.
        uint32_t size_guess = framebuffer->u * framebuffer->v * 2;
        // Add in extra bytes to prevent round down from shrinking buffer too small.
        size_guess += FRAMEBUFFER_ALIGNMENT - 1;
        // Limit the frame buffer size.
        size = RTE_MIN(size, size_guess);
    }

    // Needs to be a multiple of FRAMEBUFFER_ALIGNMENT for DMA transfers...
    return FB_ALIGN_SIZE_ROUND_DOWN(size);
}

// Each raw frame buffer is split into two parts. The vbuffer_t struct followed by
// padding and then the pixel array starting at the next 32-byte offset.
ds_burstbuffer_t framebuffer_get_buffer(ds_framebuffer_t pframebuffer, int32_t index)
{
    framebuffer_t *framebuffer = (framebuffer_t *)pframebuffer;
    uint32_t offset = (sizeof(burstbuffer_t) + framebuffer_get_buffer_size(pframebuffer)) * index;
    return (ds_burstbuffer_t) (framebuffer->data + offset);
}

void framebuffer_set(ds_framebuffer_t pframebuffer, int32_t w, int32_t h, int32_t bpp)
{
    framebuffer_t *framebuffer = (framebuffer_t *)pframebuffer;
    framebuffer->w = w;
    framebuffer->h = h;
    framebuffer->bpp = bpp;
}

void framebuffer_flush_buffers(ds_framebuffer_t pframebuffer)
{
    framebuffer_t *framebuffer = (framebuffer_t *)pframebuffer;
    // Move the tail pointer to the head which empties the virtual fifo while keeping the same
    // position of the current frame for the rest of the code.
    framebuffer->tail = framebuffer->head;
    framebuffer->check_head = true;
    framebuffer->sampled_head = 0;
}

void framebuffer_reset_buffers(ds_framebuffer_t pframebuffer)
{
    framebuffer_t *framebuffer = (framebuffer_t *)pframebuffer;
    for (int32_t i = 0; i < framebuffer->n_buffers; i++) {
        memset(framebuffer_get_buffer(pframebuffer, i), 0, sizeof(burstbuffer_t));
    }

    framebuffer_flush_buffers(pframebuffer);
}

int framebuffer_set_buffers(ds_framebuffer_t pframebuffer, int32_t n_buffers)
{
    framebuffer_t *framebuffer = (framebuffer_t *)pframebuffer;
    uint32_t total_size = framebuffer_raw_buffer_size(pframebuffer);
    uint32_t size = total_size / n_buffers;

    // Error out if frame buffers are smaller than this...
    if (size < (sizeof(burstbuffer_t) + FRAMEBUFFER_ALIGNMENT)) {
        return -1;
    }

    // Invalidate frame.
    framebuffer->bpp = -1;

    // Cache the maximum size we can allocate for the frame buffer when vbuffers are greater than 1.
    framebuffer->raw_buffer_size = size;
    framebuffer->n_buffers = n_buffers;
    framebuffer->head = 0;

    framebuffer_reset_buffers(pframebuffer);

    return 0;
}

// Returns the real size of bytes in the frame buffer.
static uint32_t framebuffer_total_buffer_size(ds_framebuffer_t pframebuffer)
{
    framebuffer_t *framebuffer = (framebuffer_t *)pframebuffer;
    if (framebuffer->n_buffers == 1) {
        // Allow fb_alloc to use frame buffer space up until the image size.
        image_t img;
        framebuffer_initialize_image(pframebuffer, &img);
        return sizeof(burstbuffer_t) + FB_ALIGN_SIZE_ROUND_UP(image_size(&img));
    } else {
        // fb_alloc may only use up to the size of all the virtual buffers...
        return (sizeof(burstbuffer_t) + framebuffer_get_buffer_size(pframebuffer)) * framebuffer->n_buffers;
    }
}

void framebuffer_auto_adjust_buffers(ds_framebuffer_t pframebuffer)
{
    framebuffer_t *framebuffer = (framebuffer_t *)pframebuffer;
     // Keep same buffer count in video fifo mode but resize buffer sizes.
    framebuffer_set_buffers(pframebuffer, framebuffer->n_buffers);
}

void framebuffer_free_current_buffer(ds_framebuffer_t pframebuffer)
{
    framebuffer_t *framebuffer = (framebuffer_t *)pframebuffer;
    // Invalidate frame.
    framebuffer->bpp = -1;
    // Allow frame to be updated in single buffer mode...
    if (framebuffer->n_buffers == 1) {
        ((burstbuffer_t *)framebuffer_get_buffer(pframebuffer, framebuffer->head))->waiting_for_data = true;
    }
}

ds_burstbuffer_t framebuffer_get_head(ds_framebuffer_t pframebuffer, framebuffer_flags_t flags)
{
    framebuffer_t *framebuffer = (framebuffer_t *)pframebuffer;
    int32_t new_head = (framebuffer->head + 1) % framebuffer->n_buffers;
    // Single Buffer Mode.
    if (framebuffer->n_buffers == 1) {
        if (((burstbuffer_t *)framebuffer_get_buffer(pframebuffer, framebuffer->head))->waiting_for_data) {
            return NULL;
        }
    // Double Buffer Mode.
    } else if (framebuffer->n_buffers == 2) {
        if (framebuffer->head == framebuffer->tail) {
            return NULL;
        }
    // Triple Buffer Mode.
    } else if (framebuffer->n_buffers == 3) {
        int32_t sampled_tail = framebuffer->tail;
        if (framebuffer->head == sampled_tail) {
            return NULL;
        } else {
            new_head = sampled_tail;
        }
    // Video FIFO Mode.
    } else {
        if (framebuffer->head == framebuffer->tail) {
            return NULL;
        }
    }

    if (!(flags & FB_PEEK)) {
        framebuffer->head = new_head;
    }

    return framebuffer_get_buffer(pframebuffer, new_head);
}

ds_burstbuffer_t framebuffer_get_tail(ds_framebuffer_t pframebuffer, framebuffer_flags_t flags)
{
    framebuffer_t *framebuffer = (framebuffer_t *)pframebuffer;
    // Sample head on the first line of a new frame.
    if (framebuffer->check_head) {
        framebuffer->check_head = false;
        framebuffer->sampled_head = framebuffer->head;
    }

    int32_t new_tail = (framebuffer->tail + 1) % framebuffer->n_buffers;

    // Single Buffer Mode.
    if (framebuffer->n_buffers == 1) {
        if (!((burstbuffer_t *)framebuffer_get_buffer(pframebuffer, new_tail))->waiting_for_data) {
            // Setup to check head again.
            framebuffer->check_head = true;
            return NULL;
        }
    // Double Buffer Mode.
    } else if (framebuffer->n_buffers == 2) {
        if (new_tail == framebuffer->sampled_head) {
            // Setup to check head again.
            framebuffer->check_head = true;
            return NULL;
        }
    // Triple Buffer Mode.
    } else if (framebuffer->n_buffers == 3) {
        // For triple buffering we are never writing where tail or head
        // (which may instantly update to be equal to tail) is.
        if (new_tail == framebuffer->sampled_head) {
            new_tail = (new_tail + 1) % framebuffer->n_buffers;
        }
    // Video FIFO Mode.
    } else {
        if (new_tail == framebuffer->sampled_head) {
            // Setup to check head again.
            framebuffer->check_head = true;
            return NULL;
        }
    }

    burstbuffer_t *buffer = framebuffer_get_buffer(pframebuffer, new_tail);

    // Reset on start versus the end so offset and jpeg_buffer_overflow are valid after FB_COMMIT.
    if (buffer->reset_state) {
        buffer->reset_state = false;
        buffer->offset = 0;
        buffer->jpeg_buffer_overflow = false;
    }

    if (!(flags & FB_PEEK)) {
        // Trigger reset on the frame buffer the next time it is used.
        buffer->reset_state = true;

        // Mark the frame buffer ready in single buffer mode.
        if (framebuffer->n_buffers == 1) {
            buffer->waiting_for_data = false;
        }

        framebuffer->tail = new_tail;

        // Setup to check head again.
        framebuffer->check_head = true;
    }

    return buffer;
}

char *framebuffer_get_buffers_end(ds_framebuffer_t pframebuffer)
{
    framebuffer_t *framebuffer = (framebuffer_t *)pframebuffer;
    return (char *) (framebuffer->data + framebuffer_total_buffer_size(pframebuffer));
}
