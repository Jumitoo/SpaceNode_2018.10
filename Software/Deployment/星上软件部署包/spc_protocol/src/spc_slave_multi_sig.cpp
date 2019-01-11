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
** ��   ��   ��: spc_slave.c
**
** ��   ��   ��: Zhang.ZhaoGe (���׸�)
**
** �ļ���������: 2018 �� 07 �� 15 ��
**
** ��        ��: spc �ӻ��˴��� (ģ�����غɶ˹���)
*********************************************************************************************************/
#include "SylixOS.h"
#include "pthread.h"
#include "stdlib.h"
#include "string.h"
#include "getopt.h"
#include <stdio.h>
#include <endian.h>
#include "hwrtc/hwrtc.h"
#include "can/can.h"
#include "json/json.h"
#include <iostream>
#include <string>
/*********************************************************************************************************
  �������Ͷ���
  [start flag 0x5A][2B long][xB data][end flag 0xA5][...]
*********************************************************************************************************/
typedef struct {
    UINT8     ucStartFlag;
    UINT16    usDataLong;                                               /*  ����� 496 �ֽ�             */
    UINT8     ucData[1];
} __attribute__((packed)) ProtocolDataHead;
/*********************************************************************************************************
  �궨��
*********************************************************************************************************/
#define IMPORTPRIVKEYLEN          43                                    /*  ����˽Կ�����            */
#define PRIVKEYLEN                52                                    /*  ��Ǯ��˽Կ�ֽڳ���          */
#define RCVBUFLEN                 50                                    /*  �������ݻ�������С          */
#define ALLRCVBUFLEN              500                                   /*  �������ݻ�������С          */
#define SENDBUFLEN                198                                   /*  �������ݻ�������С          */
#define ALLSENDBUFLEN             2048                                  /*  �������ݻ�������С          */
#define CANBUAD                   500000                                /*  can ������                  */
#define CANDEV                    "/dev/can0"                           /*  can �豸�ļ�                */
#define SIGCMDSTR                 "/apps/qtum/qtum-cli signrawtransaction "
                                                                        /*  ����ǩ������                */
#define STARTSTR                  " ["                                  /*  txid key ��ʼ�ַ���         */
#define TXIDSTR                   "{\\\"txid\\\":\\\""                  /*  txid key �ַ���             */
#define VOUTSTR                   "\\\",\\\"vout\\\":"                  /*  vout key �ַ���             */
#define PUBKEYSTR                 ",\\\"scriptPubKey\\\":\\\""          /*  pubkey key �ַ���           */
#define REDEEMSCRIPTSTR           "\\\",\\\"redeemScript\\\":\\\""      /*  amount key �ַ���           */
#define ENDSTR                    "\\\"},"                              /*  end �ַ���                  */
#define ENDENDSTR                 "]"                                   /*  end end �ַ���              */
#define PRIVKEYSTARTSTR           " [\\\""                              /*  priv key start �ַ���       */
#define PRIVKEYENDSTR             "\\\"]"                               /*  priv key end �ַ���         */
#define PRIVKEYFILEDIC            "/root/"                              /*  priv key �ļ�·��           */
#define IMPORTPRIVKEYSTR          "/apps/qtum/qtum-cli importprivkey "
                                                                        /*  ����˽Կ����                */
/*********************************************************************************************************
  ȫ�ֱ�������
*********************************************************************************************************/
static UINT32 _G_usRestartCnt          = 0;                             /*  ��¼�ӻ�����������          */

static UCHAR  _G_ucData[CAN_MAX_DATA]  = {0xFA, 0xCE, 0xFA, 0xCE,
                                         0xFA, 0xCE, 0xFA, 0xCE};       /*  �������͸�λ����ĸ�������  */
