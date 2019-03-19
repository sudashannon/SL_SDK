/*****************************************************************************
*** Author: Shannon
*** Version: 1.0 2019.03.18
*** History: 1.0 创建，适配SL_RTE框架得APP模块
*** Issue：  
*****************************************************************************/
#include "APP_Module.h"
#include "Board_RTC.h"
#if APP_USE_WALLPAPER == 1
#include "img_wallpaper.inc"
#endif
typedef struct {
    app_inst_t * sender;
    app_inst_t * receiver;
}app_con_t;
//程序描述链表
static RTE_LL_t app_dsc_ll;
//程序GUI链表
static RTE_LL_t app_inst_ll;
//程序通讯链表
static RTE_LL_t app_con_ll;

//APP内交互动作回调函数
static lv_res_t app_menu_rel_action(lv_obj_t * app_btn);
static lv_res_t app_menu_elem_rel_action(lv_obj_t * app_elem_btn);
static lv_res_t app_sc_page_rel_action(lv_obj_t * page);
static lv_res_t app_sc_rel_action(lv_obj_t * sc);
static lv_res_t app_win_open_anim_create(app_inst_t * app);
static lv_res_t app_win_minim_action(lv_obj_t * minim_btn);
static lv_res_t app_win_close_action(lv_obj_t * close_btn);
static lv_res_t app_win_conf_action(lv_obj_t * set_btn);
static void app_menu_rel_animate_callback(lv_obj_t *list);
//APP内GUI组定时器回调函数
static void app_statusbar_refreshtime_timercallback(void *arg);
static void app_statusbar_refreshcpu_timercallback(void *arg);
//APP消息接收函数
static void overall_com_rec(app_inst_t * app_send, app_inst_t * app_rec,
                       app_com_type_t type , const void * data, uint32_t size);
