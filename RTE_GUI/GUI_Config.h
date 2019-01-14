#ifndef __LV_CONFIG_H
#define __LV_CONFIG_H
//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------
//<s>  GUI Version
//<i> LITTLEV GUI 原始文件版本号
#define LV_VERSION                 "5.2.1"
// <h> Resolution settings
 //<i> GUI分辨率设置
	 //<o> LV_HOR_RES
	 //<i> GUI水平像素大小
	#define LV_HOR_RES          (320)
	 //<o> LV_VER_RES
	 //<i> GUI垂直像素大小
	#define LV_VER_RES          (240)
	 //<o> LV_DPI
	 //<i> GUIDPI大小
	#define LV_DPI              100
// </h>

// <h> Display settings
// <i> GUI显示设置
	// <o> LV_VDB_SIZE
	// <i> 默认大小为 ~1/10 screen，用于缓冲绘图，抗锯齿以及不透明度
	#define LV_VDB_SIZE         (LV_VER_RES / 10 * LV_HOR_RES) 	
	// <q> LV_VDB_DOUBLE
	// <i> GUI 双缓冲绘图，需要DMA配合
	#define LV_VDB_DOUBLE       0       /*1: Enable the use of 2 VDBs*/
	// GUI 每像素点色彩深度
	#define LV_VDB_PX_BPP       LV_COLOR_SIZE  
	// <q> LV_ANTIALIAS
	// <i> GUI 抗锯齿使能
	#define LV_ANTIALIAS        1       /*1: Enable anti-aliasing*/
	// <o> LV_INV_FIFO_SIZE
	// <i> GUI 屏幕默认最大object数目
	#define LV_INV_FIFO_SIZE          32        /*The average count of objects on a screen */
// </h>

// <h> Input device settings
// <i> GUI输入设备设置
	// <o> LV_INDEV_READ_PERIOD
	// <i> GUI 输入设备读取间隔时间（单位：ms）
	#define LV_INDEV_READ_PERIOD            50    	/*Input device read period in milliseconds*/
  #define LV_INDEV_POINT_MARKER           0                      /*Mark the pressed points  (required: USE_LV_REAL_DRAW = 1)*/
	// <o> LV_INDEV_DRAG_LIMIT
	// <i> GUI 拖动阈值
	#define LV_INDEV_DRAG_LIMIT             10                     /*Drag threshold in pixels */
	#define LV_INDEV_DRAG_THROW             20                     /*Drag throw slow-down in [%]. Greater value means faster slow-down */
	// <o> LV_INDEV_LONG_PRESS_TIME
	// <i> GUI 输入设备长按时间（单位：ms）
	#define LV_INDEV_LONG_PRESS_TIME        400                    /*Long press time in milliseconds*/
	// <o> LV_INDEV_LONG_PRESS_REP_TIME
	// <i> GUI 输入设备长按重发时间（单位：ms）
	#define LV_INDEV_LONG_PRESS_REP_TIME    100                    /*Repeated trigger period in long press [ms] */
// </h>

// <h> Color settings
// <i> GUI色彩设置
	// <o> LV_COLOR_DEPTH
	// <i> GUI使用得色彩位数大小
	// <i> 默认大小: 16（单位：bit）
	#define LV_COLOR_DEPTH     16                     /*Color depth: 1/8/16/32*/
	// <q> LV_COLOR_16_SWAP
	// <i> 是否交换RGB565的两个高低位 八位接口LCD时需要注意设置
	#define LV_COLOR_16_SWAP   0
	// <q> LV_COLOR_SCREEN_TRANSP
	// <i> 是否使能屏幕透明效果 需要32位色彩支持
	#define LV_COLOR_SCREEN_TRANSP        0           /*1: Enable screen transparency. Useful for OSD or other overlapping GUIs. Requires ARGB8888 colors*/
	#define LV_COLOR_TRANSP    LV_COLOR_LIME          /*Images pixels with this color will not be drawn (with chroma keying)*/
// </h>

// <h> Text settings
// <i> GUI 文本设置
	// <q> LV_TXT_UTF8
	// <i> GUI 使用UTF8编码
	#define LV_TXT_UTF8             1                /*Enable UTF-8 coded Unicode character usage */
	//<s>  LV_TXT_BREAK_CHARS
	//<i> GUI TXT使用特定字符串
	#define LV_TXT_BREAK_CHARS     " ,.;:-_"         /*Can break texts on these chars*/
// </h>

