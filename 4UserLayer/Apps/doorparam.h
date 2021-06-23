/******************************************************************************

                  Copyright @1998 - 2021 BSG

 ******************************************************************************
文 件 名   : doorparam.h
 					
@author  
@date 2021年2月26日
  最近修改   :
@file doorparam.h
@brief 门点参数，以及各种数据结构 
  函数列表   :
修改历史   :
@date 2021年2月26日
@author  
修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "stdint.h"
/*----------------------------------------------*
 * 宏定义                                 *
 *----------------------------------------------*/
 
//typedef void (*initCallBack)(void); //参数初始化
//typedef char(*addCallBack)(uint32_t,char *,int);
//typedef char(*delCallBack)(uint32_t,char *,int);
//typedef char (*modifyCallBack)(uint32_t,char *,int,char *,int);
//typedef char (*readCallBack)(uint32_t,char *,int *);

 /*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 枚举定义                            *
 *----------------------------------------------*/
//开门方式
typedef enum OPEN_TYPE
{
    FIRST_CARD_AUTH= 0, //首卡授权
    FIRST_CARD_NORMAL,  //首卡常开
    MULTI_CARD_SIMPLE,  //简单多卡  
    BRUSH_CARD,         //刷卡
    COERCION_BRUSH_CARD,//胁迫刷卡
    PASSWORD,           //密码开门
    COERCION_PASSWORD,  //胁迫密码
    QR_CODE,            //二维码开门
    CARD_PASSWORD,      //卡+密码开门
    MUTEX               //互斥开门
}OPEN_TYPE_ENUM;

//门禁类型
typedef enum DEV_TYPE
{
    ONE_DOOR = 1, //一门门禁
    TWO_DOORS     //二门门禁          
}DEV_TYPE_ENUM;

//工作模式
typedef enum WORK_TYPE
{
    NORMAL = 0,     //正常模式
    ALWAYS_OPEN,    //常开模式
    TIMING_OPEN     //定时开门          
}WORK_TYPE_ENUM;



//是否输出非法警告
typedef enum ILLEGALWARNING
{
    NO = 0,
    YES           
}ILLEGALWARNING_ENUM;

/*----------------------------------------------*
 * 结构体定义                              *
 *----------------------------------------------*/
 //有效期
typedef struct VALID_TIME
{
    int beginTime;
    int endTime;
}VALID_TIME_STRU;

//首卡授权
typedef struct FIRST_CARD_AUTH
{
    uint8_t isBrushCard;    //已刷首卡标志
    VALID_TIME_STRU validTime;   //刷首卡有效期          
}FIRST_CARD_AUTH_STRU;

//多卡开门
typedef struct MULIT_CARD_AUTH
{
    uint8_t isMulitCard;    //是否是多卡开门
    uint8_t mulitCardNumber;//多卡开门人数
    VALID_TIME_STRU validTime;   //多卡开门有效期
}MULIT_CARD_AUTH_STRU;

//胁迫密码开门
typedef struct COERCION_PW_AUTH
{
    uint8_t isEnable;   //是否启用胁迫密码开门
    uint8_t passWord[6];//胁迫密码
}COERCION_PW_AUTH_STRU;

//胁迫卡开门
typedef struct COERCION_CARD_AUTH
{
    uint8_t isEnable;   //是否启用胁迫密码开门
    uint8_t card[4];    //胁迫卡号
}COERCION_CARD_AUTH_STRU;

//密码开门
typedef struct PASS_WORD_AUTH
{
    uint8_t isEnable;//是否开启密码开门
    uint8_t passWord[6];//普通密码
}PASS_WORD_AUTH_STRU;

//二维码开门
typedef struct QR_CODE_AUTH
{
    uint8_t isEnable;
}QR_CODE_AUTH_STRU;

//卡号+密码开门
typedef struct CARD_PW_AUTH
{
    uint8_t isEnable;
    uint8_t password[6];
    uint8_t card[4];
}CARD_PW_AUTH_STRU;

//定时常开
typedef struct TIMING_AUTH
{
    uint8_t isEnable;
    VALID_TIME_STRU validTime[3];
}TIMING_AUTH_STRU;

//互锁开门
typedef struct MUTEX_AUTH
{
    uint8_t isEnable;
    uint8_t doorA;
    uint8_t doorB;
}MUTEX_AUTH_STRU;
 
typedef struct DEV_PARAM
{
    DEV_TYPE_ENUM devType;  //设备型号： 1门门禁/2门门禁
    OPEN_TYPE_ENUM openType; //开门方式
    WORK_TYPE_ENUM workType; //应用场景
    FIRST_CARD_AUTH_STRU firstCardAuth;  //首卡模式
    MULIT_CARD_AUTH_STRU mulitCardAuth; //多卡模式
    COERCION_PW_AUTH_STRU coercionPwAuth; //胁迫密码开门模式
    COERCION_CARD_AUTH_STRU coercionCardAuth;//胁迫卡开门模式
    PASS_WORD_AUTH_STRU passwordAuth;    //密码开门
    QR_CODE_AUTH_STRU   qrCodeAuth;  //二维码开门
    CARD_PW_AUTH_STRU   cardAndPwAuth; //卡+密码开门
    TIMING_AUTH_STRU    timingAuth;    //定时常开模式   
    MUTEX_AUTH_STRU     mutexAuth;      //互斥开门
}DEV_PARAM_STRU;



/*----------------------------------------------*
 * 外部函数原型说明                                     *
 *----------------------------------------------*/
extern DEV_PARAM_STRU gDevParam;


/*----------------------------------------------*
 * 常量定义                                       *
 *----------------------------------------------*/