//主界面程序句柄
static app_control_t app_handle = {0};
static app_dsc_t overall_app_dsc =
{
	.img = NULL,
	.name = "OverAll",
	.mode = APP_MODE_NONE,
	.inst = NULL,
	.background = false,
	.app_run = NULL,
	.app_close = NULL,
	.com_rec = overall_com_rec,
	.win_open = NULL,
	.win_close = NULL,
	.sc_open = NULL,
	.sc_close = NULL,
	.app_data_size = 0,
	.sc_data_size = 0,
	.win_data_size = 0,
};
/*************************************************
*** Args:   总APP名称
*** Function: 初始化总APP
*************************************************/
void APP_Init(lv_obj_t *father,const char *name)
{
#if APP_USE_WALLPAPER == 1
    lv_obj_t * wp = lv_img_create(lv_scr_act(), NULL);
    lv_img_set_src(wp, &img_bubble_pattern);
    lv_obj_set_width(wp, LV_HOR_RES * 4);
    lv_obj_set_protect(wp, LV_PROTECT_POS);
#endif
	app_handle.name = name;
    //初始化链表
	RTE_LL_Init(&app_dsc_ll, sizeof(app_dsc_t *));
	RTE_LL_Init(&app_inst_ll, sizeof(app_inst_t));
    RTE_LL_Init(&app_con_ll, sizeof(app_con_t));
	//风格
	//菜单风格 即最上方包含状态栏在内得标题栏
	lv_style_copy(&app_handle.style.menu,&lv_style_plain);
    app_handle.style.menu.image.color = LV_COLOR_WHITE;
	app_handle.style.menu.text.font = APP_FONT_SMALL;
	app_handle.style.menu.text.color = LV_COLOR_WHITE;
	app_handle.style.menu.body.main_color = LV_COLOR_MAKE(0x30, 0x30, 0x30);
	app_handle.style.menu.body.grad_color = LV_COLOR_MAKE(0x30, 0x30, 0x30);
    app_handle.style.menu.body.border.color = LV_COLOR_MAKE(0x80, 0x80, 0x80);
	app_handle.style.menu.body.border.width = 2;
	app_handle.style.menu.body.shadow.color = LV_COLOR_MAKE(230,230,250);
	app_handle.style.menu.body.shadow.width = 3;
	app_handle.style.menu.body.opa = 0x80;
	app_handle.style.menu.body.radius = 0;
    app_handle.style.menu.body.padding.ver = LV_DPI / 12;
    app_handle.style.menu.body.padding.hor = LV_DPI / 12;
    app_handle.style.menu.body.border.opa  = LV_DPI / 12;
	//菜单按钮风格 即最左侧可以查看后台程序得按钮
    lv_style_copy(&app_handle.style.menu_btn_rel,&lv_style_btn_rel);
    app_handle.style.menu_btn_rel.image.color = LV_COLOR_WHITE;
	app_handle.style.menu_btn_rel.text.color = LV_COLOR_WHITE;
    app_handle.style.menu_btn_rel.body.main_color = LV_COLOR_MAKE(0x30, 0x30, 0x30);
    app_handle.style.menu_btn_rel.body.grad_color = LV_COLOR_MAKE(0x30, 0x30, 0x30);
    app_handle.style.menu_btn_rel.body.border.color = LV_COLOR_MAKE(0xa0, 0xa0, 0xa0);
    app_handle.style.menu_btn_rel.body.border.opa = LV_OPA_20;
    app_handle.style.menu_btn_rel.body.border.width = 0;

    app_handle.style.menu_btn_rel.body.radius = 0;
	app_handle.style.menu_btn_rel.body.shadow.width = 0;
    app_handle.style.menu_btn_rel.body.empty = 1;
    app_handle.style.menu_btn_rel.text.font = APP_FONT_MEDIUM;
    app_handle.style.menu_btn_rel.image.opa = LV_OPA_90;
    app_handle.style.menu_btn_rel.body.padding.ver = LV_DPI / 10;
    app_handle.style.menu_btn_rel.body.padding.hor = LV_DPI / 10;
    app_handle.style.menu_btn_rel.body.padding.inner  = LV_DPI / 10;

    memcpy(&app_handle.style.menu_btn_pr, &app_handle.style.menu_btn_rel, sizeof(lv_style_t));
	app_handle.style.menu_btn_pr.body.main_color = LV_COLOR_GRAY;
	app_handle.style.menu_btn_pr.body.grad_color = LV_COLOR_GRAY;
    app_handle.style.menu_btn_pr.body.border.color = LV_COLOR_GRAY;
    app_handle.style.menu_btn_pr.body.border.width = 0;
    app_handle.style.menu_btn_pr.body.radius = 0;
    app_handle.style.menu_btn_pr.body.empty = 0;
    app_handle.style.menu_btn_pr.body.shadow.width = 0;

	//APP风格 每个APP包含若干GUI控件
	lv_style_copy(&app_handle.style.sc_rel,&lv_style_transp);
	app_handle.style.sc_rel.text.font = APP_FONT_MEDIUM;
	app_handle.style.sc_rel.text.color = LV_COLOR_WHITE;
	app_handle.style.sc_rel.text.letter_space = 1; 

	lv_style_copy(&app_handle.style.sc_pr,&lv_style_plain);
	app_handle.style.sc_pr.body.opa = 80;
	app_handle.style.sc_pr.body.main_color = LV_COLOR_MAKE(0x87, 0xCE, 0xFA);
	app_handle.style.sc_pr.body.grad_color = LV_COLOR_MAKE(0x1E, 0x90, 0xEF);
	app_handle.style.sc_pr.text.font = APP_FONT_MEDIUM;
	app_handle.style.sc_pr.text.color = LV_COLOR_WHITE;
	app_handle.style.sc_pr.text.letter_space = 1;

    memcpy(&app_handle.style.sc_title, &app_handle.style.sc_rel, sizeof(lv_style_t));
	app_handle.style.sc_title.text.font = APP_FONT_MEDIUM;
	
	/*Window*/
	lv_style_copy(&app_handle.style.win_header,&lv_style_plain_color);
    memcpy(&app_handle.style.win_header, &app_handle.style.menu, sizeof(lv_style_t));
    app_handle.style.win_header.text.font = APP_FONT_MEDIUM;

    lv_style_copy(&app_handle.style.win_scrl,&lv_style_transp);

    lv_style_copy( &app_handle.style.win_cbtn_rel,&lv_style_btn_rel);
    memcpy(&app_handle.style.win_cbtn_rel, &app_handle.style.menu_btn_rel, sizeof(lv_style_t));
    app_handle.style.win_cbtn_rel.text.font = APP_FONT_MEDIUM;

    lv_style_copy(&app_handle.style.win_cbtn_pr,&lv_style_btn_pr);
    memcpy(&app_handle.style.win_cbtn_pr, &app_handle.style.menu_btn_pr, sizeof(lv_style_t));
    app_handle.style.win_cbtn_pr.text.font = APP_FONT_MEDIUM;
	
	//主界面
	lv_style_copy(&app_handle.style.sc_page,&lv_style_plain_color);
	app_handle.style.sc_page.body.main_color = LV_COLOR_MAKE(176,196,222);
	app_handle.style.sc_page.body.grad_color = LV_COLOR_MAKE(176,196,222);
    //根控件
	app_handle.scr = father;
	//--状态栏
    app_handle.menuh = lv_cont_create(app_handle.scr, NULL);
    lv_cont_set_fit(app_handle.menuh, false, false);
	lv_obj_set_size(app_handle.menuh, LV_HOR_RES,40);
    lv_obj_set_style(app_handle.menuh, &app_handle.style.menu);
	//---电池图标
	app_handle.iconinfor[0] = lv_img_create(app_handle.menuh,NULL);
	lv_img_set_src(app_handle.iconinfor[0],SYMBOL_BATTERY_FULL);
	lv_obj_align(app_handle.iconinfor[0],app_handle.menuh,LV_ALIGN_IN_RIGHT_MID,-15,0);
	//---wifi图标
	app_handle.iconinfor[1] = lv_img_create(app_handle.menuh,NULL);
	lv_img_set_src(app_handle.iconinfor[1],SYMBOL_CLOSE);
	lv_obj_align(app_handle.iconinfor[1],app_handle.iconinfor[0],LV_ALIGN_IN_LEFT_MID,-15,0);
	//---时间信息
	static lv_style_t labelstyle;
	app_handle.labelinfor[0] = lv_label_create(app_handle.menuh, NULL);
	lv_style_copy(&labelstyle,lv_label_get_style(app_handle.labelinfor[0]));
	labelstyle.text.font = &lv_font_dejavu_10;
	labelstyle.text.color = LV_COLOR_MAKE(248,248,255);
	lv_label_set_style(app_handle.labelinfor[0],&labelstyle);
	RTC_TimeTypeDef RTC_TimeStructure;
	RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
	char buf[32] = {0};
	usprintf(buf,"%02d:%02d:%02d",RTC_TimeStructure.RTC_Hours,RTC_TimeStructure.RTC_Minutes,RTC_TimeStructure.RTC_Seconds);
	lv_label_set_text(app_handle.labelinfor[0], buf);
	lv_obj_align(app_handle.labelinfor[0],app_handle.iconinfor[1],LV_ALIGN_OUT_LEFT_MID,-20,0);
	RTE_RoundRobin_CreateTimer(1,"StatusbarTime",500,1,1,app_statusbar_refreshtime_timercallback,app_handle.labelinfor[0]);
	//---cpu使用率
	app_handle.labelinfor[1] = lv_label_create(app_handle.menuh, NULL);
	lv_label_set_style(app_handle.labelinfor[1],&labelstyle);
	lv_label_set_text(app_handle.labelinfor[1], "CPU:100%");
	lv_obj_align(app_handle.labelinfor[1],app_handle.labelinfor[0],LV_ALIGN_OUT_LEFT_MID,-20,0);
	RTE_RoundRobin_CreateTimer(1,"StatusbarCPU",25,1,1,app_statusbar_refreshcpu_timercallback,app_handle.labelinfor[1]);
	
	
    app_handle.btn = lv_btn_create(app_handle.menuh, NULL);
    lv_btn_set_style(app_handle.btn,LV_BTN_STATE_REL, &app_handle.style.menu_btn_rel);
	lv_btn_set_style(app_handle.btn,LV_BTN_STATE_PR, &app_handle.style.menu_btn_pr);
    lv_cont_set_fit(app_handle.btn, true, false);
	lv_obj_set_height(app_handle.btn,40);
    lv_btn_set_action(app_handle.btn,LV_BTN_ACTION_CLICK, app_menu_rel_action);
    app_handle.labelbtn = lv_label_create(app_handle.btn, NULL);
    lv_label_set_text(app_handle.labelbtn, app_handle.name);
    lv_obj_set_pos(app_handle.btn, 0, 0);
    lv_obj_set_pos(app_handle.menuh, 0, 0);

    /*Shortcut area*/
	app_handle.sc_page = lv_page_create(app_handle.scr, NULL);
	lv_obj_set_style(app_handle.sc_page, &app_handle.style.sc_page);
	lv_obj_set_style(lv_page_get_scrl(app_handle.sc_page), &lv_style_transp);
	
	lv_obj_set_size(app_handle.sc_page, LV_HOR_RES, LV_VER_RES - lv_obj_get_height(app_handle.menuh));
	lv_obj_set_pos(app_handle.sc_page, 0, lv_obj_get_height(app_handle.menuh));
	lv_obj_set_width(lv_page_get_scrl(app_handle.sc_page), LV_HOR_RES - 20);
	lv_cont_set_fit(lv_page_get_scrl(app_handle.sc_page), false, true);
	lv_cont_set_layout(lv_page_get_scrl(app_handle.sc_page), LV_LAYOUT_PRETTY);
	lv_page_set_rel_action(app_handle.sc_page, app_sc_page_rel_action);
	lv_page_set_sb_mode(app_handle.sc_page, LV_SB_MODE_AUTO);
	
	//添加overall APP
	const app_dsc_t ** dsc;
	dsc = RTE_LL_InsHead(&app_dsc_ll);
	*dsc = &overall_app_dsc;
	APP_New(&overall_app_dsc, &app_handle);
}
/*************************************************
*** Args:   待添加APP名称
*** Function: 添加一个APP 并为其初始化GUI界面
*************************************************/
void APP_Add(app_dsc_t *app,void * conf)
{
	const app_dsc_t ** dsc;
	dsc = RTE_LL_InsHead(&app_dsc_ll);
	*dsc = app;
	app_inst_t * appinst = APP_New(app, conf);
	APP_GUI_Init(appinst);
}
/**
 * Test an application communication connection
 * @param sender pointer to an application which sends data
 * @param receiver pointer to an application which receives data
 * @return false: no connection, true: there is connection
 */
