/**
 * @file lvgl.h
 * Include all LittleV GL related headers
 */

#ifndef LVGL_H
#define LVGL_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

/*Test misc. module version*/
#include "GUI_Core/lv_obj.h"
#include "GUI_Core/lv_group.h"
#include "GUI_Core/lv_vdb.h"
#include "GUI_Core/lv_refr.h"

#include "GUI_Theme/lv_theme.h"

#include "GUI_Hal/lv_hal_indev.h"
#include "GUI_Hal/lv_hal_disp.h"

#include "GUI_Objx/lv_btn.h"
#include "GUI_Objx/lv_imgbtn.h"
#include "GUI_Objx/lv_img.h"
#include "GUI_Objx/lv_label.h"
#include "GUI_Objx/lv_line.h"
#include "GUI_Objx/lv_page.h"
#include "GUI_Objx/lv_cont.h"
#include "GUI_Objx/lv_list.h"
#include "GUI_Objx/lv_chart.h"
#include "GUI_Objx/lv_cb.h"
#include "GUI_Objx/lv_bar.h"
#include "GUI_Objx/lv_slider.h"
#include "GUI_Objx/lv_led.h"
#include "GUI_Objx/lv_btnm.h"
#include "GUI_Objx/lv_kb.h"
#include "GUI_Objx/lv_ddlist.h"
#include "GUI_Objx/lv_roller.h"
#include "GUI_Objx/lv_ta.h"
#include "GUI_Objx/lv_win.h"
#include "GUI_Objx/lv_tabview.h"
#include "GUI_Objx/lv_mbox.h"
#include "GUI_Objx/lv_gauge.h"
#include "GUI_Objx/lv_lmeter.h"
#include "GUI_Objx/lv_sw.h"
#include "GUI_Objx/lv_kb.h"
#include "GUI_Objx/lv_arc.h"
#include "GUI_Objx/lv_preload.h"
#include "GUI_Objx/lv_calendar.h"
/*********************
 *      DEFINES
 *********************/
/*Current version of LittlevGL*/
#define LVGL_VERSION_MAJOR   5
#define LVGL_VERSION_MINOR   2
#define LVGL_VERSION_PATCH   1
#define LVGL_VERSION_INFO    "dev"

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
}
#endif

#endif /*LVGL_H*/
