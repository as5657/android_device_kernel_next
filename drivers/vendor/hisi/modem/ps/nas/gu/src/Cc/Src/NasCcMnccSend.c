/******************************************************************************

                  版权所有 (C), 2001-2011, 华为技术有限公司

 ******************************************************************************
  文 件 名   : NasCcMnccSend.c
  版 本 号   : 初稿
  作    者   : 丁庆 49431
  生成日期   : 2007年10月17日
  最近修改   : 2007年10月17日
  功能描述   : 发送MNCC原语
  函数列表   :
  修改历史   :
  1.日    期   : 2007年10月17日
    作    者   : 丁庆 49431
    修改内容   : 创建文件
******************************************************************************/

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "NasCcInclude.h"


#ifdef  __cplusplus
  #if  __cplusplus
  extern "C"{
  #endif
#endif


/*****************************************************************************
  2 宏定义
*****************************************************************************/
/* 取得MNCC原语(CC -> MN)的头部长度 */
#define NAS_CC_MNCC_IND_HDR_LEN   \
    (sizeof(MNCC_IND_PRIM_MSG_STRU) - sizeof(MNCC_IND_PARAM_UNION))

#define THIS_FILE_ID PS_FILE_ID_NASCC_MNCCSEND_C

/*lint -save -e958 */

/*****************************************************************************
  3 函数定义
*****************************************************************************/

VOS_VOID  NAS_CC_SendMnccMsg(
    NAS_CC_ENTITY_ID_T                  entityId,
    MNCC_PRIM_NAME_ENUM_U16             enPrimName,
    const VOS_VOID                      *pParam,
    VOS_UINT32                          ulParamLen
)
{
    MsgBlock                            *pstTmpMsg;
    MNCC_IND_PRIM_MSG_STRU              *pstMsg;

    /* 申请VOS消息 */


    pstTmpMsg = PS_ALLOC_MSG_WITH_HEADER_LEN(WUEPS_PID_CC, NAS_CC_MNCC_IND_HDR_LEN + sizeof(pstMsg->unParam));
    if (VOS_NULL_PTR == pstTmpMsg)
    {
        NAS_CC_ERR_LOG("NAS_CC_SendMnccMsg: Failed to alloc VOS message.");
        return;
    }

    /* 填写VOS消息头 */
    pstMsg = (MNCC_IND_PRIM_MSG_STRU*)pstTmpMsg;
    pstMsg->ulSenderCpuId = VOS_LOCAL_CPUID;
    pstMsg->ulSenderPid = WUEPS_PID_CC;
    pstMsg->ulReceiverCpuId = VOS_LOCAL_CPUID;
    pstMsg->ulReceiverPid = WUEPS_PID_TAF;

    /* 填写原语首部 */
    pstMsg->enPrimName = enPrimName;
    if(NAS_CC_MAX_ENTITY_NUM != entityId )
    {
        pstMsg->ucTi = NAS_CC_GetEntityTi(entityId);
        pstMsg->ucCallId = NAS_CC_GetCallId(entityId);
    }

    pstMsg->ulParamLen = ulParamLen;
    PS_MEM_SET(&pstMsg->unParam, 0 ,sizeof(pstMsg->unParam));
    /* 如果有原语参数，填写原语参数 */
    if ((pParam != VOS_NULL_PTR)
     && (ulParamLen != 0))
    {
        PS_MEM_CPY(&pstMsg->unParam, pParam, ulParamLen);
    }

    /* 发送VOS消息 */
    if (VOS_OK != PS_SEND_MSG(WUEPS_PID_CC, pstMsg))
    {
        NAS_WARNING_LOG(WUEPS_PID_CC, "NAS_CC_SendMnccMsgs():WARNING:SEND MSG FIAL");
    }        
}
VOS_VOID  API_CcSyncInd(VOS_UINT8  ucActType, VOS_UINT32 ucRabId)
{
    MMCC_SYNC_IND_STRU* pstMsg;

    /* 此消息只处理RAB释放消息，对于建立和重配，由MM上报 */
    if(ucActType != 2)
    {
        return;
    }

    /* 申请VOS消息 */
    pstMsg = (MMCC_SYNC_IND_STRU *)
        PS_ALLOC_MSG_WITH_HEADER_LEN(WUEPS_PID_CC, sizeof(MMCC_SYNC_IND_STRU));
    if (VOS_NULL_PTR == pstMsg)
    {
        NAS_CC_ERR_LOG("API_CcSyncInd: Failed to alloc VOS message.");
        return;
    }

    /* 填写VOS消息头 */
    pstMsg->MsgHeader.ulSenderCpuId = VOS_LOCAL_CPUID;
    pstMsg->MsgHeader.ulSenderPid = WUEPS_PID_MM;
    pstMsg->MsgHeader.ulReceiverCpuId = VOS_LOCAL_CPUID;
    pstMsg->MsgHeader.ulReceiverPid = WUEPS_PID_CC;

    pstMsg->MsgHeader.ulMsgName= MMCC_SYNC_IND;
    /* message content, default value */
    pstMsg->enChannelMode= 0;

    pstMsg->ulRabNum = 1;
    pstMsg->syncInfo[0].ulRabId = ucRabId;
    pstMsg->syncInfo[0].ulRabSyncInfo = ucRabId;
    pstMsg->enSyncReason = MMCC_WCDMA_RAB_RELEASED;

    /* 发送VOS消息 */
    if (VOS_OK != PS_SEND_MSG(WUEPS_PID_CC, pstMsg))
    {
        NAS_WARNING_LOG(WUEPS_PID_CC, "API_CcSyncInd():WARNING:SEND MSG FIAL");
    }        
}

#if (FEATURE_ON == FEATURE_IMS)

VOS_VOID  NAS_CC_SendMnccSrvccStatusIndMsg(
    NAS_MNCC_SRVCC_STATUS_ENUM_UINT32   enSrvccSta
)
{
    MNCC_SRVCC_STATUS_IND_STRU          stSrvccStaInd;
    
    PS_MEM_SET((VOS_INT8*)&stSrvccStaInd,0x0, sizeof(MNCC_SRVCC_STATUS_IND_STRU));
  
    stSrvccStaInd.enSrvccStatus  = enSrvccSta;

    /* 调用统一发送到CALL的消息函数,entity ID对CALL模块不关注 */
    NAS_CC_SendMnccMsg(0,
                       MNCC_SRVCC_STATUS_IND,
                       &stSrvccStaInd,
                       sizeof(MNCC_SRVCC_STATUS_IND_STRU));

    return;
}
#endif

/*lint -restore */


#ifdef  __cplusplus
  #if  __cplusplus
  }
  #endif
#endif

