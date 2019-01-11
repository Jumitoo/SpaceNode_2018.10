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
** 文   件   名: cpu_cfg_x86.h
**
** 创   建   人: Han.Hui (韩辉)
**
** 文件创建日期: 2015 年 11 月 20 日
**
** 描        述: x86 CPU 类型与功能配置.
*********************************************************************************************************/

#ifndef __CPU_CFG_X86_H
#define __CPU_CFG_X86_H

/*********************************************************************************************************
  CPU 体系结构
*********************************************************************************************************/

#define LW_CFG_CPU_ARCH_X86             1                               /*  CPU 架构                    */
#if defined(__x86_64__)
#define LW_CFG_CPU_ARCH_FAMILY          "x86-64(R)"                     /*  x64 family                  */
#else
#define LW_CFG_CPU_ARCH_FAMILY          "x86(R)"                        /*  x86 family                  */
#endif

/*********************************************************************************************************
  SMT 同步多线程调度优化
*********************************************************************************************************/

#define LW_CFG_CPU_ARCH_SMT             1                               /*  同步多线程优化              */

/*********************************************************************************************************
  CACHE LINE 对齐
*********************************************************************************************************/

#define LW_CFG_CPU_ARCH_CACHE_LINE      128                             /*  cache 最大行对齐属性        */

/*********************************************************************************************************
  CPU 体系结构配置
*********************************************************************************************************/

#define LW_CFG_CPU_X86_NO_BARRIER       0                               /*  不支持内存屏障指令          */
                                                                        /*  老式奔腾处理器 (1, 2, 3, 4) */
#define LW_CFG_CPU_X86_NO_PAUSE         0                               /*  不支持 PAUSE 指令           */
#define LW_CFG_CPU_X86_NO_HLT           0                               /*  不支持 HLT 指令             */

/*********************************************************************************************************
  CPU 字长与整型大小端定义
*********************************************************************************************************/

#define LW_CFG_CPU_ENDIAN               0                               /*  0: 小端  1: 大端            */
#if defined(__x86_64__)
#define LW_CFG_CPU_WORD_LENGHT          64                              /*  CPU 字长                    */
#else
#define LW_CFG_CPU_WORD_LENGHT          32                              /*  CPU 字长                    */
#endif

/*********************************************************************************************************
  浮点运算单元
*********************************************************************************************************/

#define LW_CFG_CPU_FPU_EN               1                               /*  CPU 是否拥有 FPU            */
#define LW_CFG_CPU_FPU_XSAVE_SIZE       1200                            /*  XSAVE & XRSTOR 上下文大小   */

/*********************************************************************************************************
  DSP 数字信号处理器
*********************************************************************************************************/

#define LW_CFG_CPU_DSP_EN               0                               /*  CPU 是否拥有 DSP            */

#endif                                                                  /*  __CPU_CFG_X86_H             */
/*********************************************************************************************************
  END
*********************************************************************************************************/
