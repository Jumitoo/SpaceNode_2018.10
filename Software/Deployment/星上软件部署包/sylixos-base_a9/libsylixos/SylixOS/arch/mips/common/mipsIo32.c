/*********************************************************************************************************
**
**                                    中国软件开源组织
**
**                                   嵌入式实时操作系统
**
**                                SylixOS(TM)  LW : long wing
**
**                               Copyright All Rights Reserved
**
**--------------文件信息--------------------------------------------------------------------------------
**
** 文   件   名: mipsIo32.c
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2016 年 11 月 02 日
**
** 描        述: MIPS32 体系架构 I/O 端口读写函数库.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
#include "mips64.h"
/*********************************************************************************************************
  MIPS32 体系架构和 MIPS64 32 位模式才使用以下函数
*********************************************************************************************************/
#if LW_CFG_CPU_WORD_LENGHT == 32
/*********************************************************************************************************
  定义
*********************************************************************************************************/
#define __32BIT_SPACE        (4ULL * LW_CFG_GB_SIZE)                    /*  32 位空间                   */
/*********************************************************************************************************
  全局变量
*********************************************************************************************************/
static UINT64  _G_ui64MipsIoPortBase = 0ULL;                            /*  MIPS I/O 端口基地址         */
/*********************************************************************************************************
** 函数名称: archSetIoPortBase
** 功能描述: 设置 I/O 端口基地址
** 输　入  : ui64IoPortBase        I/O 端口基地址
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  archSetIoPortBase (UINT64  ui64IoPortBase)
{
    _G_ui64MipsIoPortBase = ui64IoPortBase;
}
/*********************************************************************************************************
  MIPS 处理器 I/O 端口读
*********************************************************************************************************/
/*********************************************************************************************************
** 函数名称: in8
** 功能描述: 读指定 I/O 端口的一个字节
** 输　入  : ulAddr        I/O 端口
** 输　出  : 一个字节数据
** 全局变量:
** 调用模块:
*********************************************************************************************************/
UINT8  in8 (addr_t  ulAddr)
{
    UINT64  ui64Addr = _G_ui64MipsIoPortBase + ulAddr;
    UINT8   ucVal;

    if (ui64Addr < __32BIT_SPACE) {
        ucVal = *(volatile UINT8 *)(addr_t)ui64Addr;
    } else {
        ucVal = mips64Read8(ui64Addr);
    }

    KN_IO_RMB();
    return  (ucVal);
}
/*********************************************************************************************************
** 函数名称: in16
** 功能描述: 读指定 I/O 端口的一个 16 位数据
** 输　入  : ulAddr        I/O 端口
** 输　出  : 一个 16 位数据
** 全局变量:
** 调用模块:
*********************************************************************************************************/
UINT16  in16 (addr_t  ulAddr)
{
    UINT64  ui64Addr = _G_ui64MipsIoPortBase + ulAddr;
    UINT16  usVal;

    if (ui64Addr < __32BIT_SPACE) {
        usVal = *(volatile UINT16 *)(addr_t)ui64Addr;
    } else {
        usVal = mips64Read16(ui64Addr);
    }

    KN_IO_RMB();
    return  (usVal);
}
/*********************************************************************************************************
** 函数名称: in32
** 功能描述: 读指定 I/O 端口的一个 32 位数据
** 输　入  : ulAddr        I/O 端口
** 输　出  : 一个 32 位数据
** 全局变量:
** 调用模块:
*********************************************************************************************************/
UINT32  in32 (addr_t  ulAddr)
{
    UINT64  ui64Addr = _G_ui64MipsIoPortBase + ulAddr;
    UINT32  uiVal;

    if (ui64Addr < __32BIT_SPACE) {
        uiVal = *(volatile UINT32 *)(addr_t)ui64Addr;
    } else {
        uiVal = mips64Read32(ui64Addr);
    }

    KN_IO_RMB();
    return  (uiVal);
}
/*********************************************************************************************************
** 函数名称: in64
** 功能描述: 读指定 I/O 端口的一个 64 位数据
** 输　入  : ulAddr        I/O 端口
** 输　出  : 一个 64 位数据
** 全局变量:
** 调用模块:
*********************************************************************************************************/
UINT64  in64 (addr_t  ulAddr)
{
    UINT64  ui64Addr = _G_ui64MipsIoPortBase + ulAddr;
    UINT64  ui64Val;

    if (ui64Addr < __32BIT_SPACE) {
        ui64Val = *(volatile UINT64 *)(addr_t)ui64Addr;
    } else {
        ui64Val = mips64Read64(ui64Addr);
    }

    KN_IO_RMB();
    return  (ui64Val);
}
/*********************************************************************************************************
  MIPS 处理器 I/O 端口写
*********************************************************************************************************/
/*********************************************************************************************************
** 函数名称: out8
** 功能描述: 写一个字节到指定 I/O 端口
** 输　入  : ucData        一个字节数据
**           ulAddr        I/O 端口
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  out8 (UINT8  ucData, addr_t  ulAddr)
{
    UINT64  ui64Addr = _G_ui64MipsIoPortBase + ulAddr;

    KN_IO_WMB();
    if (ui64Addr < __32BIT_SPACE) {
        *(volatile UINT8 *)ulAddr = ucData;
    } else {
        mips64Write8(ucData, ui64Addr);
    }
}
/*********************************************************************************************************
** 函数名称: out16
** 功能描述: 写一个 16 位数据到指定 I/O 端口
** 输　入  : usData        一个 16 位数据
**           ulAddr        I/O 端口
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  out16 (UINT16  usData, addr_t  ulAddr)
{
    UINT64  ui64Addr = _G_ui64MipsIoPortBase + ulAddr;

    KN_IO_WMB();
    if (ui64Addr < __32BIT_SPACE) {
        *(volatile UINT16 *)ulAddr = usData;
    } else {
        mips64Write16(usData, ui64Addr);
    }
}
/*********************************************************************************************************
** 函数名称: out32
** 功能描述: 写一个 32 位数据到指定 I/O 端口
** 输　入  : uiData        一个 32 位数据
**           ulAddr        I/O 端口
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  out32 (UINT32  uiData, addr_t  ulAddr)
{
    UINT64  ui64Addr = _G_ui64MipsIoPortBase + ulAddr;

    KN_IO_WMB();
    if (ui64Addr < __32BIT_SPACE) {
        *(volatile UINT32 *)ulAddr = uiData;
    } else {
        mips64Write32(uiData, ui64Addr);
    }
}
/*********************************************************************************************************
** 函数名称: out64
** 功能描述: 写一个 64 位数据到指定 I/O 端口
** 输　入  : u64Data       一个 64 位数据
**           ulAddr        I/O 端口
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  out64 (UINT64  u64Data, addr_t  ulAddr)
{
    UINT64  ui64Addr = _G_ui64MipsIoPortBase + ulAddr;

    KN_IO_WMB();
    if (ui64Addr < __32BIT_SPACE) {
        *(volatile UINT64 *)ulAddr = u64Data;
    } else {
        mips64Write64(u64Data, ui64Addr);
    }
}
/*********************************************************************************************************
  MIPS 处理器 I/O 端口连续读 (数据来自单个地址)
*********************************************************************************************************/
/*********************************************************************************************************
** 函数名称: ins8
** 功能描述: 连续 8 位读指定 I/O 端口
** 输　入  : ulAddr        I/O 端口
**           pvBuffer      缓冲区
**           stCount       次数
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  ins8 (addr_t  ulAddr, PVOID  pvBuffer, size_t  stCount)
{
    REGISTER UINT8  *pucBuffer = (UINT8 *)pvBuffer;

    while (stCount > 0) {
        *pucBuffer++ = in8(ulAddr);
        stCount--;
    }
}
/*********************************************************************************************************
** 函数名称: ins16
** 功能描述: 连续 16 位读指定 I/O 端口
** 输　入  : ulAddr        I/O 端口
**           pvBuffer      缓冲区
**           stCount       次数
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  ins16 (addr_t  ulAddr, PVOID  pvBuffer, size_t  stCount)
{
    REGISTER UINT16  *pusBuffer = (UINT16 *)pvBuffer;

    while (stCount > 0) {
        *pusBuffer++ = in16(ulAddr);
        stCount--;
    }
}
/*********************************************************************************************************
** 函数名称: ins32
** 功能描述: 连续 32 位读指定 I/O 端口
** 输　入  : ulAddr        I/O 端口
**           pvBuffer      缓冲区
**           stCount       次数
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  ins32 (addr_t  ulAddr, PVOID  pvBuffer, size_t  stCount)
{
    REGISTER UINT32  *puiBuffer = (UINT32 *)pvBuffer;

    while (stCount > 0) {
        *puiBuffer++ = in32(ulAddr);
        stCount--;
    }
}
/*********************************************************************************************************
** 函数名称: ins64
** 功能描述: 连续 64 位读指定 I/O 端口
** 输　入  : ulAddr        I/O 端口
**           pvBuffer      缓冲区
**           stCount       次数
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  ins64 (addr_t  ulAddr, PVOID  pvBuffer, size_t  stCount)
{
    REGISTER UINT64  *pu64Buffer = (UINT64 *)pvBuffer;

    while (stCount > 0) {
        *pu64Buffer++ = in64(ulAddr);
        stCount--;
    }
}
/*********************************************************************************************************
  MIPS 处理器 I/O 端口连续写 (数据写入单个地址)
*********************************************************************************************************/
/*********************************************************************************************************
** 函数名称: outs8
** 功能描述: 连续 8 位写指定 I/O 端口
** 输　入  : ulAddr        I/O 端口
**           pvBuffer      缓冲区
**           stCount       次数
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  outs8 (addr_t  ulAddr, CPVOID  pvBuffer, size_t  stCount)
{
    REGISTER const UINT8  *pucBuffer = (const UINT8 *)pvBuffer;

    while (stCount > 0) {
        out8(*pucBuffer++, ulAddr);
        stCount--;
    }
}
/*********************************************************************************************************
** 函数名称: outs16
** 功能描述: 连续 16 位写指定 I/O 端口
** 输　入  : ulAddr        I/O 端口
**           pvBuffer      缓冲区
**           stCount       次数
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  outs16 (addr_t  ulAddr, CPVOID  pvBuffer, size_t  stCount)
{
    REGISTER const UINT16  *pusBuffer = (const UINT16 *)pvBuffer;

    while (stCount > 0) {
        out16(*pusBuffer++, ulAddr);
        stCount--;
    }
}
/*********************************************************************************************************
** 函数名称: outs32
** 功能描述: 连续 32 位写指定 I/O 端口
** 输　入  : ulAddr        I/O 端口
**           pvBuffer      缓冲区
**           stCount       次数
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  outs32 (addr_t  ulAddr, CPVOID  pvBuffer, size_t  stCount)
{
    REGISTER const UINT32  *puiBuffer = (const UINT32 *)pvBuffer;

    while (stCount > 0) {
        out32(*puiBuffer++, ulAddr);
        stCount--;
    }
}
/*********************************************************************************************************
** 函数名称: outs64
** 功能描述: 连续 64 位写指定 I/O 端口
** 输　入  : ulAddr        I/O 端口
**           pvBuffer      缓冲区
**           stCount       次数
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  outs64 (addr_t  ulAddr, CPVOID  pvBuffer, size_t  stCount)
{
    REGISTER const UINT64  *pu64Buffer = (const UINT64 *)pvBuffer;

    while (stCount > 0) {
        out64(*pu64Buffer++, ulAddr);
        stCount--;
    }
}

#endif                                                                  /*  LW_CFG_CPU_WORD_LENGHT == 64*/
/*********************************************************************************************************
  END
*********************************************************************************************************/

