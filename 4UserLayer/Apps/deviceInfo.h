/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : deviceInfo.h
  �� �� ��   : ����
  ��    ��   :  
  ��������   : 2020��6��28��
  ����޸�   :
  ��������   : �豸���ֲ������ü�Ĭ��ֵ
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2020��6��28��
    ��    ��   :  
    �޸�����   : �����ļ�

******************************************************************************/
#ifndef __DEVICEINFO_H__
#define __DEVICEINFO_H__
/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "stm32f4xx.h" 

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
 #define UPGRADE_URL_MAX_LEN    300
 #define MQTT_TOPIC_MAX_LEN     128

 #define DEFAULT_INIVAL 0x55AA55BB
 #define DEFAULT_DEV_NAME "SMARTDOOR"

 #define DEVICE_DISABLE 0x00
 #define DEVICE_ENABLE  0x55

 #define CARD_UNORDERED 0x00
 #define CARD_ORDERLY  0x55AA55AA

 #define WRITE_PRARM    0x01
 #define READ_PRARM     0x02

 #define STATIC_IP      0x55AA55AA
 #define DHCP_IP        0xAA55AA55


//#define DEFAULT_TEMPLATE_PARAM                                              
//{                                                                             
//    {.id = 1,
//    .modeType = "5",
//    .templateCode = "100000",
//    .templateName = "defaultName",
//    .templateStatus = 1,
//    .callingWay = "1,2,3",
//    .offlineProcessing = 2
//    }
//}


/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/
////////////////////ģ����Ϣ////////////////////////////////////////
 
 //��������Ϣ
 typedef struct
 {
     uint8_t createorID[12];
     uint8_t createTime[20];
     uint8_t createCompanyId[12];
     uint8_t updateUserId[12];
     uint8_t updateUserName[20];    
 }CREATOR_INFO_STRU;
 
 typedef struct
 {
     uint8_t channelType;//ͨ������ 1���ǻ�ͨ�� 2���ݿ�
     uint8_t voiceSize;
     uint8_t templateType;//ģ��ģʽ���� 1���߷�ģʽ 2���ڼ���ģʽ
     uint8_t startTime[20];
     uint8_t endTime[20];
 }TEMPLATE_BASE_DATA_STRU;
 
 typedef struct 
 {
     uint8_t isFace;
     uint8_t isQrCode;
     uint8_t isIcCard;          
 }CALLINGWAY_STRU;
 
 typedef struct
 {
     uint8_t isPeakMode;
     uint8_t isHolidayMode;
     uint8_t isNormalMode;          
 }WORKMODE_STRU;
 
 typedef struct 
 {
     uint8_t callingWay[8];
     uint8_t beginTime[20];
     uint8_t endTime[20];
     uint8_t outsideTimeMode[20];
     uint8_t outsideTimeData[20];          
 }TEMPLATE_SET_DATA_STRU;

typedef union
{
	unsigned int iFlag;        //
	unsigned char cFlag[4];    //
}DEVICE_SWITCH;

 
 typedef struct
 {
     DEVICE_SWITCH initFlag;   //��ʼ����־\x55\xAA\x55\xBB�϶�Ϊ������ʼ��
     uint8_t id;
     uint8_t modeType[8];
     uint8_t templateCode[20];
     uint8_t templateName[50];
     uint8_t templateStatus;
     uint8_t callingWay[8];
     uint8_t offlineProcessing;
     uint8_t peakModeCnt;
     WORKMODE_STRU workMode;
     CALLINGWAY_STRU templateCallingWay;
     CALLINGWAY_STRU peakCallingWay;
     TEMPLATE_BASE_DATA_STRU peakMode[3];
     TEMPLATE_BASE_DATA_STRU holidayMode[3];
     TEMPLATE_SET_DATA_STRU peakInfo[1];
     TEMPLATE_SET_DATA_STRU hoildayInfo[1];
     CREATOR_INFO_STRU creatorInfo;     
 }TEMPLATE_PARAM_STRU;
//////////////////////////////////////////////////////////////////
 
//����״̬
typedef enum UPGRADE_STATUS
{
    UPGRADING = 0,
    UPGRADE_COMPLETE,
    UPGRADE_FAILED
}UPGRADE_STATUS_ENUM;