bool APP_Con_Check(app_inst_t * sender, app_inst_t * receiver)
{
    app_con_t * con;
    LL_READ(app_con_ll, con) {
        if(con->sender == sender && con->receiver == receiver) {
            return true;
        }
    }
    return false;
}
/**
 * Create a new connection between two applications
 * @param sender pointer to a data sender application
 * @param receiver pointer to a data receiver application
 */
void APP_Con_Set(app_inst_t * sender, app_inst_t * receiver)
{
    if(APP_Con_Check(sender, receiver) == false) {
        app_con_t * con;
        con = RTE_LL_InsHead(&app_con_ll);
        con->sender = sender;
        con->receiver = receiver;
    }
}
/**
 * Delete a communication connection
 * @param sender pointer to a data sender application or NULL to be true for all sender
 * @param receiver pointer to a data receiver application  or NULL to be true for all receiver
 */
void APP_Con_Del(app_inst_t * sender, app_inst_t * receiver)
{
    app_con_t * con;

    LL_READ(app_con_ll, con) {
        if((con->sender == sender || sender == NULL) &&
           (con->receiver == receiver || receiver == NULL)) {
            RTE_LL_Remove(&app_con_ll, con);
            Memory_Free(BANK_RTE, con);
        }
    }
}
/**
 * Send data to other applications
 * @param app_send pointer to the application which is sending the message
 * @param type type of data from 'lv_app_com_type_t' enum
 * @param data pointer to the sent data
 * @param size length of 'data' in bytes
 * @return number application which were received the message
 */