static UCHAR  _G_ucSendBuf[SENDBUFLEN];                                 /*  �������ݻ�����              */
static UCHAR  _G_ucAllSendBuf[ALLSENDBUFLEN];                           /*  ���������ܻ�����            */
static UINT   _G_uiAllSendDataIndex    = 0;                             /*  ���������ܻ�������          */
static UCHAR  _G_ucRcvBuf[RCVBUFLEN];                                   /*  �������ݻ�����              */
static UCHAR  _G_ucAllRcvBuf[ALLRCVBUFLEN];                             /*  ���������ܻ�����            */
static UINT   _G_uiAllRcvDataIndex     = 0;                             /*  ���������ܻ�������          */
/*********************************************************************************************************
** ��������: spcStringStrip
** ��������: ȥ���������еĻس��Ϳո�
** �䡡��  : cString ����Ҫ��ʽ���Ļ�����
**         : iLen    ���ַ�������
** �䡡��  : ��ʽ������ַ����׵�ַ
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
CHAR  *spcStringStrip(CHAR  *cString, INT  iLen)
{
    INT   i     = 0;
    CHAR *cRet  = cString;
    CHAR *cDest = cString;

    while (((' ' == *cString) || ('\n' == *cString) || ('\r' == *cString)) ?
            (*cString++) :
            (*cDest++ = *cString++)) {
        if (++i >= iLen) {
            *cDest = '\0';
            break;
        }
    }

    return cRet;
}
/*********************************************************************************************************
** ��������: spcSlaveReturnTelemetryData
** ��������: �ӻ�����������ң������
** �䡡��  : iFd��     can �����豸�ļ�
**           pucBuf��  �������ݻ�����
**           iDataLen���������ݳ��ȣ����198
** �䡡��  :
** ������Ϣ: ÿ�� 198 �ֽ� = 1����֡(5B) + 27���м�֡(7B) + 1��β֡(4B)
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
VOID  spcSlaveReturnTelemetryData (INT  iFd, UCHAR  *pucBuf, INT  iDataLen)
{
    CAN_FRAME   canframe;
    ssize_t     stLen;
    ssize_t     stFrameNum;
    UCHAR       ucCnt = 0;
    INT         iIndex;

    if (iDataLen > SENDBUFLEN) {
        printf("data too long\n");
        return;
    }

    memset(_G_ucSendBuf, 0, SENDBUFLEN);
    memcpy(_G_ucSendBuf, pucBuf, iDataLen);                             /*  �������ݵ����ػ�������      */

    /*
     * �����֡����
     */
    canframe.CAN_bRtr       = 0;                                        /*  ��Զ��֡                    */
    canframe.CAN_bExtId     = 0;                                        /*  ����չ֡                    */
    canframe.CAN_uiId       = 0x431;
    canframe.CAN_ucLen      = 8;

    canframe.CAN_ucData[0]  = 0x01;
    canframe.CAN_ucData[1]  = 29;
    canframe.CAN_ucData[2]  = 0x1A;

    memcpy(&canframe.CAN_ucData[3], _G_ucSendBuf + ucCnt, 5);           /*  ��֡��Ч�������ݳ���Ϊ5�ֽ� */
    ucCnt += 5;

    stLen = write(iFd, &canframe, sizeof(CAN_FRAME));
    ioctl(iFd, CAN_DEV_STARTUP, 0);

    stFrameNum = stLen / sizeof(CAN_FRAME);

    if (stFrameNum != 1) {
        printf("failed to send can frame!, abort sending\n");
        return;
    }

    /*
     * ����м�֡����
     */
    canframe.CAN_uiId = 0x432;

    for (iIndex = 2; iIndex <= 28; iIndex++) {
        canframe.CAN_ucData[0] = iIndex;

        memcpy(&canframe.CAN_ucData[1], _G_ucSendBuf + ucCnt, 7);
        ucCnt += 7;

        stLen = write(iFd, &canframe, sizeof(CAN_FRAME));
        ioctl(iFd, CAN_DEV_STARTUP, 0);

        stFrameNum = stLen / sizeof(CAN_FRAME);

        if (stFrameNum != 1) {
            printf("failed to send can frame!, abort sending\n");
            return;
        }
    }

    /*
     * ���β֡����
     */
    canframe.CAN_uiId      = 0x433;
    canframe.CAN_ucLen     = 5;
    canframe.CAN_ucData[0] = 29;

    memcpy(&canframe.CAN_ucData[1], _G_ucSendBuf + ucCnt, 4);           /*  β֡��Ч���ݳ����� 4 �ֽ�   */

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
** ��������: �ӻ������������͵�����ң����������
** �䡡��  : pvArg�������߳�ʱ����Ĳ���
** �䡡��  :
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static VOID  *spcMasterRequestTelemetryData(INT  iFd)
{
    UCHAR  ucBuf[SENDBUFLEN];

    ioctl(iFd, FIOFLUSH, 0);

    memcpy(ucBuf, _G_ucAllSendBuf + _G_uiAllSendDataIndex, SENDBUFLEN);
    spcSlaveReturnTelemetryData(iFd, ucBuf, SENDBUFLEN);
    _G_uiAllSendDataIndex += SENDBUFLEN;

    printf("send Telemetry Data\n");

    if (_G_uiAllSendDataIndex >= ALLSENDBUFLEN) {
        _G_uiAllSendDataIndex = 0;
    }

    /*
     * �ȴ�����ȫ���������
     */
    while (1) {
        INT  iNum;

        if (ioctl(iFd, FIONWRITE, &iNum)) {
            printf("error");
            break;
        }

        if (iNum == 0) {
            break;
        }
    }

    ioctl(iFd, CAN_DEV_STARTUP, 0);

    return  (NULL);
}
/*********************************************************************************************************
** ��������: spcReturnTestTelemetryData
** ��������: �ӻ������������͵����������������
** �䡡��  : pvArg�������߳�ʱ����Ĳ���
** �䡡��  :
** ������Ϣ: ÿ�� 50 �ֽ� = 1����֡(5B) + 6���м�֡(7B) + 1��β֡(3B)
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
VOID  spcReturnTestTelemetryData (INT  iFd, UCHAR  *pucBuf, INT  iDataLen)
{
    CAN_FRAME   canframe;
    ssize_t     stLen;
    ssize_t     stFrameNum;
    UCHAR       ucCnt = 0;
    INT         iIndex;

    if (iDataLen > SENDBUFLEN) {
        printf("data too long\n");
        return;
    }

    memset(_G_ucSendBuf, 0, SENDBUFLEN);
    memcpy(_G_ucSendBuf, pucBuf, iDataLen);                             /*  �������ݵ����ػ�������      */

    /*
     * �����֡����
     */
    canframe.CAN_bRtr       = 0;                                        /*  ��Զ��֡                    */
    canframe.CAN_bExtId     = 0;                                        /*  ����չ֡                    */
    canframe.CAN_uiId       = 0x431;
    canframe.CAN_ucLen      = 8;

    canframe.CAN_ucData[0]  = 0x01;
    canframe.CAN_ucData[1]  = 8;
    canframe.CAN_ucData[2]  = 0x1B;

    memcpy(&canframe.CAN_ucData[3], _G_ucSendBuf + ucCnt, 5);           /*  ��֡��Ч�������ݳ���Ϊ5�ֽ� */
    ucCnt += 5;

    stLen = write(iFd, &canframe, sizeof(CAN_FRAME));
    ioctl(iFd, CAN_DEV_STARTUP, 0);

    stFrameNum = stLen / sizeof(CAN_FRAME);

    if (stFrameNum != 1) {
        printf("failed to send can frame!, abort sending\n");
        return;
    }

    /*
     * ����м�֡����
     */
    canframe.CAN_uiId = 0x432;

    for (iIndex = 2; iIndex <= 7; iIndex++) {
        canframe.CAN_ucData[0] = iIndex;

        memcpy(&canframe.CAN_ucData[1], _G_ucSendBuf + ucCnt, 7);
        ucCnt += 7;

        stLen = write(iFd, &canframe, sizeof(CAN_FRAME));
        ioctl(iFd, CAN_DEV_STARTUP, 0);

        stFrameNum = stLen / sizeof(CAN_FRAME);

        if (stFrameNum != 1) {
            printf("failed to send can frame!, abort sending\n");
            return;
        }
    }

    /*
     * ���β֡����
     */
    canframe.CAN_uiId      = 0x433;
    canframe.CAN_ucLen     = 4;
    canframe.CAN_ucData[0] = 8;

    memcpy(&canframe.CAN_ucData[1], _G_ucSendBuf + ucCnt, 3);           /*  β֡��Ч���ݳ����� 3 �ֽ�   */

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
** ��������: spcMasterRequestTestData
** ��������: �ӻ������������͵����������������
** �䡡��  : pvArg�������߳�ʱ����Ĳ���
** �䡡��  :
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static VOID  *spcMasterRequestTestData(INT  iFd)
{
    UCHAR             ucBuf[50] = {0};
    UCHAR             ucIndex;
    UINT32            uiStartTime;
    CHAR             *pcRetData = "SpacechainSupportQtumMultisigTransactions";

    ioctl(iFd, FIOFLUSH, 0);

    uiStartTime = (UINT32)(Lw_Time_Get() / Lw_Time_GetFrequency());

    memcpy(ucBuf, pcRetData, strlen(pcRetData) + 1);

    ucIndex = 42;
    ucBuf[ucIndex++]  = uiStartTime & 0xFF;
    ucBuf[ucIndex++]  = uiStartTime & 0xFF00;
    ucBuf[ucIndex++]  = uiStartTime & 0xFF0000;
    ucBuf[ucIndex++]  = uiStartTime & 0xFF000000;

    ucBuf[ucIndex++]  = _G_usRestartCnt & 0xFF;
    ucBuf[ucIndex++]  = _G_usRestartCnt & 0xFF00;
    ucBuf[ucIndex++]  = _G_usRestartCnt & 0xFF0000;
    ucBuf[ucIndex++]  = _G_usRestartCnt & 0xFF000000;

    spcReturnTestTelemetryData (iFd, ucBuf, 50);

    /*
     * �ȴ�����ȫ���������
     */
    while (1) {
        INT  iNum;

        if (ioctl(iFd, FIONWRITE, &iNum)) {
            printf("error");
            break;
        }

        if (iNum == 0) {
            break;
        }
    }

    ioctl(iFd, CAN_DEV_STARTUP, 0);

    return  (NULL);
}
/*********************************************************************************************************
** ��������: spcSlaveReturnRcvDataNum
** ��������: �ӻ����ص�ǰ�Ѿ����յ��������ֽ���
** �䡡��  : �����߳�ʱ����Ĳ���
** �䡡��  :
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static VOID  *spcSlaveReturnRcvDataNum(INT  iFd, UCHAR  ucDataNum)
{
    CAN_FRAME   canframe;
    ssize_t     stLen;
    ssize_t     stFrameNum;

    ioctl(iFd, FIOFLUSH, 0);

    /*
     * ���֡����
     */
    canframe.CAN_bRtr       = 0;                                        /*  ��Զ��֡                    */
    canframe.CAN_bExtId     = 0;                                        /*  ����չ֡                    */
    canframe.CAN_uiId       = 0x42C;
    canframe.CAN_ucLen      = 1;

    canframe.CAN_ucData[0]  = ucDataNum;

    stLen = write(iFd, &canframe, sizeof(CAN_FRAME));
    ioctl(iFd, CAN_DEV_STARTUP, 0);

    stFrameNum = stLen / sizeof(CAN_FRAME);

    if (stFrameNum != 1) {
        printf("failed to send can frame!, abort sending\n");
        return  (LW_NULL);
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

    return  (NULL);
}
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
** ������Ϣ: ��Ҫע�������ַ����ַ�����������������Լ� ucOutData �ڵ��õ�ʱ����Ҫ���һ��
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
            /*
             * ��������һ���ֽڵĸ���λ
             */
            *(ucOutData + iOutIndex) |= (spcCharToHex(*(cData + iInIndex)) << 4);
        } else {
            /*
             * ż������һ���ֽڵĵ���λ
             */
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
    UCHAR  *ucData    = ucInData;
    INT     iInIndex  = 0;
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
** ��������: spcRcvDataToCmdString
** ��������: �ѽ��յ���ѹ������ת����ǩ��ʱִ�е��ַ�������
** �䡡��  : ucDest���洢����Ļ�����
**         : ucSrc �����յ�ѹ������
**         : iLen  ��ucDest ����������
** �䡡��  : ERROR_CODE
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
INT  spcRcvDataToCmdString (CHAR  *ucDest, INT  iLen, UCHAR  *ucSrc)
{
    INT               ret;
    INT               iIndex     = 0;
    INT               iTxidIndex = 2;
    ProtocolDataHead *pData      = (ProtocolDataHead *)ucSrc;

    memset(ucDest, '\0', iLen);

    if (pData->ucStartFlag == 0xba) {                                   /*  ���ױ�������                */
        memcpy(ucDest + iIndex, SIGCMDSTR, strlen(SIGCMDSTR));
        iIndex += strlen(SIGCMDSTR);
        ret = spcHexToString(ucDest + iIndex, &pData->ucData[0],
                            pData->usDataLong & 0x7FFF, pData->usDataLong & 0x8000);
        iIndex += ret;
        pData = (ProtocolDataHead *)((UCHAR *)pData + (3 + (pData->usDataLong & 0x7FFF)));
    }

    if (pData->ucStartFlag == 0xca) {                                   /*  txid ��������               */
        UCHAR     i;
        INT       iFd;
        ssize_t   ssReadNum;
        CHAR      cPrivKeyIndex[6 + strlen(PRIVKEYFILEDIC)] = {0};

        iTxidIndex = 1;
        memcpy(ucDest + iIndex, STARTSTR, strlen(STARTSTR));
        iIndex += strlen(STARTSTR);

        for (i = 0; i < pData->ucData[0]; i++) {
            /*
             * TXID
             */
            memcpy(ucDest + iIndex, TXIDSTR, strlen(TXIDSTR));
            iIndex += strlen(TXIDSTR);
            ret = spcHexToString(ucDest + iIndex, &pData->ucData[iTxidIndex + 1],
                              pData->ucData[iTxidIndex] & 0x7F, pData->ucData[iTxidIndex] & 0x80);
            iIndex += ret;
            iTxidIndex += (1 + (pData->ucData[iTxidIndex]  & 0x7F));

            /*
             * VOUT
             */
            memcpy(ucDest + iIndex, VOUTSTR, strlen(VOUTSTR));
            iIndex += strlen(VOUTSTR);

            sprintf(ucDest + iIndex, "%d", pData->ucData[iTxidIndex]);
            do {
                iIndex += 1;
                pData->ucData[iTxidIndex] /= 10;
            } while (pData->ucData[iTxidIndex] > 0);
            iTxidIndex += 1;

            /*
             * scriptPubKey
             */
            memcpy(ucDest + iIndex, PUBKEYSTR, strlen(PUBKEYSTR));
            iIndex += strlen(PUBKEYSTR);
            ret = spcHexToString(ucDest + iIndex, &pData->ucData[iTxidIndex + 2],
                              (pData->ucData[iTxidIndex] | (pData->ucData[iTxidIndex + 1] << 8)) & 0x7FFF,
                              pData->ucData[iTxidIndex + 1] & 0x80);
            iIndex += ret;
            iTxidIndex += (2 + ((pData->ucData[iTxidIndex] | (pData->ucData[iTxidIndex + 1] << 8)) & 0x7FFF));

            /*
             * redeemScript
             */
            memcpy(ucDest + iIndex, REDEEMSCRIPTSTR, strlen(REDEEMSCRIPTSTR));
            iIndex += strlen(REDEEMSCRIPTSTR);
            ret = spcHexToString(ucDest + iIndex, &pData->ucData[iTxidIndex + 2],
                              (pData->ucData[iTxidIndex] | (pData->ucData[iTxidIndex + 1] << 8)) & 0x7FFF,
                              pData->ucData[iTxidIndex + 1] & 0x80);
            iIndex += ret;
            iTxidIndex += (2 + ((pData->ucData[iTxidIndex] | (pData->ucData[iTxidIndex + 1] << 8)) & 0x7FFF));

            /*
             * ENDENDSTR
             */
            memcpy(ucDest + iIndex, ENDSTR, strlen(ENDSTR));
            iIndex += strlen(ENDSTR);
        }

        iIndex -= 1;                                                    /*  ȥ�� ENDSTR �е� ','        */
        memcpy(ucDest + iIndex, ENDENDSTR, strlen(ENDENDSTR));
        iIndex += strlen(ENDENDSTR);

        memcpy(ucDest + iIndex, PRIVKEYSTARTSTR, strlen(PRIVKEYSTARTSTR));
        iIndex += strlen(PRIVKEYSTARTSTR);

        /*
         * ��ȡ����ֵ��Ȼ���ļ����ҵ���Ӧ����Կ��ƴ�ӵ�������
         */
        memcpy(cPrivKeyIndex, PRIVKEYFILEDIC, strlen(PRIVKEYFILEDIC));
        sprintf(cPrivKeyIndex + strlen(PRIVKEYFILEDIC), "%d", pData->ucData[iTxidIndex] |
                                                             (pData->ucData[iTxidIndex + 1] << 8));
        iFd = open(cPrivKeyIndex, O_RDONLY, 0666);
        if (iFd < 0) {
            printf("failed to open %s!\n", cPrivKeyIndex);
            return (-1);
        }

        lseek(iFd, 0, SEEK_SET);
        ssReadNum = read(iFd, ucDest + iIndex, PRIVKEYLEN);
        if (ssReadNum != PRIVKEYLEN) {
            printf("read private key %s file failed\n", cPrivKeyIndex);
            return (-1);
        }
        close(iFd);

        iIndex     += PRIVKEYLEN;
        iTxidIndex += 2;
        memcpy(ucDest + iIndex, PRIVKEYENDSTR, strlen(PRIVKEYENDSTR));
    }

    return ((pData->ucData[iTxidIndex] == 0x1a) ? 0 : -1);
}
/*********************************************************************************************************
** ��������: spcProcessingJsonData
** ��������: ���� json ����
** �䡡��  : cJsonStr ��json �ַ�������
**         : sOutLen  ��ucOutData ����������
**         : sLen     ��cJsonStr �ַ�������
** �䡡��  : ucOutData��ѹ����� hex �ֶ�����
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
INT  spcProcessingJsonData (UCHAR  *ucOutData, size_t  sOutLen, CHAR  *cJsonStr, size_t  stLen)
{
    UINT16          sHexLong;
    INT             sRet;
    INT             iIndex;
    std::string     sHex;
    std::string     sComplete;
    std::string     strJson     = cJsonStr;
    Json::Value     tempVal;
    Json::Reader   *pJsonParser = new Json::Reader(Json::Features::strictMode());

    if(!pJsonParser->parse(strJson, tempVal))
    {
        return (-1);
    }

    sHex = tempVal["hex"].asString();
    sComplete = tempVal["complete"].asString();

    if (!memcmp(sComplete.c_str(), "true", strlen("true"))) {
        sRet = spcStringToHex (ucOutData + 3, (CHAR *)sHex.c_str(), sHex.length());
    } else {
        if (stLen > sOutLen) {
            stLen = sOutLen;
        }
        memcpy(ucOutData, cJsonStr, stLen);
        spcStringStrip((CHAR *)ucOutData, sOutLen);                     /*  ǩ��ʧ�ܣ���ʽ�����ݲ�����  */
        return (-1);
    }

    iIndex = 0;
    ucOutData[iIndex++] = 0xc1;

    sHexLong = (sRet / 2) + (sRet % 2);
    if (sRet % 2) {
        sHexLong |= 0x8000;
    }

    memcpy(&ucOutData[iIndex], &sHexLong, sizeof(sHexLong));
    iIndex = iIndex + 2 + (sHexLong & 0x7FFF);

    ucOutData[iIndex] = 0x1c;

    delete pJsonParser;

    return (1);
}
/*********************************************************************************************************
** ��������: spcImportPrivKey
** ��������: ������Ǯ��˽Կ
** �䡡��  : cPrivKey  ��˽Կ�ַ�����ַ
**         : ucPrivNum ��˽Կ����
** �䡡��  : ��������ַ������ȣ���'\0'��������洢���� _G_ucAllSendBuf ��
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
INT  spcImportPrivKey (CHAR  *cPrivKey, UCHAR  ucPrivNum)
{
    UCHAR     ucIndex;
    CHAR      cKey[IMPORTPRIVKEYLEN + PRIVKEYLEN + 1];
    FILE     *sigResult;
    size_t    sSigDataNum = 0;

    cKey[IMPORTPRIVKEYLEN + PRIVKEYLEN] = '\0';
    memcpy(cKey, IMPORTPRIVKEYSTR, IMPORTPRIVKEYLEN);

    for (ucIndex = 0; ucIndex < ucPrivNum; ucIndex++) {
        memcpy(cKey + IMPORTPRIVKEYLEN, cPrivKey + (ucIndex * PRIVKEYLEN), PRIVKEYLEN);

        printf("\n%s\n", cKey);

        sigResult = popen((const CHAR *)cKey, "r");
        if(sigResult == NULL){
            return (-1);
        }

        if ((sizeof(_G_ucAllSendBuf) - sSigDataNum) > 0) {
            sSigDataNum += fread(_G_ucAllSendBuf + sSigDataNum, sizeof(CHAR),
                                sizeof(_G_ucAllSendBuf) - sSigDataNum, sigResult);
                                                                        /*  ����˽Կ�������Ԥ�� 100B   */
        }

        pclose(sigResult);
