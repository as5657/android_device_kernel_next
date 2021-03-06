



/*****************************************************************************
  1 Include HeadFile
*****************************************************************************/
#include    "NasEmmSsMsgProc.h"
#include    "NasLmmPubMFsm.h"
#include    "PsCommonDef.h"
#include    "NasLmmPubMOsa.h"
#include    "NasEmmPubUGlobal.h"
#include    "NasEmmMrrcPubInterface.h"
#include    "NasLmmPubMOm.h"
#include    "NasEmmAttDetInclude.h"
#include    "NasEmmLppMsgProc.h"
#include "NasEmmPubUCnMsgDecode.h"
#include "NasEmmSerProc.h"


/*lint -e767*/
#define    THIS_FILE_ID            PS_FILE_ID_NASEMMSSMSGPROC_C
#define    THIS_NAS_FILE_ID        NAS_FILE_ID_NASEMMSSMSGPROC_C
/*lint +e767*/


/*****************************************************************************
  1.1 Cplusplus Announce
*****************************************************************************/
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if (FEATURE_LPP == FEATURE_ON)

/*****************************************************************************
  2 Declare the Global Variable
*****************************************************************************/


/*****************************************************************************
  3 Function
*****************************************************************************/

VOS_UINT32 NAS_EMM_CheckLcsOrLppMsgNeedHighPrioStore( VOS_VOID )
{
    /*如果是建链过程中，则高优先级缓存*/
    if(NAS_EMM_CONN_ESTING == NAS_EMM_GetConnState())
    {
        NAS_LMM_PUBM_LOG_WARN("NAS_EMM_CheckLcsOrLppMsgNeedHighPrioStore: ESTING!");

        return NAS_EMM_SUCC;
    }

    /* 链路释放过程中，则高优先级缓存 */
    if( (NAS_EMM_CONN_RELEASING == NAS_EMM_GetConnState())
        || (NAS_LMM_TIMER_RUNNING == NAS_LMM_IsStaTimerRunning(TI_NAS_EMM_STATE_T3440))
        || (NAS_LMM_TIMER_RUNNING == NAS_LMM_IsPtlTimerRunning(TI_NAS_EMM_PTL_REATTACH_DELAY)))
    {
        NAS_LMM_PUBM_LOG_WARN("NAS_EMM_CheckLcsOrLppMsgNeedHighPrioStore: RELEASING!");

        return NAS_EMM_SUCC;
    }

    return NAS_EMM_FAIL;
}


VOS_UINT32 NAS_EMM_CheckLcsOrLppMsgNeedLowPrioStore( VOS_VOID )
{
    VOS_UINT32                          ulCurEmmStat;

    /* 获取当前状态 */
    ulCurEmmStat = NAS_LMM_PUB_COMP_EMMSTATE(   NAS_EMM_CUR_MAIN_STAT,
                                                NAS_EMM_CUR_SUB_STAT);

    if (EMM_MS_REG_INIT == NAS_EMM_CUR_MAIN_STAT)
    {
        return NAS_EMM_SUCC;
    }

    switch (ulCurEmmStat)
    {
        case    NAS_LMM_PUB_COMP_EMMSTATE(EMM_MS_AUTH_INIT, EMM_SS_AUTH_WAIT_CN_AUTH):
        case    NAS_LMM_PUB_COMP_EMMSTATE(EMM_MS_TAU_INIT, EMM_SS_TAU_WAIT_CN_TAU_CNF):
        case    NAS_LMM_PUB_COMP_EMMSTATE(EMM_MS_SER_INIT, EMM_SS_SER_WAIT_CN_SER_CNF):

            NAS_LMM_PUBM_LOG_WARN("NAS_EMM_CheckLcsOrLppMsgNeedLowPrioStore: tau,ser,auth ing!");
            return NAS_EMM_SUCC;

        default:
            return NAS_EMM_FAIL;
    }
}
VOS_VOID  NAS_LMM_SndLmmLcsEstCnf
(
    LMM_LCS_EST_RESULT_ENUM_UINT32      enEstRst,
    VOS_UINT32                          ulOpid
)
{
    LMM_LCS_EST_CNF_STRU               *pLmmLcsEstCnfMsg = VOS_NULL_PTR;

    /*申请消息内存*/
    pLmmLcsEstCnfMsg = (VOS_VOID *)NAS_LMM_ALLOC_MSG(sizeof(LMM_LCS_EST_CNF_STRU));

    /*判断申请结果，若失败退出*/
    if (NAS_EMM_NULL_PTR == pLmmLcsEstCnfMsg)
    {
        return;
    }

    NAS_LMM_MEM_SET(pLmmLcsEstCnfMsg,0,sizeof(LMM_LCS_EST_CNF_STRU));

    /* 打包消息头 */
    NAS_EMM_SS_MSG_HEADER(  pLmmLcsEstCnfMsg,
                            sizeof(LMM_LCS_EST_CNF_STRU)-NAS_EMM_LEN_VOS_MSG_HEADER);

    /*填充消息ID*/
    pLmmLcsEstCnfMsg->ulMsgId    = ID_LMM_LCS_EST_CNF;

    /*填充消息内容*/
    pLmmLcsEstCnfMsg->ulOpId     = ulOpid;

    /* 填充建链结果 */
    pLmmLcsEstCnfMsg->enEstRst   = enEstRst;

    NAS_LMM_SEND_MSG(pLmmLcsEstCnfMsg);
}


