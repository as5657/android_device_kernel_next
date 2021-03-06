


/*****************************************************************************
   1 头文件包含
*****************************************************************************/
#include "TafXCallProcMma.h"
#include "TafXCallCtx.h"
#include "TafXCallSndApp.h"
#include "TafXCallSndMma.h"
#include "TafXCallProcUsim.h"
#include "TafXCallSndXcc.h"
#include "TafSdcCtx.h"
/* Added by y00322978 for CDMA Iteration 17, 2015-7-15, begin */
#include "TafMmaSndOm.h"
/* Added by y00322978 for CDMA Iteration 17, 2015-7-15, end */
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */


#define    THIS_FILE_ID                 PS_FILE_ID_TAF_X_CALL_PROC_MMA_C

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)

/*****************************************************************************
   2 变量定义
*****************************************************************************/


/*****************************************************************************
   3 函数实现
*****************************************************************************/
/*lint -save -e958 */

VOS_VOID TAF_XCALL_ResetAllCalls(VOS_VOID)
{
    VOS_UINT32                                              ulNumOfCalls;
    VOS_UINT8                                               aucCallIds[TAF_XCALL_CALL_ENTITY_MAX_NUM];
    VOS_UINT32                                              ulModuleId;
    VOS_UINT8                                               i;
    TAF_XCALL_SS_PROGRESS_EVT_ENUM_UINT8                    enEvt;
    TAF_XCALL_SS_SUBSEQ_OP_ENUM_UINT8                       enSubseqOp;
    TAF_XCALL_CALL_ENTITY_STRU                             *pstCallEntity = VOS_NULL_PTR;
    VOS_UINT16                                              usClientId;
    MN_CALL_TYPE_ENUM_U8                                    enCallType;

    enEvt        = TAF_XCALL_SS_PROG_EVT_BUTT;
    enSubseqOp   = TAF_XCALL_SS_SUBSEQ_OP_BUTT;

    ulNumOfCalls = 0;

    PS_MEM_SET(aucCallIds, 0, sizeof(aucCallIds));

    TAF_XCALL_GetNotIdleStateCalls(&ulNumOfCalls, aucCallIds);

    if (0 == ulNumOfCalls)
    {
        return;
    }

    /* 保存当前call id的module id，用于release all上报 */
    ulModuleId = TAF_XCALL_GetClientInfo(aucCallIds[0])->ulModuleId;

    /* 软关机时需要判断当前是否有非IDLE态的呼叫，如果有需要上报MN_CALL_EVT_RELEASED事件 */
    for (i = 0; i < TAF_XCALL_MIN(ulNumOfCalls, TAF_XCALL_CALL_ENTITY_MAX_NUM); i++)
    {
        /* 记录主动挂断的原因值 */
        TAF_XCALL_UpdateCcCause(aucCallIds[i], TAF_CS_CAUSE_XCALL_POWER_DOWN_IND);

        /* 给MMA发送TAF_MMA_1X_CALL_END_NTF消息 */
        pstCallEntity       = TAF_XCALL_GetCallEntityAddr(aucCallIds[i]);
        usClientId          = MN_GetRealClientId(pstCallEntity->stClientInfo.usClientId, WUEPS_PID_TAF);
        enCallType          = TAF_XCALL_GetCallType(aucCallIds[i]);

        if (MN_CALL_DIR_MO == TAF_XCALL_GetCallDir(aucCallIds[i]))
        {
            TAF_SndMmaCdmaMoCallEndNtf(WUEPS_PID_TAF, usClientId, TAF_XCALL_BuildMmaCallType(enCallType));
        }

        /* 进行保护停止相关重拨定时器 */
        TAF_XCALL_StopTimer(TI_TAF_XCALL_WAIT_CALL_REDIAL_PERIOD, aucCallIds[i]);
        TAF_XCALL_StopTimer(TI_TAF_XCALL_WAIT_CALL_REDAIL_INTERVAL, aucCallIds[i]);

        /* 给APP上报MN_CALL_EVT_RELEASED事件 */
        TAF_XCALL_SndCallReleaseInd(aucCallIds[i]);


        TAF_XCALL_GetSsKeyEvent(aucCallIds[i], &enEvt, &enSubseqOp);

        /* 如果正在挂断过程中，网络挂断，给APP回复挂断成功 */
        if (TAF_XCALL_SS_PROG_EVT_REL == enEvt)
        {
            /* 给APP回复MN_CALL_EVT_SS_CMD_RSLT事件 */
            TAF_XCALL_SndSupsResult(TAF_XCALL_GetClientInfo(aucCallIds[i])->ulModuleId,
                                    TAF_XCALL_GetClientInfo(aucCallIds[i])->usClientId,
                                    TAF_XCALL_GetClientInfo(aucCallIds[i])->ucOpId,
                                    aucCallIds[i], MN_CALL_SS_RES_SUCCESS);

            TAF_XCALL_DeRegSsKeyEvent(aucCallIds[i], TAF_XCALL_SS_PROG_EVT_REL);
        }

        /* 如果正在接听过程中，网络挂断，给APP回复接听失败 */
        if (TAF_XCALL_SS_PROG_EVT_ANS == enEvt)
        {
            /* 给APP回复MN_CALL_EVT_SS_CMD_RSLT事件 */
            TAF_XCALL_SndSupsResult(TAF_XCALL_GetClientInfo(aucCallIds[i])->ulModuleId,
                                    TAF_XCALL_GetClientInfo(aucCallIds[i])->usClientId,
                                    TAF_XCALL_GetClientInfo(aucCallIds[i])->ucOpId,
                                    aucCallIds[i], MN_CALL_SS_RES_FAIL);

            TAF_XCALL_DeRegSsKeyEvent(aucCallIds[i], TAF_XCALL_SS_PROG_EVT_ANS);
        }
    }

    /* 给APP上报MN_CALL_EVT_ALL_RELEASED事件 */
    TAF_XCALL_SndCallAllReleaseInd(ulModuleId);

}
VOS_VOID TAF_XCALL_RcvMmaPowerOffInd(VOS_VOID *pMsg)
{
    /* 重置所有呼叫 */
    TAF_XCALL_ResetAllCalls();

    /* 上下文初始化 */
    TAF_XCALL_InitCtx();

}



