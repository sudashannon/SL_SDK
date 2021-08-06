/**
 * @file keyboard.h
 *
 */

#ifndef KEYBOARD_H
#define KEYBOARD_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#define USE_KEYBOARD        1

#if USE_KEYBOARD

#include "lvgl.h"

#ifndef MONITOR_SDL_INCLUDE_PATH
#define MONITOR_SDL_INCLUDE_PATH <SDL.h>
#endif

#include MONITOR_SDL_INCLUDE_PATH

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
/**
 * Initialize the keyboard
 */
void keyboard_init(void);

/**
 * Get the last pressed or released character from the PC's keyboard
 * @param indev_drv pointer to the related input device driver
 * @param data store the read data here
 */
void keyboard_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);

/**
 * It is called periodically from the SDL thread to check a key is pressed/released
 * @param event describes the event
 */
void keyboard_handler(SDL_Event *event);

/**********************
 *      MACROS
 **********************/

#endif /*USE_KEYBOARD*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*KEYBOARD_H*/
