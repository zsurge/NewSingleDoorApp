/******************************************************************************

                  Copyright @1998 - 2021 BSG

 ******************************************************************************
�� �� ��   : doorparam.h
 					
@author  
@date 2021��2��26��
  ����޸�   :
@file doorparam.h
@brief �ŵ�������Լ��������ݽṹ 
  �����б�   :
�޸���ʷ   :
@date 2021��2��26��
@author  
�޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "stdint.h"
/*----------------------------------------------*
 * �궨��                                 *
 *----------------------------------------------*/
 
//typedef void (*initCallBack)(void); //������ʼ��
//typedef char(*addCallBack)(uint32_t,char *,int);
//typedef char(*delCallBack)(uint32_t,char *,int);
//typedef char (*modifyCallBack)(uint32_t,char *,int,char *,int);
//typedef char (*readCallBack)(uint32_t,char *,int *);

 /*----------------------------------------------*
 * ȫ�ֱ���                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ö�ٶ���                            *
 *----------------------------------------------*/
//���ŷ�ʽ
typedef enum OPEN_TYPE
{
    FIRST_CARD_AUTH= 0, //�׿���Ȩ
    FIRST_CARD_NORMAL,  //�׿�����
    MULTI_CARD_SIMPLE,  //�򵥶࿨  
    BRUSH_CARD,         //ˢ��
    COERCION_BRUSH_CARD,//в��ˢ��
    PASSWORD,           //���뿪��
    COERCION_PASSWORD,  //в������
    QR_CODE,            //��ά�뿪��
    CARD_PASSWORD,      //��+���뿪��
    MUTEX               //���⿪��
}OPEN_TYPE_ENUM;

//�Ž�����
typedef enum DEV_TYPE
{
    ONE_DOOR = 1, //һ���Ž�
    TWO_DOORS     //�����Ž�          
}DEV_TYPE_ENUM;

//����ģʽ
typedef enum WORK_TYPE
{
    NORMAL = 0,     //����ģʽ
    ALWAYS_OPEN,    //����ģʽ
    TIMING_OPEN     //��ʱ����          
}WORK_TYPE_ENUM;



//�Ƿ�����Ƿ�����
typedef enum ILLEGALWARNING
{
    NO = 0,
    YES           
}ILLEGALWARNING_ENUM;

/*----------------------------------------------*
 * �ṹ�嶨��                              *
 *----------------------------------------------*/
 //��Ч��
typedef struct VALID_TIME
{
    int beginTime;
    int endTime;
}VALID_TIME_STRU;

//�׿���Ȩ
typedef struct FIRST_CARD_AUTH
{
    uint8_t isBrushCard;    //��ˢ�׿���־
    VALID_TIME_STRU validTime;   //ˢ�׿���Ч��          
}FIRST_CARD_AUTH_STRU;

//�࿨����
typedef struct MULIT_CARD_AUTH
{
    uint8_t isMulitCard;    //�Ƿ��Ƕ࿨����
    uint8_t mulitCardNumber;//�࿨��������
    VALID_TIME_STRU validTime;   //�࿨������Ч��
}MULIT_CARD_AUTH_STRU;

//в�����뿪��
typedef struct COERCION_PW_AUTH
{
    uint8_t isEnable;   //�Ƿ�����в�����뿪��
    uint8_t passWord[6];//в������
}COERCION_PW_AUTH_STRU;

//в�ȿ�����
typedef struct COERCION_CARD_AUTH
{
    uint8_t isEnable;   //�Ƿ�����в�����뿪��
    uint8_t card[4];    //в�ȿ���
}COERCION_CARD_AUTH_STRU;

//���뿪��
typedef struct PASS_WORD_AUTH
{
    uint8_t isEnable;//�Ƿ������뿪��
    uint8_t passWord[6];//��ͨ����
}PASS_WORD_AUTH_STRU;

//��ά�뿪��
typedef struct QR_CODE_AUTH
{
    uint8_t isEnable;
}QR_CODE_AUTH_STRU;

//����+���뿪��
typedef struct CARD_PW_AUTH
{
    uint8_t isEnable;
    uint8_t password[6];
    uint8_t card[4];
}CARD_PW_AUTH_STRU;

//��ʱ����
typedef struct TIMING_AUTH
{
    uint8_t isEnable;
    VALID_TIME_STRU validTime[3];
}TIMING_AUTH_STRU;

//��������
typedef struct MUTEX_AUTH
{
    uint8_t isEnable;
    uint8_t doorA;
    uint8_t doorB;
}MUTEX_AUTH_STRU;
 
typedef struct DEV_PARAM
{
    DEV_TYPE_ENUM devType;  //�豸�ͺţ� 1���Ž�/2���Ž�
    OPEN_TYPE_ENUM openType; //���ŷ�ʽ
    WORK_TYPE_ENUM workType; //Ӧ�ó���
    FIRST_CARD_AUTH_STRU firstCardAuth;  //�׿�ģʽ
    MULIT_CARD_AUTH_STRU mulitCardAuth; //�࿨ģʽ
    COERCION_PW_AUTH_STRU coercionPwAuth; //в�����뿪��ģʽ
    COERCION_CARD_AUTH_STRU coercionCardAuth;//в�ȿ�����ģʽ
    PASS_WORD_AUTH_STRU passwordAuth;    //���뿪��
    QR_CODE_AUTH_STRU   qrCodeAuth;  //��ά�뿪��
    CARD_PW_AUTH_STRU   cardAndPwAuth; //��+���뿪��
    TIMING_AUTH_STRU    timingAuth;    //��ʱ����ģʽ   
    MUTEX_AUTH_STRU     mutexAuth;      //���⿪��
}DEV_PARAM_STRU;



/*----------------------------------------------*
 * �ⲿ����ԭ��˵��                                     *
 *----------------------------------------------*/
extern DEV_PARAM_STRU gDevParam;


/*----------------------------------------------*
 * ��������                                       *
 *----------------------------------------------*/



