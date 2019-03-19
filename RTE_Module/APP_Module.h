#ifndef __APP_MODULE_H
#define __APP_MODULE_H
#include "RTE_Include.h"
#include "GUI_Include.h"
//配置
#define APP_USE_WALLPAPER   0
#define APP_FONT_SMALL   &lv_font_dejavu_10      /*A small font*/
#define APP_FONT_MEDIUM  &lv_font_dejavu_20      /*A medium font*/
#define APP_FONT_LARGE   &lv_font_dejavu_30      /*A large font*/
#define APP_SC_WIDTH     (70)        /*Shortcut width*/
#define APP_SC_HEIGHT    (APP_SC_WIDTH + 30)    /*Shortcut height*/
typedef enum
{
	APP_MODE_NONE 	 = 0x0000, 
	APP_MODE_NOT_LIST = 0x0001,		/*Do not list the application*/
	APP_MODE_NO_SC_TITLE = 0x0002,	/*No short cut title*/
}app_mode_t;
typedef enum
{
	APP_COM_TYPE_CHAR,   /*Stream of characters. Not '\0' terminated*/
	APP_COM_TYPE_INT,    /*Stream of 'int32_t' numbers*/
	APP_COM_TYPE_LOG,    /*String about an event to log*/
    APP_COM_TYPE_TRIG,   /*A trigger to do some specific action (data is ignored)*/
    APP_COM_TYPE_INV,    /*Invalid type*/
    APP_COM_TYPE_NUM,    /*Indicates the number of com. types*/
}app_com_type_t;
struct __APP_DSC_T;
typedef struct
{
	struct __APP_DSC_T * dsc;
	char * name;
	void *conf;
	lv_obj_t * icon;
	lv_obj_t * sc;
	lv_obj_t * sc_title;
	lv_obj_t * win;
    lv_obj_t * conf_win;
	void * app_data;
	void * sc_data;
	void * win_data;
}app_inst_t;
typedef struct __APP_DSC_T
{
	bool background;
	const char * name;
	app_mode_t mode;
	app_inst_t *inst;
	const lv_img_dsc_t *img;
	void (*app_run)(app_inst_t *, void *);
	void (*app_close) (app_inst_t *);
	void (*com_rec) (app_inst_t *, app_inst_t *, app_com_type_t, const void *, uint32_t);
	void (*sc_open) (app_inst_t *, lv_obj_t *);
	void (*sc_close) (app_inst_t *);
	void (*win_open) (app_inst_t *, lv_obj_t *);
	void (*win_close) (app_inst_t *);
    void (*conf_open) (app_inst_t *, lv_obj_t * );
	uint16_t app_data_size;
	uint16_t sc_data_size;
	uint16_t win_data_size;
}app_dsc_t;
typedef struct {
	lv_style_t sc_page;
	lv_style_t menu;
    lv_style_t menu_btn_rel;
    lv_style_t menu_btn_pr;
    lv_style_t sc_rel;
    lv_style_t sc_pr;
    lv_style_t sc_title;
    lv_style_t win_header;
    lv_style_t win_scrl;
    lv_style_t win_cbtn_rel;
    lv_style_t win_cbtn_pr;
}app_style_t;
typedef struct
{
	//--程序总得APP得一些参数
	const char * name;
	lv_obj_t * scr;   /*Screen of the applications*/
	lv_obj_t * menuh;     /*Holder of timg_bubbleshe menu on the top*/
	lv_obj_t * btn;   /*The "Apps" button on the menu*/
	lv_obj_t * labelbtn;
	lv_obj_t * sc_page;   /*A page for the shortcuts */
	lv_obj_t * list;      /*A list which is opened on 'app_btn' release*/
	//--状态栏
	lv_obj_t *iconinfor[4];
	lv_obj_t *labelinfor[8];
	app_inst_t * con_send; /*The sender application in connection mode. Not NLL means connection mode is active*/
	app_style_t style; /*Styles for application related things*/
	//--消息收发
	uint8_t MSGTimerID;
	
}app_control_t;

extern void APP_Init(lv_obj_t *father,const char *name);
extern void APP_Add(app_dsc_t *app,void * conf);
extern app_inst_t * APP_New(app_dsc_t * app_dsc, void * conf);
extern void APP_Remove(app_inst_t * app);

extern lv_obj_t * APP_GUI_Init(app_inst_t * app);
extern void APP_GUI_DeInit(app_inst_t * app);

extern lv_obj_t * APP_Win_Open(app_inst_t * app);
extern void APP_Win_Close(app_inst_t * app);

extern void APP_Rename(app_inst_t * app, const char * name);
extern app_dsc_t * APP_Dsc_Get(const char * name);

extern bool APP_Con_Check(app_inst_t * sender, app_inst_t * receiver);
extern void APP_Con_Set(app_inst_t * sender, app_inst_t * receiver);
extern void APP_Con_Del(app_inst_t * sender, app_inst_t * receiver);
extern uint16_t APP_Con_Send(app_inst_t * app_send, app_com_type_t type , const void * data, uint32_t size);

#endif