VOS_VOID TAF_XCALL_RcvMmaPowerOnInd(VOS_VOID *pMsg)
{
    /* 6F47文件,存储到SDC中TAF_SDC_Set1xCallUcimEmccNumList */
    /* TO DO 后续USIMM模块提供API接口直接获取紧急呼叫号码,异步读取文件接口暂不实现 */
}



VOS_UINT8 TAF_XCALL_IsNeedRedialImmediately(
    TAF_CS_CAUSE_ENUM_UINT32            enCause
)
{
    switch(enCause)
    {
        case TAF_CS_CAUSE_XCALL_NDSS:
        case TAF_CS_CAUSE_XCALL_REDIRECTION:
        case TAF_CS_CAUSE_XCALL_ABORT:
            return VOS_TRUE;

        default:
            return VOS_FALSE;
    }
}

VOS_VOID TAF_XCALL_RcvMmaServiceStatusInd(VOS_VOID *pMsg)
{
    VOS_UINT8                           ucCallId;
    VOS_UINT8                           ucCallFound;
    TAF_XCALL_CALL_ENTITY_STRU         *pstXcallEntity   = VOS_NULL_PTR;
    VOS_UINT8                           ucRedialFlg;
    TAF_SDC_SERVICE_STATUS_ENUM_UINT8   enServiceStatus;

    MMA_TAF_1X_SERVICE_STATUS_IND_STRU *pstMmaServiceStatusInd;

    pstMmaServiceStatusInd = (MMA_TAF_1X_SERVICE_STATUS_IND_STRU*)pMsg;

    if (VOS_TRUE != pstMmaServiceStatusInd->ulExist1XService)
    {
        return;
    }


    ucCallFound     = VOS_FALSE;
    enServiceStatus = TAF_SDC_GetCsServiceStatus();

    for (ucCallId = 1; ucCallId <= TAF_XCALL_CALL_ENTITY_MAX_NUM; ucCallId++)
    {
        pstXcallEntity = TAF_XCALL_GetCallEntityAddr(ucCallId);

        /* Added by h00313353 for Iteration 17, 2015-7-28, begin */
        /* 如果是紧急呼，认为需要立即尝试发起呼叫 */
        if ((VOS_TRUE == pstXcallEntity->ulUsed)
         && (MN_CALL_TYPE_EMERGENCY == pstXcallEntity->enCallType))
        {
            ucCallFound = VOS_TRUE;
            break;
        }
        /* Added by h00313353 for Iteration 17, 2015-7-28, end */

        /*获取是否需要立即重拨的标记 */
        ucRedialFlg    = TAF_XCALL_IsNeedRedialImmediately(pstXcallEntity->enCause);

        /*是否存在重拨呼叫 */
        if ((VOS_TRUE == pstXcallEntity->ulUsed)
         && (VOS_TRUE == ucRedialFlg))
        {
            ucCallFound = VOS_TRUE;
            break;
        }
    }

    if (VOS_FALSE == ucCallFound)
    {
        return;
    }

    /*判断呼叫状态 */
    if (MN_CALL_TYPE_EMERGENCY == pstXcallEntity->enCallType)
    {
        /*判断服务状态*/
        if ((TAF_SDC_SERVICE_STATUS_NORMAL_SERVICE  == enServiceStatus)
         || (TAF_SDC_SERVICE_STATUS_LIMITED_SERVICE == enServiceStatus))
         {
            /*判断呼叫重拨定时器是否已经超时 */
            if (TAF_XCALL_TIMER_STATUS_RUNING == TAF_XCALL_GetTimerStatus(TI_TAF_XCALL_WAIT_CALL_REDAIL_INTERVAL, ucCallId))
            {
                /*关闭重拨定时器，发起重拨 */
                TAF_XCALL_StopTimer(TI_TAF_XCALL_WAIT_CALL_REDAIL_INTERVAL, ucCallId);

                TAF_XCALL_SndXccOrigCallReq(ucCallId);
            }
         }
    }
    else
    {
        if (TAF_SDC_SERVICE_STATUS_NORMAL_SERVICE == enServiceStatus)
        {
            /*判断呼叫重拨定时器是否已经超时 */
            if (TAF_XCALL_TIMER_STATUS_RUNING == TAF_XCALL_GetTimerStatus(TI_TAF_XCALL_WAIT_CALL_REDAIL_INTERVAL, ucCallId))
            {
                /*关闭重拨定时器，发起重拨 */
                TAF_XCALL_StopTimer(TI_TAF_XCALL_WAIT_CALL_REDAIL_INTERVAL, ucCallId);

                TAF_XCALL_SndXccOrigCallReq(ucCallId);
            }
        }
    }

    return;
}

#endif

/*lint -restore */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */



