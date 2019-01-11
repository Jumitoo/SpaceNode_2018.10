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
** 文   件   名: inlAddress.h
**
** 创   建   人: Han.Hui (韩辉)
**
** 文件创建日期: 2007 年 08 月 29 日
**
** 描        述: 这是内存地址相关判断
*********************************************************************************************************/

#ifndef __INLADDRESS_H
#define __INLADDRESS_H

/*********************************************************************************************************
  判断内存是否对齐
*********************************************************************************************************/

static LW_INLINE BOOL  _Addresses_Is_Aligned (PVOID  pvAddress)
{
    return  (ALIGNED(pvAddress, sizeof(LW_STACK)));
}

#endif                                                                  /*  __INLADDRESS_H              */
/*********************************************************************************************************
  END
*********************************************************************************************************/