//        sleep(1);                                                       /*  ÿ����һ����Կ�ȴ�1��       */
    }

    return (sSigDataNum + ucIndex);
}
/*********************************************************************************************************
** ��������: spcSlaveProcessRcvData
** ��������: �ӻ�������������յ�������
** �䡡��  : canframe��һ�� can ֡����
** �䡡��  :
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
void  spcSlaveProcessRcvData(INT  iFd, CAN_FRAME  canframe)
{
    if ((canframe.CAN_uiId == 0x30) && (canframe.CAN_ucLen == 1) && (canframe.CAN_ucData[0] == 0x1A)) {
        /*
         * ������ʽң��������֤ͨ��������ң������
         */
        printf("request Telemetry Data\n");

        spcMasterRequestTelemetryData(iFd);

    } else if ((canframe.CAN_uiId == 0x30) && (canframe.CAN_ucLen == 1) && (canframe.CAN_ucData[0] == 0x1B)) {
        /*
         * �������ң��������֤ͨ��������ң������
         */
        printf("id = 0x%x, len = %d, data = %02x\n", canframe.CAN_uiId, canframe.CAN_ucLen, canframe.CAN_ucData[0]);

        spcMasterRequestTestData(iFd);

    } else if ((canframe.CAN_uiId == 0x24) && (canframe.CAN_ucLen == 8)
            && (memcmp(canframe.CAN_ucData, _G_ucData, CAN_MAX_DATA) == 0)) {
        /*
         * �յ���λ can ����������
         */
        if (ioctl(iFd, CAN_DEV_REST_CONTROLLER, NULL)) {
            printf("Restart CAN Controler failed.\n");
            return;
        }

        _G_uiAllSendDataIndex    = 0;
        _G_uiAllRcvDataIndex     = 0;

        printf("Reset the can's controller Success\n");

    } else if ((canframe.CAN_uiId == 0x18) && (canframe.CAN_ucLen == 6)) {
        /*
         * �յ�ͬ��ʱ������֡
         */
        uint32_t            uiTime;
        struct timespec     tv;

        uiTime = (time_t)be32toh(*((uint32_t *)&canframe.CAN_ucData[0]));

        /*
         * ��ǰʱ�� = 2018 �� 1 �� 1 �� 0 ʱ 0 �� 0 �루1514736000�� + ��λ�����͵�ʱ������
         */
        tv.tv_sec  = (time_t)(1514736000) + uiTime;
        tv.tv_nsec = 0;

        lib_clock_settime(CLOCK_REALTIME, &tv);
        API_SysToRtc();

    } else if (canframe.CAN_uiId == 0x2C) {
        /*
         * ��֡ע�����ݣ�ֱ�Ӵ�ӡ
         */
        printf("id = 0x%x, len = %d, data = %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x\n",
                canframe.CAN_uiId,
                canframe.CAN_ucLen,
                canframe.CAN_ucData[0],
                canframe.CAN_ucData[1],
                canframe.CAN_ucData[2],
                canframe.CAN_ucData[3],
                canframe.CAN_ucData[4],
                canframe.CAN_ucData[5],
                canframe.CAN_ucData[6],
                canframe.CAN_ucData[7]);

        /*
         * �� OBC �����غɰ�һ�����յ��������ֽ���
         */
        spcSlaveReturnRcvDataNum(iFd, canframe.CAN_ucLen);

    } else if ((canframe.CAN_uiId == 0x2D) || (canframe.CAN_uiId == 0x2E) || (canframe.CAN_uiId == 0x2F)) {
        /*
         * ��֡ע�����ݣ�ÿ�ν��� 50 �ֽڣ�1����֡(5B) + 6���м�֡(7B) + 1��β֡(3B)
         */
        static UCHAR ucRcvNum = 0;
        static UCHAR ucIndex  = 0;

        if (canframe.CAN_uiId == 0x2D) {                                /*  �յ���֡����                */
            ucRcvNum = 0;
            ucIndex  = 0;

            memset(_G_ucRcvBuf, 0, RCVBUFLEN);
            memcpy(_G_ucRcvBuf + ucIndex, &canframe.CAN_ucData[3], 5);

            ucIndex  += 5;
            ucRcvNum += (canframe.CAN_ucLen - 3);                       /*  �ۼӽ��������ֽڼ���ֵ      */

        } else if (canframe.CAN_uiId == 0x2E) {

            if (ucIndex > (RCVBUFLEN - 7)) {                            /*  �յ��м�֡����              */
                return;
            }

            memcpy(_G_ucRcvBuf + ucIndex, &canframe.CAN_ucData[1], 7);

            ucIndex  += 7;
            ucRcvNum += (canframe.CAN_ucLen - 1);                       /*  �ۼӽ��������ֽڼ���ֵ      */

        } else if (canframe.CAN_uiId == 0x2F) {
            /*
             * �յ�β֡����
             */
            if (ucIndex > (RCVBUFLEN - 3)) {
                return;
            }

            memcpy(_G_ucRcvBuf + ucIndex, &canframe.CAN_ucData[1], 3);

            ucIndex  += 3;
            ucRcvNum += (canframe.CAN_ucLen - 1);                       /*  �ۼӽ��������ֽڼ���ֵ      */

            /*
             * �����������Ѿ����յ��������ֽ���
             */
            spcSlaveReturnRcvDataNum(iFd, ucRcvNum);

            memcpy(_G_ucAllRcvBuf + _G_uiAllRcvDataIndex, _G_ucRcvBuf, RCVBUFLEN);
            _G_uiAllRcvDataIndex += RCVBUFLEN;                          /*  50B �������� 500B ������    */

            if (_G_uiAllRcvDataIndex >= ALLRCVBUFLEN) {                 /*  �������ݴﵽ�� 500 �ֽ�     */
                ProtocolDataHead *data = (ProtocolDataHead *)_G_ucAllRcvBuf;

                _G_uiAllRcvDataIndex = 0;

                if ((data->ucStartFlag == 0xa1) && (data->ucData[data->usDataLong - 3] == 0x1a)) {
                                                                        /*  ���ݺϷ����㲢����ǩ�����  */
                    FILE           *sigResult;
                    FILE           *logSave;
                    void           *cSigDataBuf;
                    size_t          sSigDataNum;

                    spcRcvDataToCmdString((CHAR *)_G_ucAllSendBuf, ALLSENDBUFLEN, data->ucData);
                                                                        /*  ������յ���500�ֽ�         */
                    sigResult = popen((const CHAR *)_G_ucAllSendBuf, "r");
                    if(sigResult == NULL){
                        return;
                    }

                    memset(_G_ucAllSendBuf, '\0', ALLSENDBUFLEN);
                    sSigDataNum = fread(_G_ucAllSendBuf, sizeof(CHAR), sizeof(_G_ucAllSendBuf), sigResult);
                                                                        /*  ��ȡjson��ʽ�Ĳ������      */
                    pclose(sigResult);

                    cSigDataBuf = malloc(sSigDataNum);
                    memcpy(cSigDataBuf, _G_ucAllSendBuf, sSigDataNum);
                    memset(_G_ucAllSendBuf, '\0', ALLSENDBUFLEN);
                    spcProcessingJsonData(_G_ucAllSendBuf, ALLSENDBUFLEN, (CHAR *)cSigDataBuf, sSigDataNum);
                                                                        /*  ����json���ݲ�����ѹ������  */
                    free(cSigDataBuf);

                    {
                        INT   iIndex;

                        printf("\n");
                        for (iIndex = 0; iIndex < ALLSENDBUFLEN; iIndex++) {
                            printf("%02x", _G_ucAllSendBuf[iIndex]);
                        }
                        printf("\n");
                    }

                    logSave = fopen( "/root/spc_log.txt", "a");         /*  ��¼ִ�е�ÿ������ļ�    */
                    if(logSave == NULL){
                        printf("open /root/spc_log.txt file failed\n");
                           return;
                    }

                    fwrite(&data->ucData[data->usDataLong], 1, sizeof(data->ucData[data->usDataLong]), logSave);
                    fwrite(_G_ucAllSendBuf, 1, sizeof(_G_ucAllSendBuf), logSave);

                    fclose(logSave);

                } else if ((data->ucStartFlag == 0xb1) && (data->ucData[data->usDataLong] == 0x1b)) {

                    if ((data->ucData[0] * PRIVKEYLEN) == (data->usDataLong - 1)) {
                                                                        /*  ���ݺϷ���ִ��˽Կ�������  */
                        spcImportPrivKey((CHAR *)&data->ucData[1], data->ucData[0]);
                    } else {
                                                                        /*  NOP                         */
                    }

                } else {
                                                                        /*  NOP                         */
                }
            }

            {
                /*
                 * ��ӡ���յ�������
                 */
                INT  i;
                printf("\n");
                for (i = 0; i < RCVBUFLEN; i++) {
                    printf("%02x", _G_ucRcvBuf[i]);
                }
                printf("\n");
            }



        } else {                                                        /*  NOP                         */

        }
    } else {
        printf("The data is error\n");
    }

    return ;
}
/*********************************************************************************************************
** ��������: spcPthreadRecv
** ��������: CAN �������� pthread �����߳�
** �䡡��  : pvArg         �̲߳���
** �䡡��  : �̷߳���ֵ
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static VOID  *spcPthreadRecv(VOID  *pvArg)
{
    INT         iFd;
    CAN_FRAME   canframe;
    ssize_t     stLen;
    ssize_t     stFrameNum;
    CHAR       *devname = (CHAR *)pvArg;

    memset(&canframe, 0, sizeof(CAN_FRAME));

    iFd = open(devname, O_RDWR, 0666);
    if (iFd < 0) {
        printf("failed to open %s!\n", devname);
        return  (LW_NULL);
    }
    printf("Successed to open %s!\n", devname);

    if (ioctl(iFd, CAN_DEV_SET_BAUD, CANBUAD)) {
        printf("set baud to %d error.\n", CANBUAD);
        close (iFd);
        return  (LW_NULL);
    }

    printf("start %s recv test with baud %d.\n", devname, CANBUAD);

    while (1) {
        stLen = read(iFd, &canframe, sizeof(CAN_FRAME));
        stFrameNum = stLen / sizeof(CAN_FRAME);

        if (stFrameNum != 1) {                                          /*  ��������ʧ��                */
            printf("failed to recv can frame, abort recving!\n");
            continue;
        } else {                                                        /*  �������ݳɹ��������������  */
            spcSlaveProcessRcvData(iFd, canframe);
        }
    }

    close(iFd);

    return  (NULL);
}
/*********************************************************************************************************
** ��������: spcSlaveProtocolRecvStart
** ��������: CAN ����
** �䡡��  : NONE
** �䡡��  : ERROR_CODE
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
INT  spcSlaveProtocolRecvStart (char  *pcdevname)
{
    pthread_t   tid_recv;
    INT         iError = ERROR_NONE;

    iError = pthread_create(&tid_recv, NULL, spcPthreadRecv, pcdevname);
    pthread_join(tid_recv, NULL);

    return  (iError == 0 ? ERROR_NONE : PX_ERROR);
}
/*********************************************************************************************************
** ��������: spcRestartNum
** ��������: �����豸��������
** �䡡��  : NONE
** �䡡��  : ERROR_CODE
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
INT  spcRestartNum (VOID)
{
    INT      iFramFd;

    iFramFd = open("/dev/fram", O_RDWR, 0666);
    if (iFramFd < 0) {
        printf("open /dev/fram fail\n");
    }

    ioctl(iFramFd, 0, 0);
    read(iFramFd, &_G_usRestartCnt, 4);

    _G_usRestartCnt++;

    write(iFramFd, &_G_usRestartCnt, 4);
    close(iFramFd);

    return (0);
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
INT  main (INT  argc, CHAR  *argv[])
{
    spcRestartNum();
    spcSlaveProtocolRecvStart((CHAR *)CANDEV);

    return  (0);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