VOS_UINT32 NAS_EMM_IsEpcLcsSupported( VOS_VOID )
{
    NAS_MM_UE_NET_CAP_STRU             *pstUeNetCap = VOS_NULL_PTR;
    NAS_EMM_PUB_INFO_STRU              *pstEmmInfo  = VOS_NULL_PTR;
    NAS_LMM_PUB_INFO_STRU              *pstPubInfo  = VOS_NULL_PTR;

    /* 指向 UeNetCap*/
    pstEmmInfo  = NAS_LMM_GetEmmInfoAddr();
    pstUeNetCap = NAS_LMM_GetEmmInfoSecuParaUeNetCapAddr();

    if (NAS_EMM_BIT_SLCT != pstEmmInfo->bitOpUeNetCap)
    {
        NAS_LMM_PUBM_LOG_NORM("NAS_EMM_IsEpcLcsSupported: Ue net cap not exist");
        return NAS_EMM_NO;
    }

    /* 判断UE能力中是否包含LPP和LCS的能力 */
    if (pstUeNetCap->ucUeNetCapLen < 5)
    {
        NAS_LMM_PUBM_LOG_NORM("NAS_EMM_IsEpcLcsSupported: Ue net cap len < 5");
        return NAS_EMM_NO;
    }

    /* 判断LCS和LPP是否支持，格式如下:
    ProSe-dd ProSe H.245-ASH ACC-CSFB LPP LCS 1xSRVCC NF */
    if (0x0c != (pstUeNetCap->aucUeNetCap[4] & 0x0c))
    {
        NAS_LMM_PUBM_LOG_NORM("NAS_EMM_IsEpcLcsSupported: ue not support epc-lcs");
        return NAS_EMM_NO;
    }

    /* 判断网络是否支持EPC-LCS
       0   0   ESR-PS  CS-LCS(2bit)  EPC-LCS EMC-BS IMS-VoPS */
    pstPubInfo = NAS_LMM_GetEmmInfoAddr();
    if ((NAS_EMM_BIT_NO_SLCT == pstPubInfo->bitOpNetFeature)
        || (0x04 != (pstPubInfo->ucNetFeature & 0x04)))
    {
        NAS_LMM_PUBM_LOG_NORM("NAS_EMM_IsEpcLcsSupported: nw not support epc-lcs");
        return NAS_EMM_NO;
    }

    return NAS_EMM_YES;
}


