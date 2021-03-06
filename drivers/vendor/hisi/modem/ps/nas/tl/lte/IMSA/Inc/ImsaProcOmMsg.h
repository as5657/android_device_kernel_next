

#ifndef __IMSAPROCOMMSG_H__
#define __IMSAPROCOMMSG_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "vos.h"
#include "omerrorlog.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#if (VOS_OS_VER != VOS_WIN32)
#pragma pack(4)
#else
#pragma pack(push, 4)
#endif


/*****************************************************************************
  2 宏定义
*****************************************************************************/


/*****************************************************************************
  3 枚举定义
*****************************************************************************/


/*****************************************************************************
  4 全局变量声明
*****************************************************************************/



/*****************************************************************************
  5 消息头定义
*****************************************************************************/


/*****************************************************************************
  6 消息定义
*****************************************************************************/


/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/



/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
extern VOS_VOID IMSA_RcvAcpuOmErrLogCtrlInd
(
    VOS_VOID                           *pMsg
);

extern VOS_VOID IMSA_SndAcpuOmErrLogRptCnf
(
    VOS_CHAR                           *pbuffer,
    VOS_UINT32                          ulBufUseLen
);

extern VOS_VOID IMSA_RcvAcpuOmErrLogRptReq
(
    VOS_VOID                           *pMsg
);

extern VOS_VOID IMSA_ReadErrlogCtrlInfoNvim(VOS_VOID);
#if (VOS_OS_VER == VOS_WIN32)
#pragma pack()
#else
#pragma pack(0)
#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of NasErrorLog.h */


