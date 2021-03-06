/*--------------------------------------------------------------------------------------------------------------------------*/
/**
 * @file sre_callstack.h
 *
 * Copyright(C), 2008-2050, Huawei Tech. Co., Ltd. ALL RIGHTS RESERVED. \n
 *
 * 描述：硬件相关模块的对外头文件。 \n
 */

 /**@defgroup SRE_hw 硬件支持
   *@ingroup SRE_kernel
 */

#ifndef _SRE_CALLSTACK_H
#define _SRE_CALLSTACK_H

#include "sre_base.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */


/**
 * @ingroup SRE_callstack
 *  硬件相关信息错误码：获取函数调用栈传入的参数为NULL
 *
 * 值: 0x02003501
 *
 * 解决方案: 请确保传入的参数不为NULL
 */
#define OS_ERRNO_CALLSTACK_PTR_NULL                        SRE_ERRNO_OS_ERROR(OS_MID_CALLSTACK, 0x01)

/**
 * @ingroup SRE_callstack
 *  硬件相关信息错误码：获取函数调用栈传入的最大调用层数非法
 *
 * 值: 0x02003502
 *
 * 解决方案: 请确保传入的最大调用层数大于0且小于10
 */
#define OS_ERRNO_CALLSTACK_MAX_NUM_INVALID                    SRE_ERRNO_OS_ERROR(OS_MID_CALLSTACK, 0x02)


#if (OS_HARDWARE_PLATFORM == OS_CORTEX_AX)
/**
 * @ingroup  SRE_callstack
 * @brief 获取在线调用栈信息
 *
 * @par 描述:
 * 获取当前函数或者异常时函数的调用栈信息。
 *
 * @attention
 * <ul>
 * <li>只支持Cortex-AX平台。</li>
 * <li>支持普通调用(系统正常运行)和异常钩子调用(系统发生异常)，前者获取的为当前函数的调用栈信息，后者获取的为异常时函数的调用栈信息。</li>
 * <li>缓冲区存储各级函数PC的顺序为从最底层函数到最上层函数。</li>
 * </ul>
 *
 * @param  puwMaxNum [IN/OUT] 类型#UINT32 *，最大调用层数，输入/输出参数。输入：缓冲区puwPCList可容纳的元素个数；输出：实际解析出的函数调用层数。
 * @param  puwPCList [OUT] 类型#UINT32 *，存储函数PC信息的缓冲区首地址，用于填写各级调用函数的PC。
 *
 * @retval #OS_ERRNO_CALLSTACK_PTR_NULL         0x02003501，获取调用栈信息时，输入的参数指针为NULL。
 * @retval #OS_ERRNO_CALLSTACK_MAX_NUM_VALID    0x02003502，获取调用栈信息时，输入的最大调用层数等于0或者大于10。
 * @retval #SRE_OK                              0x00000000，获取调用栈信息成功。
 * @par 依赖:
 * <ul><li>sre_callstack.h：该接口声明所在的头文件。</li></ul>
 * @since RTOSck V100R005C00
 * @see NONE
 **/
extern UINT32 SRE_GetCallStack(UINT32 *puwMaxNum, UINT32 *puwPCList);
#endif


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */


#endif /* _SRE_HW_H */


