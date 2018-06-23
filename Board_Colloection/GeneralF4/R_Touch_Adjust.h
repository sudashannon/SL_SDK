#ifndef __R_Touch_Adjust_H
#define __R_Touch_Adjust_H	 

#include "common.h" 
 

extern float xfac;		 //触摸屏校准参数(电容屏不需要校准) 
extern float yfac;
extern short xoff;
extern short yoff;	 

/*********电阻屏校准用到的函数*********************/
void TP_Save_Adjdata(void);				//保存校准参数
void TP_Adjust(void);							//触摸屏校准
u8 TP_Get_Adjdata(void);					//读取校准参数
void TP_Adjdata_Show(u16 x0,u16 y0,u16 x1,u16 y1,u16 x2,u16 y2,u16 x3,u16 y3,u16 fac);//显示校准信息


#endif