VOS_VOID NAS_EMM_SndLcsDataCnf
(
    LMM_LCS_SEND_RSLT_ENUM_UINT32       enDataCnfRst,
    VOS_UINT32                          ulOpId
)
{
    LMM_LCS_DATA_CNF_STRU               *pstEmmLcsDataCnf = NAS_EMM_NULL_PTR;

    NAS_EMM_PUBU_LOG_NORM("NAS_EMM_SndLcsDataCnf is entered!");

    /*申请消息内存*/
    pstEmmLcsDataCnf   = (VOS_VOID *)NAS_LMM_ALLOC_MSG(sizeof(LMM_LCS_DATA_CNF_STRU));

    /*判断申请结果，若失败退出*/
    if (NAS_EMM_NULL_PTR == pstEmmLcsDataCnf)
    {
        return;
    }

    /*填充消息头*/
    NAS_EMM_SS_MSG_HEADER(  pstEmmLcsDataCnf,
                            (sizeof(LMM_LCS_DATA_CNF_STRU)- NAS_EMM_LEN_VOS_MSG_HEADER));

    /* 填充消息ID */
    pstEmmLcsDataCnf->ulMsgId        = ID_LMM_LCS_DATA_CNF;

    /* 填充消息OPID */
    pstEmmLcsDataCnf->ulOpId         = ulOpId;

    /* 填充发送结果 */
    pstEmmLcsDataCnf->enDataCnfRst   = enDataCnfRst;

    /*向SS发送ID_LMM_LCS_DATA_CNF消息*/
    NAS_LMM_SEND_MSG(pstEmmLcsDataCnf);
}
VOS_VOID NAS_EMM_SndLcsCnf
(
    VOS_UINT32                          ulMsgId,
    LMM_LCS_EST_RESULT_ENUM_UINT32      enEstRst,
    LMM_LCS_SEND_RSLT_ENUM_UINT32       enDataCnfRst,
    VOS_UINT32                          ulOpId
)
{
    if(ID_LCS_LMM_EST_REQ == ulMsgId)
    {
        NAS_LMM_SndLmmLcsEstCnf(enEstRst, ulOpId);
    }
    else
    {
        NAS_EMM_SndLcsDataCnf(enDataCnfRst, ulOpId);
    }
}


