/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : bsp.c
  版 本 号   : 初稿
  作    者   : 张舵
  生成日期   : 2019年7月9日
  最近修改   :
  功能描述   : 这是硬件底层驱动程序的主文件。每个可以 #include "bsp.h" 来包含所有的外设驱动模块。
            bsp = Borad surport packet 板级支持包
  函数列表   :
  修改历史   :
  1.日    期   : 2019年7月9日
    作    者   : 张舵
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "bsp.h"
/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define HARDWARE_VERSION               "V1.0.1"
#define SOFTWARE_VERSION               "V1.0.1"

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

static void my_mem_init(void)
{
	mymem_init(SRAMIN);								//初始化内部内存池
	mymem_init(SRAMEX);								//初始化外部内存池
	mymem_init(SRAMCCM);	  					    //初始化CCM内存池
}

 static void DisplayDevInfo(void);
 
 
 
 static void DisplayDevInfo(void)
 {
     printf("\r\n==========Version==========\r\n");
     printf("Softversion :%s\r\n",gDevinfo.SoftwareVersion);
     printf("HardwareVersion :%s\r\n", gDevinfo.HardwareVersion);
     printf("Model :%s\r\n", gDevinfo.Model);
     printf("ProductBatch :%s\r\n", gDevinfo.ProductBatch);      
     printf("BulidDate :%s\r\n", gDevinfo.BulidDate);
     printf("DevSn :%s\r\n", gDevinfo.GetSn());
     printf("Devip :%s\r\n", gDevinfo.GetIP());
     printf("DevID :%s\r\n", gDevBaseParam.deviceCode.qrSn);

     printf("bit 0 = %d,bit 1 = %d,bit 2 = %d,bit 3 = %d\r\n",DIP0,DIP1,DIP2,DIP3);
 }

 void bsp_Init(void)
{
    NVIC_SetVectorTable(NVIC_VectTab_FLASH,0x30000);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//设置系统中断优先级分组4
	delay_init(168);            //初始化延时函数

    bsp_dipswitch_init();
    
	bsp_TIM6_Init();            //定时器6初始化
	
    bsp_InitUart();

    bsp_ds1302_init();            //时钟芯片初始化    
    
	bsp_LED_Init();		        //初始化LED端口	  
	
    bsp_MRAM_Init();           //MR25
    
    STM_FLASH_Init();           //芯片内部FLASH初始化
    
    easyflash_init();           //外部FLASH初始化，使用easyflash       
    
    bsp_beep_init();            //蜂鸣器初始化

    bsp_WiegandInit();          //韦根读卡器初始化

    bsp_sw_init();


    my_mem_init();                  //对内存进行初始化  
    
    initDevParam();
    
    DisplayDevInfo();
  /* CmBacktrace initialize */
//   cm_backtrace_init("SingleDoorApp", HARDWARE_VERSION, SOFTWARE_VERSION);

}

