/*********************************************************************************************************
**
**                                    �й������Դ��֯
**
**                                   Ƕ��ʽʵʱ����ϵͳ
**
**                                       SylixOS(TM)
**
**                               Copyright  All Rights Reserved
**
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**
** ��   ��   ��: spc_master.c
**
** ��   ��   ��: Zhang.ZhaoGe (���׸�)
**
** �ļ���������: 2018 �� 07 �� 15 ��
**
** ��        ��: spc �����˴��� (ģ���� OBC �˹���)
*********************************************************************************************************/
#include "SylixOS.h"
#include "pthread.h"
#include "stdlib.h"
#include "string.h"
#include "getopt.h"
#include <stdio.h>
#include <endian.h>
#include "../SylixOS/system/device/can/can.h"
/*********************************************************************************************************
  �궨��
*********************************************************************************************************/
#define SENDBUFLEN                      50                              /*  �������ݻ�������С          */
#define RCVBUFLEN                       198                             /*  �������ݻ�������С          */
#define CANBUAD                         500000                          /*  can ������                  */
#define CANDEV                          "/dev/can0"                     /*  can �豸�ļ�                */
/*********************************************************************************************************
  ������������
*********************************************************************************************************/
typedef struct spc_send_data {
    CHAR   *pcTransaction;
    UCHAR   ucTxidArrayNum;
    CHAR   *pcTxid[5];
    UCHAR   ucVout[5];
    CHAR   *pcScriptPubKey[5];
    CHAR   *pcRedeemScript[5];
    UINT16  uiPrivKeyIndex;
} SPC_SEND_DATA_T;
/*********************************************************************************************************
  ȫ�ֱ�������
*********************************************************************************************************/
static UCHAR _G_ucSendBuf[SENDBUFLEN];                                  /*  �������ݻ�����              */
static UCHAR _G_ucRcvBuf[RCVBUFLEN];                                    /*  �������ݻ�����              */
/*********************************************************************************************************
** ��������: spcCharToHex
** ��������: ���ַ�ת�ɶ�������
** �䡡��  : cIn
** �䡡��  : ���
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
UCHAR  spcCharToHex (CHAR  cIn)
{
    if ((cIn >= '0') && (cIn <= '9')) {
        return (cIn - '0');
    } else if ((cIn >= 'a') && (cIn <= 'f')) {
        return (cIn - 'a' + 10);
    } else if ((cIn >= 'A') && (cIn <= 'F')) {
        return (cIn - 'A' + 10);
    } else {
        return (-1);                                                    /*  NOP                         */
    }
}
/*********************************************************************************************************
** ��������: spcHexToChar
** ��������: ���ַ�ת�ɶ�������
** �䡡��  : cIn
** �䡡��  : ���
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
CHAR  spcHexToChar (UCHAR  ucIn)
{
    if ((ucIn >= 0) && (ucIn <= 9)) {
        return (ucIn + '0');
    } else if ((ucIn >= 10) && (ucIn <= 15)) {
        return (ucIn - 10 + 'a');
    } else {
        return (-1);                                                    /*  NOP                         */
    }
}
/*********************************************************************************************************
** ��������: spcStringToHex
** ��������: �� hash �ַ���ת���ɶ��������ݸ�ʽ��"0123456789abcdef" --- 0x0123456789abcdef��
** �䡡��  : cString    ���ַ�����ַ
**         : iLen       ���ַ�������
**         : ucOutData  ��ת�����
** �䡡��  : �ɹ�ת�����ַ�����
** ������Ϣ: ��Ҫע�������ַ����ַ����������������
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
INT  spcStringToHex (UCHAR  *ucOutData, CHAR  *cString, INT  iLen)
{
    CHAR  *cData = cString;
    INT    iInIndex  = 0;
    INT    iOutIndex = 0;

    for (iInIndex = 0; iInIndex < iLen; iInIndex++) {
        if((iInIndex % 2) == 0) {
            // ��������һ���ֽڵĸ���λ
            *(ucOutData + iOutIndex) |= (spcCharToHex(*(cData + iInIndex)) << 4);
        } else {
            // ż������һ���ֽڵĵ���λ
            *(ucOutData + iOutIndex) |= (spcCharToHex(*(cData + iInIndex)));
            iOutIndex++;
        }
    }

    return (iInIndex);
}
/*********************************************************************************************************
** ��������: spcHexToString
** ��������: �Ѷ��������ݸ�ʽת���� hash �ַ�����0x0123456789abcdef --- "0123456789abcdef"��
** �䡡��  : cString     �����������ݵ�ַ
**         : iLen        ����Ч���ֽڸ���
**         : bIs         ��ucInData ���һ���ֽ������Ƿ�ֻ��һ����Ч������ַ������Ƿ�Ϊ��������
**         : cOutString  ��ת�����
** �䡡��  : �ַ�������
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
INT  spcHexToString (CHAR  *cOutString, UCHAR  *ucInData, INT  iLen, BOOL bIs)
{
    UCHAR  *ucData = ucInData;
    INT     iInIndex = 0;
    INT     iOutIndex = 0;

    for (iInIndex = 0; iInIndex < iLen; iInIndex++) {
        *(cOutString + iOutIndex) = spcHexToChar(((*(ucData + iInIndex)) & 0xF0) >> 4);
        iOutIndex++;
        *(cOutString + iOutIndex) = spcHexToChar((*(ucData + iInIndex)) & 0x0F);
        iOutIndex++;
    }

    if (bIs) {
        iOutIndex--;
    }
    *(cOutString + iOutIndex) = '\0';

    return (iOutIndex);
}
/*********************************************************************************************************
** ��������: spcMasterSendToSlave
** ��������: ������ӻ�����ע������
** �䡡��  : iFd��     can �����豸�ļ�
**           pucBuf��  �������ݻ�����
**           iDataLen���������ݳ��ȣ����50
** �䡡��  :
** ������Ϣ: ÿ�η��� 50 �ֽڣ�1����֡(5B) + 6���м�֡(7B) + 1��β֡(3B)
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
VOID  spcMasterSendToSlave (INT  iFd, UCHAR  *pucBuf, INT  iDataLen)
{
    CAN_FRAME   canframe;
    ssize_t     stLen;
    ssize_t     stFrameNum;
    INT         i;
    UCHAR       ucIndex = 0;

    /*
     * �������ݵ����ػ�������
     */
    if (iDataLen > SENDBUFLEN) {
        printf("data too long\n");
        return;
    }
    memset(_G_ucSendBuf, 0, SENDBUFLEN);
    memcpy(_G_ucSendBuf, pucBuf, iDataLen);

    /*
     * ��֡���ݴ��������
     */
    canframe.CAN_bRtr       = 0;                                        /*  ��Զ��֡                    */
    canframe.CAN_bExtId     = 0;                                        /*  ����չ֡                    */
    canframe.CAN_uiId       = 0x2D;
    canframe.CAN_ucLen      = 8;

    canframe.CAN_ucData[0]  = 0x01;
    canframe.CAN_ucData[1]  = 0x08;
    canframe.CAN_ucData[2]  = 0x5A;

    memcpy(&canframe.CAN_ucData[3], _G_ucSendBuf + ucIndex, 5);
    ucIndex += 5;

    stLen = write(iFd, &canframe, sizeof(CAN_FRAME));
    ioctl(iFd, CAN_DEV_STARTUP, 0);

    stFrameNum = stLen / sizeof(CAN_FRAME);

    if (stFrameNum != 1) {
        printf("failed to send can frame!, abort sending\n");
        return;
    }

    /*
     * �м�֡���ݴ��������
     */
    for (i = 0; i < 6; i++) {
        canframe.CAN_uiId       = 0x2E;
        canframe.CAN_ucLen      = 8;

        canframe.CAN_ucData[0]  = i + 2;

        if (ucIndex > (SENDBUFLEN - 7)) {
            return;
        }
        memcpy(&canframe.CAN_ucData[1], _G_ucSendBuf + ucIndex, 7);
        ucIndex += 7;

        stLen = write(iFd, &canframe, sizeof(CAN_FRAME));
        ioctl(iFd, CAN_DEV_STARTUP, 0);

        stFrameNum = stLen / sizeof(CAN_FRAME);

        if (stFrameNum != 1) {
            printf("failed to send can frame!, abort sending\n");
            return;
        }
    }

    /*
     * β֡���ݴ��������
     */
    canframe.CAN_uiId       = 0x2F;
    canframe.CAN_ucLen      = 4;
    canframe.CAN_ucData[0]  = 0x08;                                     /*  �����                      */

    if (ucIndex > (SENDBUFLEN - 3)) {
        return;
    }

    memcpy(&canframe.CAN_ucData[1], _G_ucSendBuf + ucIndex, 3);
    ucIndex += 3;

    stLen = write(iFd, &canframe, sizeof(CAN_FRAME));
    ioctl(iFd, CAN_DEV_STARTUP, 0);

    stFrameNum = stLen / sizeof(CAN_FRAME);

    if (stFrameNum != 1) {
        printf("failed to send can frame!, abort sending\n");
        return;
    }

    return;
}
/*********************************************************************************************************
** ��������: spcMasterRequestTelemetryData
** ��������: ������ӻ���������ң����������
** �䡡��  : iFd��can �����豸�ļ�
** �䡡��  :
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
VOID  spcMasterRequestTelemetryData (INT  iFd)
{
    CAN_FRAME   canframe;
    ssize_t     stLen;
    ssize_t     stFrameNum;

    canframe.CAN_bRtr       = 0;                                        /*  ��Զ��֡                    */
    canframe.CAN_bExtId     = 0;                                        /*  ����չ֡                    */
    canframe.CAN_uiId       = 0x30;
    canframe.CAN_ucLen      = 1;
    canframe.CAN_ucData[0]  = 0x1A;

    stLen = write(iFd, &canframe, sizeof(CAN_FRAME));
    ioctl(iFd, CAN_DEV_STARTUP, 0);

    stFrameNum = stLen / sizeof(CAN_FRAME);

    if (stFrameNum != 1) {
        printf("failed to send can frame!, abort sending\n");
        return;
    }

    return ;
}
/*********************************************************************************************************
** ��������: spcMasterRequestTestTelemetryData
** ��������: ������ӻ������������ң����������
** �䡡��  : iFd��can �����豸�ļ�
** �䡡��  :
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
VOID  spcMasterRequestTestTelemetryData (INT  iFd)
{
    CAN_FRAME   canframe;
    ssize_t     stLen;
    ssize_t     stFrameNum;

    canframe.CAN_bRtr       = 0;                                        /*  ��Զ��֡                    */
    canframe.CAN_bExtId     = 0;                                        /*  ����չ֡                    */
    canframe.CAN_uiId       = 0x30;
    canframe.CAN_ucLen      = 1;
    canframe.CAN_ucData[0]  = 0x1B;

    stLen = write(iFd, &canframe, sizeof(CAN_FRAME));
    ioctl(iFd, CAN_DEV_STARTUP, 0);

    stFrameNum = stLen / sizeof(CAN_FRAME);

    if (stFrameNum != 1) {
        printf("failed to send can frame!, abort sending\n");
        return;
    }

    return ;
}
/*********************************************************************************************************
** ��������: spcMasterBroadcastTimeData
** ��������: �����㲥ʱ������
** �䡡��  : iFd��can �����豸�ļ�
** �䡡��  :
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
VOID spcMasterBroadcastTimeData (INT iFd)
{
    struct timespec tv;
    time_t          tTime;
    CAN_FRAME       canframe;
    ssize_t         stLen;
    ssize_t         stFrameNum;

    if (lib_clock_gettime(CLOCK_REALTIME, &tv) < 0) {                   /*  ���ϵͳʱ��                */
        return;
    }

    /*
     * 2018 �� 1 �� 1 �� 0 ʱ 0 �� 0 �루1514736000����ʼ������
     */
    tTime = tv.tv_sec - 1514736000;
    HTOBE32(tTime);

    canframe.CAN_bRtr       = 0;                                        /*  ��Զ��֡                    */
    canframe.CAN_bExtId     = 0;                                        /*  ����չ֡                    */
    canframe.CAN_uiId       = 0x18;
    canframe.CAN_ucLen      = 6;

    memset(&canframe.CAN_ucData[0], 0, 6);
    memcpy(&canframe.CAN_ucData[0], (UCHAR *)&tTime, 4);

    stLen = write(iFd, &canframe, sizeof(CAN_FRAME));
    ioctl(iFd, CAN_DEV_STARTUP, 0);

    stFrameNum = stLen / sizeof(CAN_FRAME);

    if (stFrameNum != 1) {
        printf("failed to send can frame!, abort sending\n");
        return;
    }

    return;
}
/*********************************************************************************************************
** ��������: spcPacketSendData
** ��������: �����������
** �䡡��  : pSendData����Ҫ���͵�ԭʼ����
** �䡡��  : ucData   ����������ݽ��
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
INT  spcPacketSendData (UCHAR  *ucData, SPC_SEND_DATA_T  *pSendData)
{
    INT              i;
    INT              sRet;
    UINT16           usHexLong;
    UINT16           usIndex = 0;

    ucData[usIndex++] = 0xa1;
    usIndex += 2;

    ucData[usIndex++] = 0xba;

    sRet = spcStringToHex(ucData + usIndex + 2, pSendData->pcTransaction, strlen(pSendData->pcTransaction));

    usHexLong = (sRet / 2) + (sRet % 2);
    if (sRet % 2) {
        usHexLong |= 0x8000;
    }

    memcpy(ucData + usIndex, &usHexLong, 2);
    usIndex = usIndex + 2 + (usHexLong & 0x7FFF);

    ucData[usIndex++] = 0xca;
    ucData[usIndex++] = 0;
    ucData[usIndex++] = 0;
    ucData[usIndex++] = pSendData->ucTxidArrayNum;

    for (i = 0; i < pSendData->ucTxidArrayNum; i++) {

        sRet = spcStringToHex(ucData + usIndex + 1, pSendData->pcTxid[i], strlen(pSendData->pcTxid[i]));

        usHexLong = (sRet / 2) + (sRet % 2);
        if (sRet % 2) {
            usHexLong |= 0x80;
        }

        *(ucData + usIndex) = usHexLong & 0xFF;
        usIndex = usIndex + 1 + (usHexLong & 0x7F);

        ucData[usIndex++] = pSendData->ucVout[i];

        sRet = spcStringToHex(ucData + usIndex + 2, pSendData->pcScriptPubKey[i], strlen(pSendData->pcScriptPubKey[i]));

        usHexLong = (sRet / 2) + (sRet % 2);
        if (sRet % 2) {
            usHexLong |= 0x8000;
        }

        memcpy(ucData + usIndex, &usHexLong, 2);
        usIndex = usIndex + 2 + (usHexLong & 0x7FFF);

        sRet = spcStringToHex(ucData + usIndex + 2, pSendData->pcRedeemScript[i], strlen(pSendData->pcRedeemScript[i]));

        usHexLong = (sRet / 2) + (sRet % 2);
        if (sRet % 2) {
            usHexLong |= 0x8000;
        }

        memcpy(ucData + usIndex, &usHexLong, 2);
        usIndex = usIndex + 2 + (usHexLong & 0x7FFF);
    }

    memcpy(ucData + usIndex, &pSendData->uiPrivKeyIndex, 2);
    usIndex += 2;

    ucData[usIndex] = 0x1a;
    memcpy(ucData + 1, &usIndex, 2);

    return (0);
}
/*********************************************************************************************************
** ��������: pthreadTestSend
** ��������: CAN �������� pthread �����߳�
** �䡡��  : pvArg         �̲߳���
** �䡡��  : �̷߳���ֵ
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static VOID *pthreadTestSend(VOID  *pvArg)
{
    INT       iFd;
    INT       iCnt    = 0;
    char     *devname = (char *)pvArg;

    iFd = open(devname, O_RDWR, 0666);
    if (iFd < 0) {
        printf("failed to open %s!\n", devname);
        return  (LW_NULL);
    }

    if (ioctl(iFd, CAN_DEV_SET_BAUD, CANBUAD)) {
        printf("set baud to %d error.\n", CANBUAD);
        close (iFd);
        return  (LW_NULL);
    }
    ioctl(iFd, FIOFLUSH, 0);


    /*
     * �����������ң������
     */
    spcMasterRequestTestTelemetryData(iFd);
    usleep(500000);

    /*
     * ͬ������ʱ�䵽�ӻ�
     */
    spcMasterBroadcastTimeData (iFd);
    usleep(500000);

    {
        INT              i;
        CHAR            *cHexData         = "0200000001eadd31b0f532542b4fda3f276ef84248ecd04bbdc254a102317ae8445704818600000000b500483045022100cc400cfb26c884d0388227c23b9018e5f05995bc5541948d0f1943d092eb100902201f48932b64db8a885784b3c6dd7001fe5dfbd1703a42544a0b010af193704439014c69522103f622b4e144f901f6efc9ba3f57be34534155386574e5a04e388c0de22da554f12103b903caea072beb172ca4fc44c6b1e3ade6451dad90384778528fdcf8f3a0786e210315015bd030f0e08a3f6ae73e9beeb3634cb71239998d826520bdb7d33b6adcb453aeffffffff018033023b000000001976a9147aa23e65228b9ee440dd4d1b3f3c0710a18a179b88ac00000000";
        CHAR            *cTxidData        = "8681045744e87a3102a154c2bd4bd0ec4842f86e273fda4f2b5432f5b031ddea";
        CHAR            *cPubKey          = "a914647ef6664d6b36e236d000a234c7f26ee0e5014a87";
        CHAR            *cRedeemScryptKey = "522103f622b4e144f901f6efc9ba3f57be34534155386574e5a04e388c0de22da554f12103b903caea072beb172ca4fc44c6b1e3ade6451dad90384778528fdcf8f3a0786e210315015bd030f0e08a3f6ae73e9beeb3634cb71239998d826520bdb7d33b6adcb453ae";
        UCHAR            ucData[2048]     = {0};
        SPC_SEND_DATA_T  spcSendData;

        spcSendData.pcTransaction     = cHexData;
        spcSendData.ucTxidArrayNum    = 1;

        spcSendData.pcTxid[0]         = cTxidData;
        spcSendData.ucVout[0]         = 0;
        spcSendData.pcScriptPubKey[0] = cPubKey;
        spcSendData.pcRedeemScript[0] = cRedeemScryptKey;

        /*
         * spcSendData.pcTxid[1]         = cTxidData;
         * spcSendData.ucVout[1]         = 1;
         * spcSendData.pcScriptPubKey[1] = cPubKey;
         * spcSendData.pcRedeemScript[1] = cRedeemScryptKey;
         */

        spcSendData.uiPrivKeyIndex    = 0;

        spcPacketSendData (ucData, &spcSendData);

        for (i = 0; i < 10; i++) {
            spcMasterSendToSlave (iFd, &ucData[i * 50], 50);
            usleep(500000);
            printf("i = %d\n", i);
        }

    }

    sleep(3);                                                           /*  �ȴ���������ǩ�����        */

    for (iCnt = 0; iCnt < 4; iCnt++) {                                  /*  ��������ӻ�ң������        */
        sleep(1);
        spcMasterRequestTelemetryData (iFd);
    }

    /*
     * �ȴ�����ȫ���������
     */
    while (1) {
        INT iNum;

        if (ioctl(iFd, FIONWRITE, &iNum)) {
            printf("error");
            break;
        }

        if (iNum == 0) {
            break;
        }
    }

    ioctl(iFd, CAN_DEV_STARTUP, 0);

    close(iFd);

    return  (NULL);
}
/*********************************************************************************************************
** ��������: pthreadTestRecv
** ��������: CAN �������� pthread �����߳�
** �䡡��  : pvArg        �̲߳���
** �䡡��  : �̷߳���ֵ
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static VOID  *pthreadTestRecv(VOID  *pvArg)
{
    INT         iFd;
    CAN_FRAME   canframe;
    ssize_t     stLen;
    ssize_t     stFrameNum;
    char       *devname = (char *)pvArg;

    memset(&canframe, 0, sizeof(CAN_FRAME));

    iFd = open(devname, O_RDWR, 0666);
    if (iFd < 0) {
        printf("failed to open %s!\n", devname);
        return  (LW_NULL);
    }

    if (ioctl(iFd, CAN_DEV_SET_BAUD, CANBUAD)) {
        printf("set baud to %d error.\n", CANBUAD);
        close (iFd);
        return  (LW_NULL);
    }

    while(1) {
        /*
         * ��ʼ��������
         */
        stLen = read(iFd, &canframe, sizeof(CAN_FRAME));
        stFrameNum = stLen / sizeof(CAN_FRAME);

        if (stFrameNum != 1) {
            /*
             * ��������ʧ��
             */
            printf("failed to recv can frame, abort recving!\n");

        } else {
            if ((canframe.CAN_uiId == 0x431) || (canframe.CAN_uiId == 0x432)
               || (canframe.CAN_uiId == 0x433)) {
                /*
                 * ����������ң�����ݣ����ض�֡����
                 * ÿ�� 198 �ֽڣ�1����֡(5B) + 27���м�֡(7B) + 1��β֡(4B)
                 */
                static UCHAR ucIndex = 0;
                static UCHAR ucIsTestData;

                if (canframe.CAN_uiId == 0x431) {
                    /*
                     * �յ���֡����
                     */
                    ucIndex = 0;

                    if (canframe.CAN_ucData[2] == 0x1B) {
                        ucIsTestData = 1;
                    } else if (canframe.CAN_ucData[2] == 0x1A){
                        ucIsTestData = 0;
                    } else {
                                                                        /*  NOP                         */
                    }

                    memset(_G_ucRcvBuf, 0, RCVBUFLEN);
                    memcpy(_G_ucRcvBuf + ucIndex, &canframe.CAN_ucData[3], 5);
                    ucIndex += 5;

                } else if (canframe.CAN_uiId == 0x432) {
                    if (ucIndex > (RCVBUFLEN - 7)) {
                        return  (NULL);
                    }

                    memcpy(_G_ucRcvBuf + ucIndex, &canframe.CAN_ucData[1], 7);
                    ucIndex += 7;

                } else if (canframe.CAN_uiId == 0x433) {
                    /*
                     * ������յ���β֡����� OBC ���ؽ��յ��������ֽ���
                     */
                    if (ucIndex > (RCVBUFLEN - 4)) {
                        return  (NULL);
                    }

                    if (ucIsTestData == 1) {
                        memcpy(_G_ucRcvBuf + ucIndex, &canframe.CAN_ucData[1], 3);
                        ucIndex += 3;
                    } else {
                        memcpy(_G_ucRcvBuf + ucIndex, &canframe.CAN_ucData[1], 4);
                        ucIndex += 4;
                    }

                    {
                        /*
                         * ��ӡ���յ�������
                         */
                        INT  i;
                        printf("\n");
                        for (i = 0; i < ucIndex; i++) {
                            printf("%02x", _G_ucRcvBuf[i]);
                        }
                        printf("\n");
                    }

                } else {

                }
            }
        }
    }

    close(iFd);

    return  (NULL);
}
/*********************************************************************************************************
** ��������: spcMasterProtocolTestStart
** ��������: CAN ����
** �䡡��  : NONE
** �䡡��  : ERROR_CODE
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
INT  spcMasterProtocolTestStart (char *pcdevname)
{
    pthread_t   tid_recv;
    pthread_t   tid_send;
    INT         iError = ERROR_NONE;

    iError = pthread_create(&tid_send, NULL, pthreadTestSend, pcdevname);
    pthread_detach(tid_send);

    iError = pthread_create(&tid_recv, NULL, pthreadTestRecv, pcdevname);
    pthread_join(tid_recv, NULL);

    return  (iError == 0 ? ERROR_NONE : PX_ERROR);
}
/*********************************************************************************************************
** ��������: main
** ��������: ������
** �䡡��  : argc�������в�������
**           argv�������в����׵�ַ
** �䡡��  :
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
int  main (int  argc, char  *argv[])
{
    spcMasterProtocolTestStart((CHAR *)CANDEV);

    return  (0);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