VOS_UINT32  NAS_EMM_SomeStateRcvLcsMsgCommProc
(
    VOS_UINT32                          ulMsgId,
    VOS_UINT32                          ulOpId
)
{

    if (EMM_MS_DEREG == NAS_EMM_CUR_MAIN_STAT)
    {
        /* 卡无效 */
        if (EMM_SS_DEREG_NO_IMSI == NAS_EMM_CUR_SUB_STAT)
        {
            NAS_EMM_SndLcsCnf(   ulMsgId, LMM_LCS_EST_RSLT_FAIL_USIM_PS_INVALID,
                                LMM_LCS_SEND_RSLT_FAIL_USIM_PS_INVALID, ulOpId);
            return NAS_LMM_MSG_HANDLED;
        }

        /* 如果3411在运行，建议SS等10s之后再次发起建链 */
        if ((EMM_SS_DEREG_ATTEMPTING_TO_ATTACH == NAS_EMM_CUR_SUB_STAT)
            && (NAS_LMM_TIMER_RUNNING == NAS_LMM_IsPtlTimerRunning(TI_NAS_EMM_PTL_T3411)))
        {
            NAS_EMM_SndLcsCnf(   ulMsgId, LMM_LCS_EST_RSLT_FAIL_3411_RUNNING,
                                LMM_LCS_SEND_RSLT_RSLT_FAIL_3411_RUNNING, ulOpId);
            return NAS_LMM_MSG_HANDLED;
        }

        /* PS不允许注册 */
        if (NAS_EMM_AUTO_ATTACH_NOT_ALLOW == NAS_EMM_GetPsAttachAllowFlg())
        {
            NAS_EMM_SndLcsCnf(   ulMsgId, LMM_LCS_EST_RSLT_FAIL_PS_ATTACH_NOT_ALLOWED,
                                LMM_LCS_SEND_RSLT_FAIL_PS_ATTACH_NOT_ALLOWED, ulOpId);
            return NAS_LMM_MSG_HANDLED;
        }

        /* PS未注册 */
        NAS_EMM_SndLcsCnf(   ulMsgId, LMM_LCS_EST_RSLT_FAIL_PS_NOT_ATTACHED,
                            LMM_LCS_SEND_RSLT_FAIL_PS_NOT_ATTACHED, ulOpId);

        return NAS_LMM_MSG_HANDLED;
    }

    if (EMM_MS_REG == NAS_EMM_CUR_MAIN_STAT)
    {
        /* 被BAR */
        if (EMM_SS_REG_WAIT_ACCESS_GRANT_IND == NAS_EMM_CUR_SUB_STAT)
        {
            NAS_EMM_SndLcsCnf(   ulMsgId, LMM_LCS_EST_RSLT_FAIL_ACCESS_BARRED,
                                LMM_LCS_SEND_RSLT_FAIL_ACCESS_BARRED, ulOpId);
            return NAS_LMM_MSG_HANDLED;
        }

        /* 如果3411在运行，建议SS等10s之后再次发起建链 */
        if ((EMM_SS_REG_ATTEMPTING_TO_UPDATE == NAS_EMM_CUR_SUB_STAT)
            && (NAS_LMM_TIMER_RUNNING == NAS_LMM_IsPtlTimerRunning(TI_NAS_EMM_PTL_T3411)))
        {
            NAS_EMM_SndLcsCnf(   ulMsgId, LMM_LCS_EST_RSLT_FAIL_3411_RUNNING,
                                LMM_LCS_SEND_RSLT_RSLT_FAIL_3411_RUNNING, ulOpId);
            return NAS_LMM_MSG_HANDLED;
        }

        /* 状态不能发起SERVICE */
        NAS_EMM_SndLcsCnf(   ulMsgId, LMM_LCS_EST_RSLT_FAIL_STATE_CANNOT_SERVICE,
                            LMM_LCS_SEND_RSLT_FAIL_RRC_CONN_NOT_EXIST, ulOpId);

        return NAS_LMM_MSG_HANDLED;
    }

    NAS_EMM_SndLcsCnf(   ulMsgId, LMM_LCS_EST_RSLT_FAIL_OTHERS,
                        LMM_LCS_SEND_RSLT_FAIL_OHTERS, ulOpId);

    return NAS_LMM_MSG_HANDLED;
}
VOS_UINT32 NAS_EMM_RcvLcsMsgEstReq
(
    MsgBlock                           *pMsg
)
{
    VOS_UINT32                          ulCurEmmStat;
    LCS_LMM_EST_REQ_STRU               *pstLcsLmmEstReq = NAS_EMM_NULL_PTR;

    pstLcsLmmEstReq = (LCS_LMM_EST_REQ_STRU*)pMsg;

    NAS_LMM_PUBM_LOG_NORM("NAS_EMM_RcvLcsMsgEstReq enter!");

    /* 获取当前状态 */
    ulCurEmmStat = NAS_LMM_PUB_COMP_EMMSTATE(   NAS_EMM_CUR_MAIN_STAT,
                                                NAS_EMM_CUR_SUB_STAT);

    /* 检查是否需要高优先级缓存 */
    if (NAS_EMM_SUCC == NAS_EMM_CheckLcsOrLppMsgNeedHighPrioStore())
    {
        return NAS_LMM_STORE_HIGH_PRIO_MSG;
    }

    /* 判断是否需要低优先级缓存 */
    if (NAS_EMM_SUCC == NAS_EMM_CheckLcsOrLppMsgNeedLowPrioStore())
    {
        return NAS_LMM_STORE_LOW_PRIO_MSG;
    }

    /* REG + EMM_SS_REG_NORMAL_SERVICE, EMM_SS_REG_ATTEMPTING_TO_UPDATE_MM*/
    if ((ulCurEmmStat == NAS_LMM_PUB_COMP_EMMSTATE(EMM_MS_REG, EMM_SS_REG_NORMAL_SERVICE))
        || (ulCurEmmStat == NAS_LMM_PUB_COMP_EMMSTATE(EMM_MS_REG, EMM_SS_REG_ATTEMPTING_TO_UPDATE_MM))
        || (ulCurEmmStat == NAS_LMM_PUB_COMP_EMMSTATE(EMM_MS_REG, EMM_SS_REG_IMSI_DETACH_WATI_CN_DETACH_CNF)))
    {
        /* 判断EPC-LCS能力是否支持 */
        if (NAS_EMM_NO == NAS_EMM_IsEpcLcsSupported())
        {
            NAS_LMM_SndLmmLcsEstCnf(    LMM_LCS_EST_RSLT_FAIL_EPC_LCS_NOT_SUPPORT,
                                        pstLcsLmmEstReq->ulOpId);

            return NAS_LMM_MSG_HANDLED;
        }

        if((NAS_EMM_CONN_DATA == NAS_EMM_GetConnState())
            || (NAS_EMM_CONN_SIG == NAS_EMM_GetConnState()))
        {
            NAS_LMM_PUBM_LOG_NORM("NAS_EMM_RcvLcsMsgEstReq: CONNECT!");

            /*如果链路已经存在，则直接回复SS，建链成功*/
            NAS_LMM_SndLmmLcsEstCnf(    LMM_LCS_EST_RSLT_SUCC,
                                        pstLcsLmmEstReq->ulOpId);

            return NAS_LMM_MSG_HANDLED;
        }

        /*PS正常服务态，而且非连接态，则进入状态机处理*/
        return NAS_LMM_MSG_DISCARD;
    }
    else
    {
        return NAS_EMM_SomeStateRcvLcsMsgCommProc(  ID_LCS_LMM_EST_REQ,
                                                    pstLcsLmmEstReq->ulOpId);
    }
}
VOS_VOID    NAS_EMM_SndLcsAirMsg
(
    LCS_LMM_DATA_REQ_STRU              *pstLcsLmmDataReq
)
{
    NAS_EMM_UPLINK_GENERIC_NAS_TRANSPORT_STRU  *pstUplinkMsg;

    pstUplinkMsg = (NAS_EMM_UPLINK_GENERIC_NAS_TRANSPORT_STRU *)NAS_LMM_MEM_ALLOC(sizeof(NAS_EMM_UPLINK_GENERIC_NAS_TRANSPORT_STRU));
    if(NAS_EMM_NULL_PTR == pstUplinkMsg)
    {
        return  ;
    }
    NAS_LMM_MEM_SET(    pstUplinkMsg,
                        0,
                        sizeof(NAS_EMM_UPLINK_GENERIC_NAS_TRANSPORT_STRU));

    /*填充container type，是LPP消息还是LCS消息*/
    pstUplinkMsg->ucContainerType = NAS_EMM_LCS_MSG;

    /*填充Msg container, LPP消息码流*/
    pstUplinkMsg->stContainer.usMsgLen = (VOS_UINT16)(pstLcsLmmDataReq->stLcsMsg.ulLcsMsgSize);
    NAS_LMM_MEM_CPY(    pstUplinkMsg->stContainer.aucMsg,
                        pstLcsLmmDataReq->stLcsMsg.aucLcsMsg,
                        pstUplinkMsg->stContainer.usMsgLen);

    /*填充sessionId*/
    pstUplinkMsg->bitOpAdditionalInfo = NAS_EMM_BIT_NO_SLCT;

    /*编码并发送给LRRC*/
    NAS_EMM_SendMrrcDataReq_UplinkGenericNasTransport(  pstUplinkMsg,
                                                        pstLcsLmmDataReq->ulOpId,
                                                        1);
    NAS_LMM_MEM_FREE(pstUplinkMsg);
    return;
}



