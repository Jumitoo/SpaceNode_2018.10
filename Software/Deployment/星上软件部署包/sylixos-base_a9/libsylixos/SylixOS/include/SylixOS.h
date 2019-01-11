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
** 文   件   名: SylixOS.h
**
** 创   建   人: Han.Hui (韩辉)
**
** 文件创建日期: 2006 年 12 月 13 日
**
** 描        述: 这是系统统一头文件。

** BUG
2007.09.25  加入内核级限制，为以后内核模块与用户模块分离做好准备.
2007.10.20  加入 C++ 兼容处理.
2009.10.14  加入 C++ run-time lib 层. 
            (注意, 所有的 CPP 文件必须第一个包含 SylixOS.h 头文件, 或者第一个单独包含 cpp_cpp.h)
2010.01.07  加入 posix 子系统.
*********************************************************************************************************/

#ifndef  __SYLIXOS_H
#define  __SYLIXOS_H

/*********************************************************************************************************
  注意如果不需要使用操作系统提供的 C 兼容库时, 请在引用此头文件前, 定义宏: __EXCLIB
  表示需要外部 C 库支持.
  
  定义宏: __EXCLIB_STDIO  表示需要外部 stdio  库支持.
          __EXCLIB_STDARG 表示需要外部 stdarg 库支持.
*********************************************************************************************************/

/*********************************************************************************************************
  C++
*********************************************************************************************************/

#ifdef __cplusplus
/*********************************************************************************************************
  内核头服务文件
*********************************************************************************************************/
#include "../SylixOS/cplusplus/include/cpp_cpp.h"                       /*  C++ run-time lib            */
extern "C" {
#endif                                                                  /*  __cplusplus                 */

/*********************************************************************************************************
  内核头服务文件
*********************************************************************************************************/

#include "../SylixOS/kernel/include/k_kernel.h"                         /*  LongWing 内核               */
#include "../SylixOS/system/include/s_system.h"                         /*  系统级                      */
#include "../SylixOS/mpi/include/mpi_mpi.h"                             /*  MPI 支持                    */
#include "../SylixOS/shell/include/ttiny_shell.h"                       /*  简易 shell 系统             */
#include "../SylixOS/fs/include/fs_fs.h"                                /*  FAT 文件系统                */
#include "../SylixOS/net/include/net_net.h"                             /*  网络接口                    */
#include "../SylixOS/posix/include/px_posix.h"                          /*  posix 子系统                */
#include "../SylixOS/symbol/include/sym_sym.h"                          /*  符号表                      */
#include "../SylixOS/loader/include/loader.h"                           /*  程序/模块装载器 (进程支持)  */

/*********************************************************************************************************
  可以去除相关中间件
*********************************************************************************************************/
#include "../SylixOS/appl/appl.h"                                       /*  应用中间件接口              */

#ifndef  __SYLIXOS_KERNEL                                               /*  是否是内核模块              */

#define  LW_API_EXEC(__class, __operate, __args)   \
         Lw_##__class##_##__operate## __args

#include "../SylixOS/api/Lw_Api_Kernel.h"                               /*  内核 API                    */
#include "../SylixOS/api/Lw_Api_System.h"                               /*  系统 API                    */
#include "../SylixOS/api/Lw_Api_Mp.h"                                   /*  多处理器 API                */
#include "../SylixOS/api/Lw_Api_Shell.h"                                /*  简易 shell 系统             */
#include "../SylixOS/api/Lw_Api_Net.h"                                  /*  网络接口                    */
#include "../SylixOS/api/Lw_Api_Fs.h"                                   /*  文件系统                    */
#include "../SylixOS/api/Lw_Api_Symbol.h"                               /*  符号表                      */
#include "../SylixOS/api/Lw_Api_Loader.h"                               /*  装载器                      */

/*********************************************************************************************************
  不对应用引出的内容处理
*********************************************************************************************************/

#undef LW_INLINE
#undef REGISTER

#else                                                                   /*  !__SYLIXOS_KERNEL           */

/*********************************************************************************************************
  驱动程序推荐使用 io.h 中的方法访问硬件寄存器.
*********************************************************************************************************/
#include "io.h"                                                         /*  io 空间访问操作             */
#endif                                                                  /*  !__SYLIXOS_KERNEL           */

#include "../SylixOS/api/Lw_Class_Kernel.h"                             /*  内核对象类型                */
#include "../SylixOS/api/Lw_Class_System.h"                             /*  系统对象类型                */
#include "../SylixOS/api/Lw_Class_Mp.h"                                 /*  多处理器对象类型            */

/*********************************************************************************************************
  C++
*********************************************************************************************************/

#ifdef __cplusplus
}
#endif                                                                  /*  __cplusplus                 */

#endif                                                                  /*  __SYLIXOS_H                 */
/*********************************************************************************************************
  END
*********************************************************************************************************/