// <h> GUI feature usage
// <i> GUI 特征配置
// <q> USE_LV_SHADOW
	// <i> GUI 阴影模块
	#define USE_LV_SHADOW           1               /*1: Enable shadows*/
	// <e> USE_LV_ANIMATION
	// <i> GUI 动画模块
	#define USE_LV_ANIMATION        1            /*1: Enable all animations*/
		// <o> LV_AINIM_TIME_PERIOD
		// <i> GUI 动画时钟基准
		// <i> 默认大小: 10（单位：MS）
		#define LV_AINIM_TIME_PERIOD          10   
	// </e>
	// <q> USE_LV_GROUP
	// <i> GUI Group模块
	#define USE_LV_GROUP            1               /*1: Enable object groups (for keyboards)*/
	// <q> USE_LV_GPU
	// <i> GUI 使用图形加速硬件
	#define USE_LV_GPU              0              /*1: Enable GPU interface*/
	// <q> USE_LV_REAL_DRAW
	// <i> GUI 直接绘制
	#define USE_LV_REAL_DRAW        1               /*1: Enable function which draw directly to the frame buffer instead of VDB (required if LV_VDB_SIZE = 0)*/
	// <q> USE_LV_FILESYSTEM
	// <i> GUI 文件系统模块
	#define USE_LV_FILESYSTEM       1               /*1: Enable file system (required by images*/
// </h>

// <h> Compiler settings
// <i> 编译器配置
	// <q> LV_COMPILER_VLA_SUPPORTED
	// <i> 编译器支持变长数组
	#define LV_COMPILER_VLA_SUPPORTED            1  /* 1: Variable length array is supported*/
	// <q> LV_COMPILER_NON_CONST_INIT_SUPPORTED
	// <i> 编译器支持const不初始化
	#define LV_COMPILER_NON_CONST_INIT_SUPPORTED 1  /* 1: Initialization with non constant values are supported */
// </h>

/*Log settings*/
#define USE_LV_LOG      0   /*Enable/disable the log module*/
#if USE_LV_LOG
/* How important log should be added:
 * LV_LOG_LEVEL_TRACE       A lot of logs to give detailed information
 * LV_LOG_LEVEL_INFO        Log important events
 * LV_LOG_LEVEL_WARN        Log if something unwanted happened but didn't caused problem
 * LV_LOG_LEVEL_ERROR       Only critical issue, when the system may fail
 */
#define LV_LOG_LEVEL    LV_LOG_LEVEL_INFO
/* 1: Print the log with 'printf'; 0: user need to register a callback*/

#define LV_LOG_PRINTF   0
#endif  /*USE_LV_LOG*/


// <h> Theme settings
// <i> 主题配置
	// <q> LV_THEME_LIVE_UPDATE
	// <i> 支持运行中更换主题 需要额外的8-10 kB RAM
	#define LV_THEME_LIVE_UPDATE    0       /*1: Allow theme switching at run time. Uses 8..10 kB of RAM*/
	// <q> USE_LV_THEME_TEMPL
	#define USE_LV_THEME_TEMPL      0       /*Just for test*/
	// <q> USE_LV_THEME_DEFAULT
	#define USE_LV_THEME_DEFAULT    0       /*Built mainly from the built-in styles. Consumes very few RAM*/
	// <q> USE_LV_THEME_ALIEN
	#define USE_LV_THEME_ALIEN      1       /*Dark futuristic theme*/
	// <q> USE_LV_THEME_NIGHT
	#define USE_LV_THEME_NIGHT      0       /*Dark elegant theme*/
	// <q> USE_LV_THEME_MONO
	#define USE_LV_THEME_MONO       0       /*Mono color theme for monochrome displays*/
	// <q> USE_LV_THEME_MATERIAL
	#define USE_LV_THEME_MATERIAL   0       /*Flat theme with bold colors and light shadows*/
	// <q> USE_LV_THEME_ZEN
	#define USE_LV_THEME_ZEN        0       /*Peaceful, mainly light theme */
	// <q> USE_LV_THEME_NEMO
	#define USE_LV_THEME_NEMO       0       /*Water-like theme based on the movie "Finding Nemo"*/
// </h>

// <h> Font Settings
// <i> GUI 字体配置
#define LV_FONT_DEFAULT        &lv_font_dejavu_20    /*Always set a default font from the built-in fonts*/
/* More info about fonts: https://littlevgl.com/basics#fonts
 * To enable a built-in font use 1,2,4 or 8 values
 * which will determine the bit-per-pixel */