VOS_UINT32 NAS_EMM_RcvLcsMsgDataReq
(
    MsgBlock                           *pMsg
)
{
    VOS_UINT32                          ulCurEmmStat;
    LCS_LMM_DATA_REQ_STRU              *pstSLcsLmmDataReq = NAS_EMM_NULL_PTR;

    pstSLcsLmmDataReq = (LCS_LMM_DATA_REQ_STRU*)pMsg;

    NAS_LMM_PUBM_LOG_NORM("NAS_EMM_RcvLcsMsgDataReq enter!");

    /* 获取当前状态 */
    ulCurEmmStat = NAS_LMM_PUB_COMP_EMMSTATE(   NAS_EMM_CUR_MAIN_STAT,
                                                NAS_EMM_CUR_SUB_STAT);

    /* 检查是否需要高优先级缓存 */
    if (NAS_EMM_SUCC == NAS_EMM_CheckLcsOrLppMsgNeedHighPrioStore())
    {
        return NAS_LMM_STORE_HIGH_PRIO_MSG;
    }

    /* 判断是否需要低优先级缓存 */
    if (NAS_EMM_SUCC == NAS_EMM_CheckLcsOrLppMsgNeedLowPrioStore())
    {
        return NAS_LMM_STORE_LOW_PRIO_MSG;
    }

    /* REG + EMM_SS_REG_NORMAL_SERVICE, EMM_SS_REG_ATTEMPTING_TO_UPDATE_MM*/
    if ((ulCurEmmStat == NAS_LMM_PUB_COMP_EMMSTATE(EMM_MS_REG, EMM_SS_REG_NORMAL_SERVICE))
        || (ulCurEmmStat == NAS_LMM_PUB_COMP_EMMSTATE(EMM_MS_REG, EMM_SS_REG_ATTEMPTING_TO_UPDATE_MM))
        || (ulCurEmmStat == NAS_LMM_PUB_COMP_EMMSTATE(EMM_MS_REG, EMM_SS_REG_IMSI_DETACH_WATI_CN_DETACH_CNF)))
    {
        /* 判断EPC-LCS能力是否支持 */
        if (NAS_EMM_YES == NAS_EMM_IsEpcLcsSupported())
        {
            NAS_EMM_SndLcsDataCnf(  LMM_LCS_SEND_RSLT_FAIL_EPC_LCS_NOT_SUPPORT,
                                    pstSLcsLmmDataReq->ulOpId);

            return NAS_LMM_MSG_HANDLED;
        }

        if((NAS_EMM_CONN_SIG == NAS_EMM_GetConnState())
          ||(NAS_EMM_CONN_DATA == NAS_EMM_GetConnState()))
        {
            /*编码上行NAS透传消息，并且发送给RRC*/
            NAS_EMM_SndLcsAirMsg(pstSLcsLmmDataReq);
        }
        else
        {
            NAS_LMM_PUBM_LOG_WARN(  "NAS_EMM_RcvLcsMsgDataReq: CONN NOT EXSIT!!");
            NAS_EMM_SndLcsDataCnf(  LMM_LCS_SEND_RSLT_FAIL_RRC_CONN_NOT_EXIST,
                                    pstSLcsLmmDataReq->ulOpId);
        }

        return NAS_LMM_MSG_HANDLED;
    }
    else
    {
        return NAS_EMM_SomeStateRcvLcsMsgCommProc(  ID_LCS_LMM_DATA_REQ,
                                                    pstSLcsLmmDataReq->ulOpId);
    }
}