uint16_t APP_Con_Send(app_inst_t * app_send, app_com_type_t type , const void * data, uint32_t size)
{
    if(type == APP_COM_TYPE_INV) return 0;
    app_con_t * con;
    uint16_t rec_cnt = 0;
    LL_READ(app_con_ll, con) {
        if(con->sender == app_send) {
            if(con->receiver->dsc->com_rec != NULL)
            con->receiver->dsc->com_rec(app_send, con->receiver, type, data, size);
            rec_cnt ++;
        }
    }
    return rec_cnt;
}
/**
 * Rename an application
 * @param app pointer to an application
 * @param name a string with the new name
 */
void APP_Rename(app_inst_t * app, const char * name)
{
	Memory_Free(BANK_RTE,app->name);
	app->name = Memory_Alloc0(BANK_RTE,strlen(name) + 1);
	strcpy(app->name, name);

	if(app->sc_title != NULL) {
		lv_label_set_text(app->sc_title, app->name);
	}
}
/**
 * Open a shortcut for an application
 * @param app pointer to an application
 * @return pointer to the shortcut
 */
lv_obj_t * APP_GUI_Init(app_inst_t * app)
{
    if(app->dsc->sc_open == NULL) 
		return NULL;
	//一个APP的主界面的根控件是btn
	app->sc = lv_btn_create(app_handle.sc_page, NULL);
    lv_page_glue_obj(app->sc, true);
	//把根控件指向该APP
	app->sc->free_ptr = app;
	
	//设置根控件风格
	lv_btn_set_style(app->sc,LV_BTN_STYLE_REL, &app_handle.style.sc_rel);
	lv_btn_set_style(app->sc,LV_BTN_STYLE_PR, &app_handle.style.sc_pr);
	lv_obj_set_size(app->sc, APP_SC_WIDTH, APP_SC_HEIGHT);
	//关闭根控件上布局
	lv_cont_set_layout(app->sc, LV_LAYOUT_OFF);
	//设置根控件动作
	lv_btn_set_action(app->sc,LV_BTN_ACTION_CLICK, app_sc_rel_action);
	//为APP设置ICON	
	if(app->dsc->img)
	{
		app->icon = lv_img_create(app->sc,NULL);
		lv_img_set_src(app->icon,app->dsc->img);
		lv_obj_align(app->icon,app->sc,LV_ALIGN_IN_TOP_MID,0,LV_DPI / 20);
	}
	if((app->dsc->mode & APP_MODE_NO_SC_TITLE) == 0) {
        /*Create a title on top of the shortcut*/
        app->sc_title = lv_label_create(app->sc, NULL);
        lv_obj_set_style(app->sc_title, &app_handle.style.sc_title);
        lv_obj_set_size(app->sc_title, APP_SC_WIDTH, lv_font_get_height(app_handle.style.sc_title.text.font)+10);
        lv_label_set_long_mode(app->sc_title, LV_LABEL_LONG_ROLL);
        lv_label_set_text(app->sc_title, app->name);
		if(app->icon)
			lv_obj_align(app->sc_title, app->icon, LV_ALIGN_OUT_BOTTOM_MID, 0, LV_DPI / 20);
		else
			lv_obj_align(app->sc_title, NULL, LV_ALIGN_IN_TOP_MID, 0, LV_DPI / 20);
        lv_obj_set_protect(app->sc_title, LV_PROTECT_POS);
	} else {
	    app->sc_title = NULL;
    }
	/*Allocate data and call the app specific sc_open function*/
	app->sc_data = Memory_Alloc0(BANK_RTE,app->dsc->sc_data_size);
	//调用APP自身的GUI绘制
	app->dsc->sc_open(app, app->sc); 
    lv_page_focus(app_handle.sc_page, app->sc, 200);
	return app->sc;
}
/**
 * Close the shortcut of an application
 * @param app pointer to an application
 */