#define USE_LV_FONT_DEJAVU_10              4
#define USE_LV_FONT_DEJAVU_10_LATIN_SUP    0
#define USE_LV_FONT_DEJAVU_10_CYRILLIC     0
#define USE_LV_FONT_SYMBOL_10              4

#define USE_LV_FONT_DEJAVU_20              4
#define USE_LV_FONT_DEJAVU_20_LATIN_SUP    0
#define USE_LV_FONT_DEJAVU_20_CYRILLIC     0
#define USE_LV_FONT_SYMBOL_20              4

#define USE_LV_FONT_DEJAVU_30              0
#define USE_LV_FONT_DEJAVU_30_LATIN_SUP    0
#define USE_LV_FONT_DEJAVU_30_CYRILLIC     0
#define USE_LV_FONT_SYMBOL_30              0

#define USE_LV_FONT_DEJAVU_40              0
#define USE_LV_FONT_DEJAVU_40_LATIN_SUP    0
#define USE_LV_FONT_DEJAVU_40_CYRILLIC     0
#define USE_LV_FONT_SYMBOL_40              0

#define USE_LV_FONT_MONOSPACE_8            0
// </h>


/*===================
 *  LV_OBJ SETTINGS
 *==================*/
#define LV_OBJ_FREE_NUM_TYPE    uint32_t    /*Type of free number attribute (comment out disable free number)*/
#define LV_OBJ_FREE_PTR         1           /*Enable the free pointer attribute*/
#endif