VOS_VOID NAS_LMM_SndLcsDataInd
(
    NAS_EMM_GENERIC_MESSAGE_CONTAINER_STRU     *pstContainer
)
{
    LMM_LCS_DATA_IND_STRU              *pLmmLcsDataInd   = NAS_EMM_NULL_PTR;
    VOS_UINT32                          ulTmpLength     = 0;

    NAS_EMM_PUBU_LOG_NORM("NAS_LMM_SndLcsDataInd enter!");

    if( 0 == pstContainer->usMsgLen)
    {
        NAS_EMM_PUBU_LOG_WARN("NAS_LMM_SndLcsDataInd: WARNING: Msg Length is zero");
        return ;
    }
    else if(pstContainer->usMsgLen > NAS_EMM_4BYTES_LEN)
    {
        ulTmpLength     = pstContainer->usMsgLen - NAS_EMM_4BYTES_LEN ;
        pLmmLcsDataInd   = (VOS_VOID*)NAS_LMM_ALLOC_MSG(sizeof(LMM_LCS_DATA_IND_STRU) + ulTmpLength);
    }
    else/*如果长度小于NAS_EMM_4BYTES_LEN，分配的空间等于NAS_EMM_4BYTES_LEN*/
    {
        ulTmpLength     = 0;
        pLmmLcsDataInd   = (VOS_VOID*)NAS_LMM_ALLOC_MSG(sizeof(LMM_LCS_DATA_IND_STRU));
    }

    /*判断申请结果，若失败退出*/
    if (NAS_EMM_NULL_PTR == pLmmLcsDataInd)
    {
        return ;
    }

    /*构造ID_LMM_LCS_DATA_IND消息*/
    /*填充消息头*/
    NAS_EMM_SS_MSG_HEADER(  pLmmLcsDataInd,\
                           ((sizeof(LMM_LCS_DATA_IND_STRU) + ulTmpLength) - NAS_EMM_LEN_VOS_MSG_HEADER));

    /*填充消息ID*/
    pLmmLcsDataInd->ulMsgId  = ID_LMM_LCS_DATA_IND;

    pLmmLcsDataInd->ulOpId   = NAS_EMM_OPID_MM;

    /*填充SS空口码流*/
    pLmmLcsDataInd->stLcsMsg.ulLcsMsgSize = pstContainer->usMsgLen;
    NAS_LMM_MEM_CPY(    pLmmLcsDataInd->stLcsMsg.aucLcsMsg,
                        pstContainer->aucMsg,
                        pLmmLcsDataInd->stLcsMsg.ulLcsMsgSize);

    /*向SS发送ID_LMM_LCS_DATA_IND消息*/
    NAS_LMM_SEND_MSG(pLmmLcsDataInd);

    return;
}