typedef struct DEVICE_ID
{
    DEVICE_SWITCH downLoadFlag;       //
    char qrSn[8];                     //��ά���·��ı���ID
    char qrSnLen;
    char deviceSn[32];                //MQTT ����ʱ��SN      
    char deviceSnLen;
}DEVICE_ID_STRU;

typedef struct UPGRADE_URL
{
    char upgradeState;//�����У�������ɣ�����ʧ��
    char url[UPGRADE_URL_MAX_LEN];//���ڴ洢�����ļ���ŵ�ַ
    char retUrl[UPGRADE_URL_MAX_LEN];//֪ͨ����������״̬������
}UPGRADE_URL_STRU;

typedef struct
{
    char publish[MQTT_TOPIC_MAX_LEN];   //����������
    char subscribe[MQTT_TOPIC_MAX_LEN]; //���ĵ�����
}MQTT_TOPIC_STRU;

//IP�Ļ�ȡ��ʽ ��̬�Ͷ�̬��DHCP��
typedef union
{
	unsigned int iMode;        
	unsigned char cMode[4];    
}IP_MODE;

typedef struct IP_CONFIG
{
    uint8_t ip[4];
    uint8_t netMask[4];
    uint8_t gateWay[4];
    //����IP״̬
    IP_MODE ipMode; 
}IP_CONFIG_STRU;


 //����ģʽ 0 = ��ģʽ��1 = �Ž�ģʽ��2=ͨ��բģʽ
 //����ģʽ
 typedef enum {
     PROGRAMMODE_TEST = 0,      //����ģʽ 
     PROGRAMMODE_DOOR,          //�Ž�ģʽ
     PROGRAMMODE_CHANNEL        //ͨ��բģʽ
 }PROGRAM_MODE;

  //���������
 typedef enum {
     CARD_READER_WG = 0,       //Τ��������
     CARD_READER_RS485         //RS485������
 }CARD_READER_TYPE;

   //���������
 typedef enum {
     DOOR_TYPE_ONE = 0,      //һ���Ž�
     DOOR_TYPE_TWO           //�����Ž�
 }DOOR_TYPE;

  //�豸����״̬
 typedef enum {
     UPLOAD_MAC = 0,      //һ���Ž�
     DWLOAD_KEY,           //�����Ž�
     DWLOAD_SN    
 }PRODUCT_STATUS;

typedef struct DEVICE_STATUS
{
    unsigned char isEnable;    
    unsigned char isUpLoadMac;
    unsigned char isDwLoadKey;    
    unsigned char isDwLoadSn;       
}DEVICE_STATUS_STRU;

 
typedef struct DEV_BASE_PARAM
{
    //����ģʽ
    PROGRAM_MODE progamMode;    //������ ����/�Ž�/ͨ�� ģʽ

    CARD_READER_TYPE cardReaderType; //Τ��/485

    DOOR_TYPE doorType;         //һ��/����   
    
    //�豸״̬
    DEVICE_STATUS_STRU deviceState; 
    
    //SN
    DEVICE_ID_STRU deviceCode;

    //��������
    UPGRADE_URL_STRU upgradeInfo;   

    //MQTT����
    MQTT_TOPIC_STRU mqttTopic; 

    //����IP��ַ
    IP_CONFIG_STRU localIP;   
}DEV_BASE_PARAM_STRU;

///////////////////////FLASH���///////////////////////////////////////////

//�洢��ͷ��һЩ����ֵ
typedef struct RECORDINDEX
{
    volatile uint32_t cardNoIndex;      //��ǰ�Ѵ洢�˶��ٸ�����
    volatile uint32_t delCardNoIndex;   //��ǰ�Ѵ洢�˶��ٸ���ɾ���Ŀ���    
    volatile uint32_t accessRecoIndex;   //��ǰ�Ѵ洢�˶���ͨ�м�¼
}RECORDINDEX_STRU;

typedef struct PUBLICKEY
{
    uint8_t pubKey[40];
    uint8_t devKey[24];          
}PUBLICKEY_STRU;

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
extern TEMPLATE_PARAM_STRU gtemplateParam;
extern DEV_BASE_PARAM_STRU gDevBaseParam;
extern RECORDINDEX_STRU gRecordIndex;
extern PUBLICKEY_STRU gPublicKey;



/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/

#endif

