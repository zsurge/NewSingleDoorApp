/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : ini.h
  版 本 号   : 初稿
  作    者   : 张舵
  生成日期   : 2019年5月28日
  最近修改   :
  功能描述   : 对参数进行操作
  函数列表   :
  修改历史   :
  1.日    期   : 2019年5月28日
    作    者   : 张舵
    修改内容   : 创建文件

******************************************************************************/

#ifndef __INI_H
#define __INI_H

#include "easyflash.h"
#include "version.h"
#include "string.h"
#include "deviceInfo.h"
#include "tool.h"
#include "eth_cfg.h"
#include "cJSON.h"
#include "errorcode.h"

#include "malloc.h"
#include "bsp_spi_mram.h"
#include "bsp_ds1302.h"

#define CARD_NO_LEN_ASC     8       //卡号ASC码长度，状态(1)+卡号(3)
#define CARD_NO_LEN_BCD     (CARD_NO_LEN_ASC/2) //卡号BCD码长度
#define HEAD_lEN 4                  //每条记录4字节卡号，
#define DEL_HEAD_lEN 4              //每条记录占4个字节,4字节为已删除卡号索引(M*512+R)
#define MAX_HEAD_RECORD          30720   //最大30720条记录
#define CARD_SECTOR_SIZE         4096    //每个扇区大小

#define MAX_HEAD_DEL_CARDNO     256   //最大可以在删除256张卡


#define CARD_NO_HEAD_SIZE   (HEAD_lEN*MAX_HEAD_RECORD)  //120K
#define CARD_DEL_HEAD_SIZE  (DEL_HEAD_lEN*MAX_HEAD_DEL_CARDNO)   //1K



#define CARD_HEAD_SECTOR_NUM     (CARD_NO_HEAD_SIZE/CARD_SECTOR_SIZE) //30个扇区
#define HEAD_NUM_SECTOR          (CARD_SECTOR_SIZE/HEAD_lEN) //每个扇区存储1024个卡号

//改为存储在铁电
#define CARD_NO_HEAD_ADDR   0x0000                                  //0-120K
#define CARD_DEL_HEAD_ADDR  (CARD_NO_HEAD_ADDR+CARD_NO_HEAD_SIZE)  


//基本参数存储长度及地址
#define DEVICE_BASE_PARAM_SIZE (1024)
#define DEVICE_BASE_PARAM_ADDR (CARD_DEL_HEAD_ADDR+CARD_DEL_HEAD_SIZE)  //121


//表头的索引存储长度及地址
#define RECORD_INDEX_SIZE   (64)
#define RECORD_INDEX_ADDR   (DEVICE_BASE_PARAM_ADDR+DEVICE_BASE_PARAM_SIZE) //122


//参数的存储地址
#define DEVICE_TEMPLATE_PARAM_SIZE   (1984)
#define DEVICE_TEMPLATE_PARAM_ADDR  (RECORD_INDEX_ADDR+RECORD_INDEX_SIZE) //参数存储分配2K空间


//FLASH空间分配
#define ACCESS_RECORD_ADDR       0x500000
#define ACCESS_RECORD_ADDR_END   0x900000
#define DATA_SECTOR_NUM          ((ACCESS_RECORD_ADDR_END-ACCESS_RECORD_ADDR)/CARD_SECTOR_SIZE)

#define CARD_MODE                   1 //卡模式
#define CARD_DEL_MODE               2 //删除卡模式
#define NO_FIND_HEAD                (-1)
#define RECORD_MAX_LEN              128 //每条通行记录的最大长度
#define RECORD_REAL_LEN              125 //每条通行记录的真实长度


//表头数据
typedef union
{
	uint32_t id;        //卡号
	uint8_t sn[4];    //卡号按字符
}HEADTPYE;

typedef struct CARDHEADINFO
{
    HEADTPYE headData;  //卡号
}HEADINFO_STRU;

extern HEADINFO_STRU gSectorBuff[1024];


void readTemplateData(void);

void RestoreDefaultSetting(void);

void SystemUpdate(void);

void SaveDevState(uint8_t     state);

//保存模板信息
SYSERRORCODE_E saveTemplateParam(uint8_t *jsonBuff);

//add 2020.07.06
void initTemplateParam(void);
uint8_t optTemplateParam(void *stParam,uint8_t mode,uint32_t len,uint32_t addr);

void initDevBaseParam(void);
uint8_t optDevBaseParam(void *stParam,uint8_t mode,uint32_t len,uint32_t addr);



uint8_t optRecordIndex(RECORDINDEX_STRU *recoIndex,uint8_t mode);

void ClearDevBaseParam(void);
void ClearTemplateParam(void);
void ClearRecordIndex(void);
void clearTemplateFRAM(void);


void eraseHeadSector(void);
void eraseDataSector(void);
void eraseUserDataAll(void);

uint8_t setLocalIpAddr(uint8_t *ipAddr,uint8_t * netMask,uint8_t *gateWay);
uint8_t resetLocalBaseParam(void);
uint8_t resetLocalSn(void);

void TestFlash(uint8_t mode);

void initDevParam(void);

void setProgramMode(PROGRAM_MODE mode);
void setCardReaderType(CARD_READER_TYPE type);
void setDoorType(DOOR_TYPE type);

PROGRAM_MODE getProgramMode(void);
CARD_READER_TYPE getCardReaderType(void);
DOOR_TYPE getDoorType(void);






#endif