/*****************************************************************************
 Function Name   : NAS_EMM_SndLcsPosCapInfoInd
 Description     : 通知LCS模块LCS能力
 Input           : None
 Output          : None
 Return          : VOS_UINT32

 History         :
    1.lihong00150010      2015-07-30  Draft Enact
*****************************************************************************/
VOS_VOID    NAS_EMM_SndLcsPosCapInfoInd( VOS_VOID )
{
    NAS_LMM_PUB_INFO_STRU              *pstPubInfo              = VOS_NULL_PTR;
    LMM_LCS_POS_CAP_INFO_IND_STRU      *pstEmmLcsPosCapInfoInd   = NAS_EMM_NULL_PTR;

    NAS_EMM_PUBU_LOG_INFO("NAS_EMM_SndLcsPosCapInfoInd is entered!");;


    /*申请消息内存*/
    pstEmmLcsPosCapInfoInd   = (VOS_VOID *)NAS_LMM_ALLOC_MSG(sizeof(LMM_LCS_POS_CAP_INFO_IND_STRU));

    /*判断申请结果，若失败退出*/
    if (NAS_EMM_NULL_PTR == pstEmmLcsPosCapInfoInd)
    {
        return;
    }

    /*填充消息头*/
    NAS_EMM_SS_MSG_HEADER(  pstEmmLcsPosCapInfoInd,
                            (sizeof(LMM_LCS_POS_CAP_INFO_IND_STRU)- NAS_EMM_LEN_VOS_MSG_HEADER));

    /* 填充消息ID */
    pstEmmLcsPosCapInfoInd->ulMsgId  = ID_LMM_LCS_POS_CAP_INFO_IND;

    /* 填充消息OPID */
    pstEmmLcsPosCapInfoInd->ulOpId   = NAS_EMM_OPID_MM;

    pstPubInfo = NAS_LMM_GetEmmInfoAddr();

    /* 填充网侧是否支持EPC-LCS和CS-LCS
     0   0   ESR-PS  CS-LCS(2bit)  EPC-LCS EMC-BS IMS-VoPS */
    if (NAS_EMM_BIT_SLCT == pstPubInfo->bitOpNetFeature)
    {
        pstEmmLcsPosCapInfoInd->enNwEpcLcs   = (pstPubInfo->ucNetFeature & NAS_EMM_BIT_3) >> NAS_EMM_MOVEMENT_2_BITS;
        pstEmmLcsPosCapInfoInd->enNwCsLcs    = (pstPubInfo->ucNetFeature & NAS_EMM_BIT_4_5) >> NAS_EMM_MOVEMENT_3_BITS;
    }
    else
    {
        pstEmmLcsPosCapInfoInd->enNwEpcLcs   = LMM_LCS_NW_EPC_LCS_NOT_SUPPORT;
        pstEmmLcsPosCapInfoInd->enNwCsLcs    = LMM_LCS_NW_CS_LCS_NO_INFORMATION;
    }

    /*向SS发送ID_LMM_LCS_POSITION_CAP_INFO_IND消息*/
    NAS_LMM_SEND_MSG(pstEmmLcsPosCapInfoInd);

    return;
}
VOS_UINT32  NAS_EMM_MsRegSsNormalMsgLcsEstReq
(
    VOS_UINT32                          ulMsgId,
    VOS_VOID                           *pMsgStru
)
{
    VOS_UINT32                          ulRslt          = NAS_EMM_FAIL;
    LCS_LMM_EST_REQ_STRU               *pstLcsLmmEstReq  = NAS_EMM_NULL_PTR;

    (VOS_VOID)ulMsgId;

    NAS_EMM_SER_LOG_INFO("NAS_EMM_MsRegSsNormalMsgLcsEstReq entered.");

    ulRslt = NAS_EMM_SER_CHKFSMStateMsgp(EMM_MS_REG,EMM_SS_REG_NORMAL_SERVICE,pMsgStru);
    if ( NAS_EMM_SUCC != ulRslt )
    {
        NAS_EMM_SER_LOG_WARN( "NAS_EMM_MsRegSsNormalMsgLcsEstReq ERROR !!");
        return NAS_LMM_MSG_DISCARD;
    }

    pstLcsLmmEstReq = (LCS_LMM_EST_REQ_STRU*)pMsgStru;

    /*如果不处于空闲态，可以直接回复建链成功*/
    if (NAS_EMM_NO == NAS_LMM_IsRrcConnectStatusIdle())
    {
        NAS_LMM_SndLmmLcsEstCnf( LMM_LCS_EST_RSLT_SUCC,
                                pstLcsLmmEstReq->ulOpId);

        return NAS_LMM_MSG_HANDLED;
    }

    /* 设置SER触发原因为 NAS_EMM_SER_START_CAUSE_SS_EST_REQ */
    NAS_EMM_SER_SaveEmmSERStartCause(NAS_EMM_SER_START_CAUSE_SS_EST_REQ);

    /* 存储OPID */
    NAS_EMM_SER_SaveSsOrLppOpid(pstLcsLmmEstReq->ulOpId);

    /* Inform RABM that SER init */
    NAS_EMM_SER_SendRabmReestInd(EMM_ERABM_REEST_STATE_INITIATE);

    /* 启动定时器3417 */
    NAS_LMM_StartStateTimer(TI_NAS_EMM_STATE_SERVICE_T3417);

    /* 转换EMM状态机MS_SER_INIT+SS_SER_WAIT_CN_CNF */
    NAS_EMM_TAUSER_FSMTranState(EMM_MS_SER_INIT, EMM_SS_SER_WAIT_CN_SER_CNF, TI_NAS_EMM_STATE_SERVICE_T3417);

    /* 组合并发送MRRC_DATA_REQ(SERVICE_REQ) */
    NAS_EMM_SER_SendMrrcDataReq_ServiceReq();

    return NAS_LMM_MSG_HANDLED;
}
VOS_UINT32  NAS_EMM_MsRegSsRegAttemptUpdateMmMsgLcsEstReq
(
    VOS_UINT32                          ulMsgId,
    VOS_VOID                           *pMsgStru
)
{
    VOS_UINT32                          ulRslt          = NAS_EMM_FAIL;
    LCS_LMM_EST_REQ_STRU               *pstLcsLmmEstReq  = NAS_EMM_NULL_PTR;

    (VOS_VOID)ulMsgId;

    NAS_EMM_SER_LOG_INFO("NAS_EMM_MsRegSsRegAttemptUpdateMmMsgLcsEstReq entered.");

    ulRslt = NAS_EMM_SER_CHKFSMStateMsgp(EMM_MS_REG,EMM_SS_REG_ATTEMPTING_TO_UPDATE_MM,pMsgStru);
    if ( NAS_EMM_SUCC != ulRslt )
    {
        NAS_EMM_SER_LOG_WARN( "NAS_EMM_MsRegSsRegAttemptUpdateMmMsgLcsEstReq ERROR !!");
        return NAS_LMM_MSG_DISCARD;
    }

    pstLcsLmmEstReq = (LCS_LMM_EST_REQ_STRU*)pMsgStru;

    /*如果不处于空闲态，可以直接回复建链成功*/
    if (NAS_EMM_NO == NAS_LMM_IsRrcConnectStatusIdle())
    {
        NAS_LMM_SndLmmLcsEstCnf( LMM_LCS_EST_RSLT_SUCC,
                                pstLcsLmmEstReq->ulOpId);

        return NAS_LMM_MSG_HANDLED;
    }

    /* 设置SER触发原因为 NAS_EMM_SER_START_CAUSE_SS_EST_REQ */
    NAS_EMM_SER_SaveEmmSERStartCause(NAS_EMM_SER_START_CAUSE_SS_EST_REQ);

    /* 存储OPID */
    NAS_EMM_SER_SaveSsOrLppOpid(pstLcsLmmEstReq->ulOpId);

    /* Inform RABM that SER init */
    NAS_EMM_SER_SendRabmReestInd(EMM_ERABM_REEST_STATE_INITIATE);

    /* 启动定时器3417 */
    NAS_LMM_StartStateTimer(TI_NAS_EMM_STATE_SERVICE_T3417);

    /* 转换EMM状态机MS_SER_INIT+SS_SER_WAIT_CN_CNF */
    NAS_EMM_TAUSER_FSMTranState(EMM_MS_SER_INIT, EMM_SS_SER_WAIT_CN_SER_CNF, TI_NAS_EMM_STATE_SERVICE_T3417);

    /* 组合并发送MRRC_DATA_REQ(SERVICE_REQ) */
    NAS_EMM_SER_SendMrrcDataReq_ServiceReq();

    return NAS_LMM_MSG_HANDLED;
}



#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