// <h> Objectx settings
// <i> GUI 控件设置
	// <e> USE_LV_LABEL
	// <i> GUI 标签控件 依赖控件：无
	#define USE_LV_LABEL    1
	#if USE_LV_LABEL != 0
	// <o> LV_LABEL_SCROLL_SPEED
	// <i> GUI 标签控件滚动时间（单位：ms）
	#define LV_LABEL_SCROLL_SPEED       20     /*Hor, or ver. scroll speed [px/sec] in 'LV_LABEL_LONG_SCROLL/ROLL' mode*/
	#endif
	// </e>
	// <e> USE_LV_IMG
	// <i> GUI 图片控件 依赖控件：label
	#define USE_LV_IMG      1
	// <q> LV_IMG_CF_INDEXED
	#define LV_IMG_CF_INDEXED   1       /*Enable indexed (palette) images*/
	// <q> LV_IMG_CF_ALPHA
	#define LV_IMG_CF_ALPHA     1       /*Enable alpha indexed images*/
	// </e>
	// <e> USE_LV_LINE
	// <i> GUI 直线控件 依赖控件：无
	#define USE_LV_LINE     1
	// </e>
	// <e> USE_LV_ARC
	// <i> GUI arc控件 依赖控件：无
	#define USE_LV_ARC     1
	// </e>
	// <e> USE_LV_CONT
	// <i> GUI 容器控件 依赖控件：无
	#define USE_LV_CONT     1
	// </e>
	// <e> USE_LV_PAGE
	// <i> GUI 页控件 依赖控件：cont
	#define USE_LV_PAGE     1
	// </e>
	// <e> USE_LV_WIN
	// <i> GUI 窗口控件 依赖控件：cont, btn, label, img, page
	#define USE_LV_WIN      1
	// </e>
	// <e> USE_LV_TABVIEW
	// <i> GUI 标签页控件 依赖控件：page, btnm
	#define USE_LV_TABVIEW      1
	#if USE_LV_TABVIEW != 0
	// <o> LV_LABEL_SCROLL_SPEED
	// <i> GUI 标签页控件切换时间（单位：ms）
	#define LV_TABVIEW_ANIM_TIME    200     /*Time of slide animation [ms] (0: no animation)*/
	#endif
	// </e>
	// <e> USE_LV_BAR
	// <i> GUI 进度条控件 依赖控件：无
	#define USE_LV_BAR      1
	// </e>
	// <e> USE_LV_LMETER
	// <i> GUI 进度表控件 依赖控件：无
	#define USE_LV_LMETER   1
	// </e>
	// <e> USE_LV_GAUGE
	// <i> GUI 进度表控件2 依赖控件：bar, lmeter
	#define USE_LV_GAUGE    1
	// </e>
	// <e> USE_LV_CHART
	// <i> GUI 二维图控件 依赖控件：无
	#define USE_LV_CHART    1
	// </e>
	// <e> USE_LV_LED
	// <i> GUI LED控件 依赖控件：无
	#define USE_LV_LED      0
	// </e>
	// <e> USE_LV_MBOX
	// <i> GUI 消息窗口控件 依赖控件：rect, btnm, label
	#define USE_LV_MBOX     1
	// </e>
	// <e> USE_LV_TA
	// <i> GUI 文本框控件 依赖控件：label, page
	#define USE_LV_TA       1
	#if USE_LV_TA != 0
	// <o> LV_TA_CURSOR_BLINK_TIME
	// <i> GUI 文本框控件焦点闪烁间隔（单位：ms）
	#define LV_TA_CURSOR_BLINK_TIME 400     /*ms*/
	// <o> LV_TA_PWD_SHOW_TIME
	// <i> GUI 文本框控件PWD显示时间（单位：ms）
	#define LV_TA_PWD_SHOW_TIME     1500    /*ms*/
	#endif
	// </e>
	// <e> USE_LV_CALENDAR
	// <i> GUI 日历控件 依赖控件：无
	#define USE_LV_CALENDAR    1
	// </e>
	// <e> USE_LV_PRELOAD
	// <i> GUI 预加载控件 依赖控件：arc
	/*Preload (dependencies: arc)*/
	#define USE_LV_PRELOAD      1
	#if USE_LV_PRELOAD != 0
	// <o> LV_PRELOAD_DEF_ARC_LENGTH
	#define LV_PRELOAD_DEF_ARC_LENGTH   60      /*[deg]*/
	// <o> LV_PRELOAD_DEF_SPIN_TIME
	#define LV_PRELOAD_DEF_SPIN_TIME    1000    /*[ms]*/
	#endif
	// </e>
	// <e> USE_LV_BTN
	// <i> GUI 按钮控件 依赖控件：cont
	#define USE_LV_BTN      1
	#if USE_LV_BTN != 0
	// <q> LV_BTN_INK_EFFECT
	// <i> GUI 按钮按下动画
	#define LV_BTN_INK_EFFECT   1       /*Enable button-state animations - draw a circle on click (dependencies: USE_LV_ANIMATION)*/
	// <q> USE_LV_IMGBTN
	// <i> GUI 图片按钮
	#define USE_LV_IMGBTN   1
	#endif
	// </e>
	// <e> USE_LV_BTNM
	// <i> GUI 矩阵按钮控件 依赖控件：无
	#define USE_LV_BTNM     1
	// </e>
	// <e> USE_LV_KB
	// <i> GUI 键盘控件 依赖控件：btnm
	#define USE_LV_KB       1
	// </e>
	// <e> USE_LV_CB
	// <i> GUI 单选框控件 依赖控件：btn, label
	#define USE_LV_CB       1
	// </e>
	// <e> USE_LV_LIST
	// <i> GUI 列表控件 依赖控件：page, btn, label, (img optionally for icons )
	#define USE_LV_LIST     1
	#if USE_LV_LIST != 0
	// <o> LV_LIST_FOCUS_TIME
	// <i> GUI 列表控件焦点动画时间（单位：ms）
	#define LV_LIST_FOCUS_TIME  100 /*Default animation time of focusing to a list element [ms] (0: no animation)  */
	#endif
	// </e>
	// <e> USE_LV_DDLIST
	// <i> GUI 下拉列表控件 依赖控件：page, label
	#define USE_LV_DDLIST    1
	#if USE_LV_DDLIST != 0
	// <o> LV_DDLIST_ANIM_TIME
	// <i> GUI 下拉列表控件动画时间（单位：ms）
	#define LV_DDLIST_ANIM_TIME     200     /*Open and close default animation time [ms] (0: no animation)*/
	#endif
	// </e>
	// <e> USE_LV_ROLLER
	// <i> GUI 滚动列表控件 依赖控件：ddlist
	#define USE_LV_ROLLER    1
	#if USE_LV_ROLLER != 0
	// <o> LV_ROLLER_ANIM_TIME
	// <i> GUI 滚动列表控件动画时间（单位：ms）
	#define LV_ROLLER_ANIM_TIME     200     /*Focus animation time [ms] (0: no animation)*/
	#endif
	// </e>
	// <e> USE_LV_SLIDER
	// <i> GUI 拖动进度条控件 依赖控件：bar
	#define USE_LV_SLIDER    1
	// </e>
	// <e> USE_LV_SW
	// <i> GUI 开关控件 依赖控件：slider
	#define USE_LV_SW       1
	// </e>
// </h>
/****************** (C) COPYRIGHT SuChow University Shannon********************/
