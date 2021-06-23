/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : doorparam.c
  版 本 号   : 初稿
  作    者   :  
  生成日期   : 2021年3月1日
  最近修改   :
  功能描述   : 门点参数，以及各种数据结构
  函数列表   :
  修改历史   :
  1.日    期   : 2021年3月1日
    作    者   :  
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "doorparam.h"
/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

 
/*----------------------------------------------*
 * 内部函数原型说明                                   *
 *----------------------------------------------*/
//static void initParam(void);
//static char addParam(uint32_t addr,char *buff,int len);
//static char delParam(uint32_t addr,char *buff,int len);
//static char readParam(uint32_t addr,char *buff,int *len);
//static char modifyParam(uint32_t addr,char *sourceBuff,int sourceLen,char *descBuff,int descLen);

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/
DEV_PARAM_STRU gDevParam = 
{
    ONE_DOOR, //设备型号
    BRUSH_CARD, //开门方式
    NORMAL,     //应用场景
    {0,{0,0}},  //首卡模式
    {0,0,{0,0}},//多卡模式
    {0,"888888"},//胁迫密码开门模式
    {0,"1234"},  //胁迫卡开门模式
    {0,"123456"},   //密码开门
    0,              //二维码开门
    {0,"123456","1234"},//卡+密码开门
    {0,{{1,2},{3,4},{5,6}}},
    {0,1,1} 
};

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