void APP_GUI_DeInit(app_inst_t * app)
{
	if(app->sc == NULL) 
		return;
    if(app->dsc->sc_close != NULL) 
		app->dsc->sc_close(app);
	lv_obj_del(app->sc);
	app->sc = NULL;
	app->icon = NULL;
	app->sc_title = NULL;
    memset(app->sc_data, 0, app->dsc->sc_data_size);
	Memory_Free(BANK_RTE,app->sc_data);
	app->sc_data = NULL;
}
/**
 * Open the application in a window
 * @param app pointer to an application
 * @return pointer to the shortcut
 */
lv_obj_t * APP_Win_Open(app_inst_t * app)
{
	/*Close the app list if opened*/
	if(app_handle.list != NULL) {
		lv_obj_animate(app_handle.list, LV_ANIM_FLOAT_LEFT | LV_ANIM_OUT, 200, 50, app_menu_rel_animate_callback);
	}

	if(app->dsc->win_open == NULL) return NULL;
	app->win = lv_win_create(lv_scr_act(), NULL);
	app->win->free_ptr = app;
	lv_win_ext_t * ext = (lv_win_ext_t*)lv_obj_get_ext_attr(app->win);
	lv_obj_set_style(ext->header, &app_handle.style.win_header);
	lv_win_set_title(app->win, app->dsc->name);
	lv_page_set_sb_mode(lv_win_get_content(app->win), LV_SB_MODE_AUTO);
	lv_win_set_style(app->win,LV_WIN_STYLE_BTN_REL, &app_handle.style.win_cbtn_rel);
	lv_win_set_style(app->win,LV_WIN_STYLE_BTN_PR , &app_handle.style.win_cbtn_pr);
	lv_obj_set_opa_scale_enable(app->win,true);
	lv_obj_set_opa_scale(app->win,0xF0);
	if(app->dsc->conf_open != NULL) {
	    lv_win_add_btn(app->win, SYMBOL_SETTINGS, app_win_conf_action);
	}
	lv_win_add_btn(app->win, SYMBOL_DOWN, app_win_minim_action);
	lv_win_add_btn(app->win, SYMBOL_CLOSE,app_win_close_action);
    app->win_data = Memory_Alloc0(BANK_RTE,app->dsc->win_data_size);
    app->dsc->win_open(app, app->win);
	return app->win;
}
/**
 * Close the window of an application
 * @param app pointer to an application
 */
void APP_Win_Close(app_inst_t * app)
{
	if(app->win == NULL) return;
	
    if(app->dsc->win_close != NULL) 
		app->dsc->win_close(app);
	
	lv_obj_del(app->win);
	app->win = NULL;

	memset(app->win_data, 0, app->dsc->win_data_size);
	Memory_Free(BANK_RTE, app->win_data);
	app->win_data = NULL;
}
/**
 * 新建一个APP描述句柄 并不会为其建立图标
 * @param app_dsc pointer to an application descriptor
 * @param conf pointer to an application specific configuration structure or NULL if unused
 * @return pointer to the opened application or NULL if any error occurred
 */
