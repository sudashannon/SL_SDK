/**
 * @file monitor.h
 *
 */

#ifndef MONITOR_H
#define MONITOR_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
/*-------------------
 *  Monitor of PC
 *-------------------*/
#ifndef USE_MONITOR
#  define USE_MONITOR         1
#endif

#if USE_MONITOR
#  define MONITOR_HOR_RES     480
#  define MONITOR_VER_RES     320

/* Scale window by this factor (useful when simulating small screens) */
#  define MONITOR_ZOOM        1

/* Used to test true double buffering with only address changing.
 * Use 2 draw buffers, bith with MONITOR_HOR_RES x MONITOR_VER_RES size*/
#  define MONITOR_DOUBLE_BUFFERED 0

/*Eclipse: <SDL2/SDL.h>    Visual Studio: <SDL.h>*/
#  define MONITOR_SDL_INCLUDE_PATH    <SDL.h>

/*Open two windows to test multi display support*/
#  define MONITOR_DUAL            0
#endif

#if USE_MONITOR

#include "lvgl.h"
/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void monitor_init(void);
void monitor_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);
void monitor_flush2(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);

/**********************
 *      MACROS
 **********************/

#endif /* USE_MONITOR */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MONITOR_H */