app_inst_t * APP_New(app_dsc_t * app_dsc, void * conf)
{
	/*Add a new application and initialize it*/
	app_inst_t * app;
	app = RTE_LL_InsHead(&app_inst_ll);
	app->conf = conf;
	app->dsc = app_dsc;
	app->app_data = Memory_Alloc0(BANK_RTE,app_dsc->app_data_size);
	app->name = NULL;
	app->icon = NULL;
	app->sc = NULL;
	app->sc_data = NULL;
	app->sc_title = NULL;
	app->win = NULL;
	app->win_data = NULL;
	APP_Rename(app, app_dsc->name); /*Set a default name*/
	app_dsc->inst = app;
	return app;
}
/**
 * 删除一个APP描述句柄 并为其删除主界面图标
 * @param app pointer to an application
 */
void APP_Remove(app_inst_t * app)
{
	//关闭当前APP窗口
	APP_Win_Close(app);
	//删除APP在主界面上的图标
	APP_GUI_DeInit(app);
	/*Clear the connection list*/
	APP_Con_Del(app, NULL);
    APP_Con_Del(NULL, app);
	if(app->dsc->app_close != NULL) 
		app->dsc->app_close(app);
    memset(app->app_data, 0, app->dsc->app_data_size);
	Memory_Free(BANK_RTE, app->app_data);
	Memory_Free(BANK_RTE, app->name);
	RTE_LL_Remove(&app_inst_ll, app);
	Memory_Free(BANK_RTE, app);
}
/**
 * Get the application descriptor from its name
 * @param name name of the app. dsc.
 * @return pointer to the app. dsc.
 */
app_dsc_t * APP_Dsc_Get(const char * name)
{
    app_dsc_t ** dsc;
    LL_READ(app_dsc_ll, dsc) {
        if(strcmp((*dsc)->name, name) == 0) {
            return *dsc;
        }
    }
    return NULL;
}
static void app_menu_rel_animate_callback(lv_obj_t *list)
{
	if(app_handle.list != NULL) {
		lv_label_set_text(app_handle.labelbtn, app_handle.name);
		lv_obj_del(app_handle.list);
		app_handle.list = NULL;
	}
}
/**
 * Called when the "Apps" button is released to open or close the app. list
 * @param app_btn pointer to the "Apps" button
 * @param dispi pointer to the caller display input
 * @return LV_RES_OK because the "Apps" button is never deleted
 */
static lv_res_t app_menu_rel_action(lv_obj_t * app_btn)
{
	/*Close the list if opened*/
	if(app_handle.list != NULL) {
		lv_obj_animate(app_handle.list, LV_ANIM_FLOAT_LEFT | LV_ANIM_OUT, 200, 50, app_menu_rel_animate_callback);
	}
	/*Create the app. list*/
	else {
		lv_label_set_text(app_handle.labelbtn, "Tasks");
		app_handle.list = lv_list_create(app_handle.scr, NULL);
		lv_obj_t * scrl = lv_page_get_scrl(app_handle.list);
		lv_obj_set_style(scrl, &app_handle.style.menu);
		lv_list_set_sb_mode(app_handle.list,LV_SB_MODE_AUTO);
		app_dsc_t ** dsc;
		lv_obj_t * elem;
		uint8_t i = 0;
		uint16_t high = 0;
		RTE_LL_READ_BACK(app_dsc_ll, dsc) {
		    if((((*dsc)->mode & APP_MODE_NOT_LIST) == 0)&&((*dsc)->background)) {
				i++;
                elem = lv_list_add(app_handle.list, NULL, (*dsc)->name, app_menu_elem_rel_action);
				elem->free_ptr = (*dsc)->inst;
				if(i==1)
					high = lv_obj_get_height(elem);
		    }
		}
		if(i == 0)
		{
			i++;
			elem = lv_list_add(app_handle.list, NULL, "NULL", app_menu_elem_rel_action);
			elem->free_ptr = NULL;
			high = lv_obj_get_height(elem);
		}
		lv_obj_set_size(app_handle.list, LV_HOR_RES / 2,i * high);
		lv_obj_align(app_handle.list, app_handle.menuh, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
		lv_list_set_style(app_handle.list,LV_LIST_STYLE_BTN_REL, &app_handle.style.menu_btn_rel);
		lv_list_set_style(app_handle.list,LV_LIST_STYLE_BTN_PR, &app_handle.style.menu_btn_pr);
		lv_obj_animate(app_handle.list, LV_ANIM_FLOAT_LEFT | LV_ANIM_IN, 200, 50, NULL);
	}
	return LV_RES_OK;
}
/**
 * Called when the shortcut page is released to hide the app list and/or
 * go back from connection mode
 * @param page pointer to the sc page
 * @param dispi pointer to the caller display input
 * @return LV_RES_OK because the sc page is not deleted
 */
static lv_res_t app_sc_page_rel_action(lv_obj_t * page)
{
	/*Close the list if opened*/
	if(app_handle.list != NULL) {
		lv_obj_animate(app_handle.list, LV_ANIM_FLOAT_LEFT | LV_ANIM_OUT, 200, 50, app_menu_rel_animate_callback);
	}

    if(app_handle.con_send != NULL) {
        app_inst_t * i;
        LL_READ(app_inst_ll, i) {
            if(i->sc != NULL)  
			{	
				lv_btn_set_style(i->sc,LV_BTN_STATE_REL, &app_handle.style.sc_rel);
				lv_btn_set_style(i->sc,LV_BTN_STATE_PR, &app_handle.style.sc_pr);
			}
        }
        app_handle.con_send = NULL;
    }

	return LV_RES_OK;
}
/**
 * Called when an element of the app list is released
 * @param app_elem_btn pointer to an element of app list
 * @param dispi pointer to the caller display input
 * @return LV_RES_INV because the list is dleted on release
 */
static lv_res_t app_menu_elem_rel_action(lv_obj_t * app_elem_btn)
{
    /*Close the app list*/
	lv_label_set_text(app_handle.labelbtn, app_handle.name);
    lv_obj_del(app_handle.list);
    app_handle.list = NULL;
	app_inst_t * app = app_elem_btn->free_ptr;
	//从后台恢复 不需要建立WIN
	if(app)
	{
		app_win_open_anim_create(app);
	}
	return LV_RES_INV;
}
/**
 * Called when the window open animation is ready to close the application
 * @param app_win pointer to a window
 */
static void app_win_open_anim_cb(lv_obj_t * app_win)
{
    lv_obj_t * win_page = lv_win_get_content(app_win);
    /*Unhide the the elements*/
    lv_obj_set_hidden(((lv_win_ext_t *)app_win->ext_attr)->title, false);
    lv_obj_set_hidden(lv_page_get_scrl(win_page), false);
    lv_page_set_sb_mode(win_page, LV_SB_MODE_AUTO);
}
/**
 * Create a window open animation
 * @param app pointer to an application
 * @return LV_RES_OK: because the window is not deleted here
 */
static lv_res_t app_win_open_anim_create(app_inst_t * app)
{
	//后台模式无需动画
	if(app->dsc->background)
	{
		lv_obj_set_hidden(app->win,false);
		app->dsc->background = false;
	}
	else
	{
		/*Temporally set a simpler style for the window during the animation*/
		lv_obj_t * win_page = lv_win_get_content(app->win);
		lv_page_set_sb_mode(win_page, LV_SB_MODE_OFF);

		/*Hide some elements to speed up the animation*/
		lv_obj_set_hidden(((lv_win_ext_t *)app->win->ext_attr)->title, true);
		lv_obj_set_hidden(lv_page_get_scrl(win_page), true);
		lv_obj_animate(app->win, LV_ANIM_FLOAT_TOP | LV_ANIM_IN, 200, 50, app_win_open_anim_cb);
	}
    return LV_RES_OK;
}
/**
 * Called when a shortcut is released to open its window (or close app list if opened) (in normal mode) or
 * add/remove it to/form a connection (in connection mode)
 * @param sc pointer to the releases shortcut object
 * @param dispi pointer to the caller display input
 * @return LV_RES_OK because the sc page is not deleted
 */
static lv_res_t app_sc_rel_action(lv_obj_t * sc)
{
	lv_page_focus(app_handle.sc_page, sc, true);
	/*Close the list if opened*/
	if(app_handle.list != NULL) {
		lv_obj_animate(app_handle.list, LV_ANIM_FLOAT_LEFT | LV_ANIM_OUT, 200, 50, app_menu_rel_animate_callback);
	}
	/*Else open the window of the shortcut*/
	else {
		app_inst_t * app = sc->free_ptr;
		//非后台模式需要先建立GUI界面
		if(app->dsc->background == false)
		{
			if((app->dsc->app_run != NULL)) 
				app->dsc->app_run(app, app->conf);
			APP_Win_Open(app);
		}
		app_win_open_anim_create(app);
	}
	return LV_RES_OK;
}
/**
 * 一个实例APP的关闭动画回调 窗口关闭
 * @param app_win pointer to a window
 */
static void app_win_close_anim_cb(lv_obj_t * app_win)
{
    app_inst_t * app = app_win->free_ptr;
	APP_Win_Close(app);
}
/**
 * 一个实例APP的最小化按钮回调
 * @param minim_btn pointer to the minim. button
 * @param dispi pointer to the caller display input
 * @return LV_RES_OK or LV_ACTION_RES_INC depending on APP_EFFECT_... settings type
 */
static lv_res_t app_win_minim_action(lv_obj_t * minim_btn)
{
	lv_obj_t * win = lv_win_get_from_btn(minim_btn);
	app_inst_t * app = win->free_ptr;
	app->dsc->background = true;
	lv_obj_set_hidden(win,true);
	return LV_RES_OK;
}
/**
 * 一个实例APP的关闭按钮回调 调用动画与动画回调
 * @param close_btn pointer to the close button
 * @param dispi pointer to the caller display input
 * @return LV_RES_OK or LV_ACTION_RES_INC depending on APP_EFFECT_... settings type
 */
static lv_res_t app_win_close_action(lv_obj_t * close_btn)
{
	lv_obj_t * win = lv_win_get_from_btn(close_btn);
	app_inst_t * app = win->free_ptr;
	/*Make an animation on window minimization*/
	lv_obj_animate(app->win, LV_ANIM_FLOAT_TOP | LV_ANIM_OUT, 200, 50, app_win_close_anim_cb);
	/*The animation will close the window*/
    return LV_RES_OK;
}
/**
 * Open the settings of an application in a window (use the set_open function of the application)
 * @param set_btn pointer to the settings button
 * @param dispi pointer to the caller display input
 * @return always LV_RES_OK because the button is not deleted here
 */
static lv_res_t app_win_conf_action(lv_obj_t * set_btn)
{
    /*Close the app list if opened*/
    if(app_handle.list != NULL) {
		lv_obj_animate(app_handle.list, LV_ANIM_FLOAT_LEFT | LV_ANIM_OUT, 200, 50, app_menu_rel_animate_callback);
    }

    lv_obj_t * win = lv_win_get_from_btn(set_btn);
    app_inst_t * app = win->free_ptr;

    app->conf_win = lv_win_create(lv_scr_act(), NULL);
    app->conf_win->free_ptr = app;
    char buf[256];
    usprintf(buf, "%s settings", app->dsc->name);
    lv_win_add_btn(app->conf_win, SYMBOL_CLOSE ,lv_win_close_action);
	lv_win_ext_t *ext = (lv_win_ext_t *)lv_obj_get_ext_attr(app->conf_win);
    lv_obj_set_style(ext->header, &app_handle.style.win_header);
    lv_win_set_title(app->conf_win, buf);
    lv_win_set_style(app->conf_win,LV_WIN_STYLE_BTN_REL, &app_handle.style.win_cbtn_rel);
	lv_win_set_style(app->conf_win,LV_WIN_STYLE_BTN_PR, &app_handle.style.win_cbtn_pr);
    lv_obj_t * scrl = lv_page_get_scrl(lv_win_get_content(app->conf_win));
    lv_cont_set_layout(scrl, LV_LAYOUT_COL_L);
    app->dsc->conf_open(app, app->conf_win);
    return LV_RES_OK;
}
//APP内GUI组定时器回调函数
static void app_statusbar_refreshtime_timercallback(void *arg)
{
	lv_obj_t *label = (lv_obj_t *)arg;
	char buf[32]={0};
	if(label == app_handle.labelinfor[0])
	{
		RTC_TimeTypeDef RTC_TimeStructure;
		RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
		usprintf(buf,"%02d:%02d:%02d",RTC_TimeStructure.RTC_Hours,RTC_TimeStructure.RTC_Minutes,RTC_TimeStructure.RTC_Seconds);
	}
	else
	{
		RTC_DateTypeDef RTC_DateStructure;
		RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);
		usprintf(buf,"20%02d/%02d/%02d",RTC_DateStructure.RTC_Year,RTC_DateStructure.RTC_Month,RTC_DateStructure.RTC_Date);
	}
	lv_label_set_text(label,buf);
}
static void app_statusbar_refreshcpu_timercallback(void *arg)
{
	lv_obj_t *label = (lv_obj_t *)arg;
	if(label)
	{
		extern volatile uint8_t   StaticsCPUUsage;
		char buf[16]={0};
		usprintf(buf,"CPU:%d%%",StaticsCPUUsage);
		lv_label_set_text(label,buf);
	}
}
//APP消息接收函数
static void overall_com_rec(app_inst_t * app_send, app_inst_t * app_rec,
                       app_com_type_t type , const void * data, uint32_t size)
{
	app_dsc_t *app = APP_Dsc_Get("WIFI");
	if(app_send == app->inst)
	{
		bool ConnectFlag = *((bool *)(data));
		if(ConnectFlag)
			lv_img_set_src(app_handle.iconinfor[1],SYMBOL_WIFI);
		else
			lv_img_set_src(app_handle.iconinfor[1],SYMBOL_CLOSE);
	}
}
