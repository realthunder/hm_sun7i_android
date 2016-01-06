#include "QMIThread.h"
extern char *strndup (const char *__string, size_t __n);

int clientWDS = -1;
int clientDMS = -1;
int clientNAS = -1;
int clientWDA = -1;
static UINT WdsConnectionIPv4Handle = 0;

typedef USHORT (*CUSTOMQMUX)(PQMUX_MSG pMUXMsg, void *arg);

// To retrieve the ith (Index) TLV
PQMI_TLV_HDR GetTLV (PQCQMUX_MSG_HDR pQMUXMsgHdr, int TLVType) {
    int TLVFind = 0;
    USHORT Length = le16_to_cpu(pQMUXMsgHdr->Length);
    PQMI_TLV_HDR pTLVHdr = (PQMI_TLV_HDR)(pQMUXMsgHdr + 1);
        
    while (Length >= sizeof(QMI_TLV_HDR)) {
        TLVFind++;
        if (TLVType > 0x1000) {
            if ((TLVFind + 0x1000) == TLVType)
                return pTLVHdr;
        } else  if (pTLVHdr->TLVType == TLVType) {
            return pTLVHdr;
        }

        Length -= (le16_to_cpu((pTLVHdr->TLVLength)) + sizeof(QMI_TLV_HDR));
        pTLVHdr = (PQMI_TLV_HDR)(((UCHAR *)pTLVHdr) + le16_to_cpu(pTLVHdr->TLVLength) + sizeof(QMI_TLV_HDR));
    }

   return NULL;
}

static USHORT GetQMUXTransactionId(void) {
    static int TransactionId = 0;
    if (++TransactionId > 0xFFFF)
        TransactionId = 1;
    return TransactionId;
}

static PQCQMIMSG ComposeQMUXMsg(UCHAR QMIType, USHORT Type, CUSTOMQMUX customQmuxMsgFunction, void *arg) {
    UCHAR QMIBuf[WDM_DEFAULT_BUFSIZE];
    PQCQMIMSG pRequest = (PQCQMIMSG)QMIBuf;
    int Length;

    pRequest->QMIHdr.IFType   = USB_CTL_MSG_TYPE_QMI;
    pRequest->QMIHdr.CtlFlags = 0x00;
    pRequest->QMIHdr.QMIType  = QMIType;   
    switch(pRequest->QMIHdr.QMIType) {
        case QMUX_TYPE_CTL: pRequest->QMIHdr.ClientId = 0x00; break;
        case QMUX_TYPE_WDS: pRequest->QMIHdr.ClientId = clientWDS & 0xFF; break;
        case QMUX_TYPE_DMS: pRequest->QMIHdr.ClientId = clientDMS & 0xFF; break;
        case QMUX_TYPE_NAS: pRequest->QMIHdr.ClientId = clientNAS & 0xFF; break;
        case QMUX_TYPE_WDS_ADMIN: pRequest->QMIHdr.ClientId = clientWDA & 0xFF; break;
        default: pRequest->QMIHdr.ClientId = -1; break;
    }

    if (!pRequest->QMIHdr.ClientId) {
        dbg_time("QMIType %d has no clientID", pRequest->QMIHdr.QMIType);
        return NULL;
    }

    pRequest->MUXMsg.QMUXHdr.CtlFlags = QMUX_CTL_FLAG_SINGLE_MSG | QMUX_CTL_FLAG_TYPE_CMD;
    pRequest->MUXMsg.QMUXHdr.TransactionId = cpu_to_le16(GetQMUXTransactionId());
    pRequest->MUXMsg.QMUXMsgHdr.Type = cpu_to_le16(Type);
    if (customQmuxMsgFunction)
        pRequest->MUXMsg.QMUXMsgHdr.Length = cpu_to_le16(customQmuxMsgFunction(&pRequest->MUXMsg, arg) - sizeof(QCQMUX_MSG_HDR));
    else
        pRequest->MUXMsg.QMUXMsgHdr.Length = cpu_to_le16(0x0000);

    pRequest->QMIHdr.Length = cpu_to_le16(le16_to_cpu(pRequest->MUXMsg.QMUXMsgHdr.Length) + sizeof(QCQMUX_MSG_HDR) + sizeof(QCQMUX_HDR)
        + sizeof(QCQMI_HDR) - 1);
    Length = le16_to_cpu(pRequest->QMIHdr.Length) + 1;

    pRequest = (PQCQMIMSG)malloc(Length);   
    if (pRequest == NULL) {
        dbg_time("%s fail to malloc", __func__);
    } else {
        memcpy(pRequest, QMIBuf, Length);
    }
    
    return pRequest;
}

#if 0
static USHORT NasSetEventReportReq(PQMUX_MSG pMUXMsg, void *arg) {
    pMUXMsg->SetEventReportReq.TLVType = 0x10;
    pMUXMsg->SetEventReportReq.TLVLength = 0x04;
    pMUXMsg->SetEventReportReq.ReportSigStrength = 0x00;
    pMUXMsg->SetEventReportReq.NumTresholds = 2;
    pMUXMsg->SetEventReportReq.TresholdList[0] = -113;
    pMUXMsg->SetEventReportReq.TresholdList[1] = -50;
    return sizeof(QMINAS_SET_EVENT_REPORT_REQ_MSG);
}

static USHORT WdsSetEventReportReq(PQMUX_MSG pMUXMsg, void *arg) {
    pMUXMsg->EventReportReq.TLVType = 0x10;          // 0x10 -- current channel rate indicator
    pMUXMsg->EventReportReq.TLVLength = 0x0001;        // 1
    pMUXMsg->EventReportReq.Mode = 0x00;             // 0-do not report; 1-report when rate changes

    pMUXMsg->EventReportReq.TLV2Type = 0x11;         // 0x11
    pMUXMsg->EventReportReq.TLV2Length = 0x0005;       // 5
    pMUXMsg->EventReportReq.StatsPeriod = 0x00;      // seconds between reports; 0-do not report
    pMUXMsg->EventReportReq.StatsMask = 0x000000ff;        // 

    pMUXMsg->EventReportReq.TLV3Type = 0x12;          // 0x12 -- current data bearer indicator
    pMUXMsg->EventReportReq.TLV3Length = 0x0001;        // 1
    pMUXMsg->EventReportReq.Mode3 = 0x01;             // 0-do not report; 1-report when changes

    pMUXMsg->EventReportReq.TLV4Type = 0x13;          // 0x13 -- dormancy status indicator
    pMUXMsg->EventReportReq.TLV4Length = 0x0001;        // 1
    pMUXMsg->EventReportReq.DormancyStatus = 0x00;    // 0-do not report; 1-report when changes
    return sizeof(QMIWDS_SET_EVENT_REPORT_REQ_MSG);
}

static USHORT DmsSetEventReportReq(PQMUX_MSG pMUXMsg) {
    PPIN_STATUS pPinState = (PPIN_STATUS)(&pMUXMsg->DmsSetEventReportReq + 1);
    PUIM_STATE pUimState = (PUIM_STATE)(pPinState + 1);
    // Pin State
    pPinState->TLVType = 0x12;
    pPinState->TLVLength = 0x01;
    pPinState->ReportPinState = 0x01;    
    // UIM State
    pUimState->TLVType = 0x15;
    pUimState->TLVLength = 0x01;
    pUimState->UIMState = 0x01;
    return sizeof(QMIDMS_SET_EVENT_REPORT_REQ_MSG) + sizeof(PIN_STATUS) + sizeof(UIM_STATE);
}
#endif

static USHORT WdsStartNwInterfaceReq(PQMUX_MSG pMUXMsg, void *arg) {
#if 1
    return sizeof(QCQMUX_MSG_HDR);
#else
    PQMIWDS_TECHNOLOGY_PREFERECE pTechPref;
    PQMIWDS_IP_FAMILY_TLV pIpFamily;
    USHORT TLVLength = 0;

    UCHAR *pTLV = (UCHAR *)(&pMUXMsg->QMUXMsgHdr + 1);
    // Set technology Preferece
    pTechPref = (PQMIWDS_TECHNOLOGY_PREFERECE)(pTLV + TLVLength);
    pTechPref->TLVType = 0x30;
    pTechPref->TLVLength = 0x01;
    pTechPref->TechPreference = 0x01;
    TLVLength += (pTechPref->TLVLength + sizeof(QCQMICTL_TLV_HDR));

    pIpFamily = (PQMIWDS_IP_FAMILY_TLV)(pTLV + TLVLength);
    pIpFamily->TLVType = 0x19;
    pIpFamily->TLVLength = 0x01;
    pIpFamily->IpFamily = 0x04; //IPV4
    TLVLength +=(pIpFamily->TLVLength + sizeof(QCQMICTL_TLV_HDR));  

    return sizeof(QCQMUX_MSG_HDR) + TLVLength;
#endif
}

static USHORT WdsStopNwInterfaceReq(PQMUX_MSG pMUXMsg, void *arg) {
    pMUXMsg->StopNwInterfaceReq.TLVType = 0x01;
    pMUXMsg->StopNwInterfaceReq.TLVLength = cpu_to_le16(0x04);
    pMUXMsg->StopNwInterfaceReq.Handle = cpu_to_le32(WdsConnectionIPv4Handle);
    return sizeof(QMIWDS_STOP_NETWORK_INTERFACE_REQ_MSG);
}

static USHORT WdaSetDataFormat(PQMUX_MSG pMUXMsg, void *arg) {
    PQMIWDS_ADMIN_SET_DATA_FORMAT_TLV_QOS pWdsAdminQosTlv;
    PQMIWDS_ADMIN_SET_DATA_FORMAT_TLV linkProto;
    PQMIWDS_ADMIN_SET_DATA_FORMAT_TLV dlTlp;

    pWdsAdminQosTlv = (PQMIWDS_ADMIN_SET_DATA_FORMAT_TLV_QOS)(&pMUXMsg->QMUXMsgHdr + 1);
    pWdsAdminQosTlv->TLVType = 0x10;
    pWdsAdminQosTlv->TLVLength = cpu_to_le16(0x0001);
    pWdsAdminQosTlv->QOSSetting = 0; /* no-QOS header */
    
    linkProto = (PQMIWDS_ADMIN_SET_DATA_FORMAT_TLV)(pWdsAdminQosTlv + 1);
    linkProto->TLVType = 0x11;
    linkProto->TLVLength = cpu_to_le16(4);
    linkProto->Value = cpu_to_le32(0x01);     /* Set Ethernet  mode */

    dlTlp = (PQMIWDS_ADMIN_SET_DATA_FORMAT_TLV)(linkProto + 1);;
    dlTlp->TLVType = 0x13;
    dlTlp->TLVLength = cpu_to_le16(4);
    dlTlp->Value = cpu_to_le32(0x00);

    if (sizeof(*linkProto) != 7 )
        dbg_time("%s sizeof(*linkProto) = %d, is not 7!", __func__, sizeof(*linkProto) );

    return sizeof(QCQMUX_MSG_HDR) + sizeof(*pWdsAdminQosTlv) + sizeof(*linkProto) + sizeof(*dlTlp);
}

static char *qstrcpy(char *to, const char *from) { //no __strcpy_chk
	char *save = to;
	for (; (*to = *from) != '\0'; ++from, ++to);
	return(save);
}

static USHORT DmsUIMVerifyPinReqSend(PQMUX_MSG pMUXMsg, void *arg) {
    pMUXMsg->UIMVerifyPinReq.TLVType = 0x01;
    pMUXMsg->UIMVerifyPinReq.PINID = 0x01;
    pMUXMsg->UIMVerifyPinReq.PINLen = strlen((const char *)arg);
    qstrcpy((PCHAR)&pMUXMsg->UIMVerifyPinReq.PINValue, ((const char *)arg));
    pMUXMsg->UIMVerifyPinReq.TLVLength = cpu_to_le16(2 + strlen((const char *)arg));
    return sizeof(QMIDMS_UIM_VERIFY_PIN_REQ_MSG) + (strlen((const char *)arg) - 1);
}  

static USHORT WdsGetDefaultSettingsReqSend(PQMUX_MSG pMUXMsg, void *arg) {
   pMUXMsg->GetDefaultSettingsReq.Length = cpu_to_le16(sizeof(QMIWDS_GET_DEFAULT_SETTINGS_REQ_MSG) - 4);
   pMUXMsg->GetDefaultSettingsReq.TLVType = 0x01;
   pMUXMsg->GetDefaultSettingsReq.TLVLength = cpu_to_le16(0x01);
   pMUXMsg->GetDefaultSettingsReq.ProfileType = 0x00;
   return sizeof(QMIWDS_GET_DEFAULT_SETTINGS_REQ_MSG);
}

static USHORT WdsModifyProfileSettingsReq(PQMUX_MSG pMUXMsg, void *arg) {
    PQMIWDS_AUTH_PREFERENCE pAuthPref;
    PQMIWDS_USERNAME pUserName;
    PQMIWDS_PASSWD pPasswd;
    PQMIWDS_APNNAME pApnName;
    USHORT TLVLength = 0;
    UCHAR *pTLV;
    const char *apn = NULL;
    const char *user = NULL;
    const char *password = NULL;
    const char *auth = NULL;  // 0 ~ None, 1 ~ Pap, 2 ~ Chap, 3 ~ MsChapV2

    if (arg != NULL) {
        if (((const char **)arg)[0] != NULL)
            apn = ((const char **)arg)[0];
        if (((const char **)arg)[1] != NULL)
            user = ((const char **)arg)[1];
        if (((const char **)arg)[2] != NULL)
            password = ((const char **)arg)[2];
        if (((const char **)arg)[3] != NULL)
            auth = ((const char **)arg)[3];
    }
    
    pMUXMsg->ModifyProfileSettingsReq.Length = cpu_to_le16(sizeof(QMIWDS_MODIFY_PROFILE_SETTINGS_REQ_MSG) - 4);
    pMUXMsg->ModifyProfileSettingsReq.TLVType = 0x01;
    pMUXMsg->ModifyProfileSettingsReq.TLVLength = cpu_to_le16(0x02);
    pMUXMsg->ModifyProfileSettingsReq.ProfileType = 0x00;
    pMUXMsg->ModifyProfileSettingsReq.ProfileIndex = 0x01;

    pTLV = (UCHAR *)(&pMUXMsg->ModifyProfileSettingsReq + 1);

    // Set APN Name
    pApnName = (PQMIWDS_APNNAME)(pTLV + TLVLength);
    pApnName->TLVType = 0x14;
    if (apn && apn[0]) {
        pApnName->TLVLength = cpu_to_le16(strlen(apn));
        qstrcpy((char *)&pApnName->ApnName, apn);
    } else {
        pApnName->TLVLength = cpu_to_le16(0x0000);
    }          
    TLVLength +=(le16_to_cpu(pApnName->TLVLength) + sizeof(QCQMICTL_TLV_HDR));

    // Set User Name
    pUserName = (PQMIWDS_USERNAME)(pTLV + TLVLength);
    pUserName->TLVType = 0x1B;
    if (user && user[0]) {
        pUserName->TLVLength = cpu_to_le16(strlen(user));
        qstrcpy((char *)&pUserName->UserName, user);
    } else {
        pUserName->TLVLength = cpu_to_le16(0x0000);
    }
    TLVLength += (le16_to_cpu(pUserName->TLVLength) + sizeof(QCQMICTL_TLV_HDR));

    // Set Password 
    pPasswd = (PQMIWDS_PASSWD)(pTLV + TLVLength);
    pPasswd->TLVType = 0x1C;
    if (password && password[0]) {
        pPasswd->TLVLength = cpu_to_le16(strlen(password));
        qstrcpy((char *)&pPasswd->Passwd, password);
    } else {
        pPasswd->TLVLength = cpu_to_le16(0x0000);
    }       
    TLVLength +=(le16_to_cpu(pPasswd->TLVLength) + sizeof(QCQMICTL_TLV_HDR));


    // Set Auth Protocol 
    pAuthPref = (PQMIWDS_AUTH_PREFERENCE)(pTLV + TLVLength);
    pAuthPref->TLVType = 0x1D;
    pAuthPref->TLVLength = cpu_to_le16(0x01);
    if (auth && ('0' <= auth[0]) && (auth[0] <= '3')) {
        pAuthPref->AuthPreference = auth[0] - '0'; // 0 ~ None, 1 ~ Pap, 2 ~ Chap, 3 ~ MsChapV2
    } else {
        pAuthPref->AuthPreference = 0; // 0 ~ None, 1 ~ Pap, 2 ~ Chap, 3 ~ MsChapV2
    }
    TLVLength += (le16_to_cpu(pAuthPref->TLVLength) + sizeof(QCQMICTL_TLV_HDR));

    return sizeof(QMIWDS_MODIFY_PROFILE_SETTINGS_REQ_MSG) + TLVLength;
}

static PQCQMIMSG s_pRequest;
static PQCQMIMSG s_pResponse;
static pthread_mutex_t s_commandmutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t s_commandcond = PTHREAD_COND_INITIALIZER;

static int is_response(const PQCQMIMSG pRequest, const PQCQMIMSG pResponse) {
    if ((pRequest->QMIHdr.QMIType == pResponse->QMIHdr.QMIType)
        && (pRequest->QMIHdr.ClientId == pResponse->QMIHdr.ClientId)) {
            USHORT requestTID, responseTID;
        if (pRequest->QMIHdr.QMIType == QMUX_TYPE_CTL) {
            requestTID = pRequest->CTLMsg.QMICTLMsgHdr.TransactionId;                        
            responseTID = pResponse->CTLMsg.QMICTLMsgHdr.TransactionId;                        
        } else {
            requestTID = le16_to_cpu(pRequest->MUXMsg.QMUXHdr.TransactionId);                        
            responseTID = le16_to_cpu(pResponse->MUXMsg.QMUXHdr.TransactionId);     
        }
        return (requestTID == responseTID);
    }
    return 0;
}

int QmiThreadSendQMITimeout(PQCQMIMSG pRequest, PQCQMIMSG *ppResponse, unsigned msecs) {
    int ret;

    pthread_mutex_lock(&s_commandmutex);

    if (ppResponse)
        *ppResponse = NULL;

    if (!pRequest) {
        return -EINVAL;
    }
    dump_qmi(pRequest, le16_to_cpu(pRequest->QMIHdr.Length) + 1);

    s_pRequest = pRequest;
    s_pResponse = NULL;

    if (!strncmp(qmichannel, "/dev/qcqmi", strlen("/dev/qcqmi")))
        ret = GobiNetSendQMI(pRequest);
    else
        ret = QmiWwanSendQMI(pRequest);
    
    if (ret == 0) {
        ret = pthread_cond_timeout_np(&s_commandcond, &s_commandmutex, msecs);
        if (!ret) {
            if (s_pResponse && ppResponse) {
                *ppResponse = s_pResponse;
            } else {
                if (s_pResponse) {
                    free(s_pResponse);
                    s_pResponse = NULL;
                }
            }
        } else {
            dbg_time("%s pthread_cond_timeout_np=%d, errno: %d (%s)", __func__, ret, errno, strerror(errno));
        }
    }
    
    pthread_mutex_unlock(&s_commandmutex);

    return ret;
}

int QmiThreadSendQMI(PQCQMIMSG pRequest, PQCQMIMSG *ppResponse) {
    return QmiThreadSendQMITimeout(pRequest, ppResponse, 30 * 1000);
}

void QmiThreadRecvQMI(PQCQMIMSG pResponse) {
    pthread_mutex_lock(&s_commandmutex);
    if (pResponse == NULL) {
        if (s_pRequest) {
            free(s_pRequest);
            s_pRequest = NULL;
            s_pResponse = NULL;
            pthread_cond_signal(&s_commandcond);
        }
        pthread_mutex_unlock(&s_commandmutex);
        return;
    }
    dump_qmi(pResponse, le16_to_cpu(pResponse->QMIHdr.Length) + 1);             
    if (s_pRequest && is_response(s_pRequest, pResponse)) {
        free(s_pRequest);
        s_pRequest = NULL;
        s_pResponse = malloc(le16_to_cpu(pResponse->QMIHdr.Length) + 1);
        if (s_pResponse != NULL) {
            memcpy(s_pResponse, pResponse, le16_to_cpu(pResponse->QMIHdr.Length) + 1);
        }
        pthread_cond_signal(&s_commandcond);
    } else if ((pResponse->QMIHdr.QMIType == QMUX_TYPE_NAS) && (!le16_to_cpu(pResponse->MUXMsg.QMUXHdr.TransactionId))
                    && (le16_to_cpu(pResponse->MUXMsg.QMUXMsgHdrResp.Type) == QMINAS_SERVING_SYSTEM_IND)) {
        qmidevice_send_event_to_main(RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED);
    } else if ((pResponse->QMIHdr.QMIType == QMUX_TYPE_WDS) && (!le16_to_cpu(pResponse->MUXMsg.QMUXHdr.TransactionId))
                    && (le16_to_cpu(pResponse->MUXMsg.QMUXMsgHdrResp.Type) == QMIWDS_GET_PKT_SRVC_STATUS_IND)) {
        qmidevice_send_event_to_main(RIL_UNSOL_DATA_CALL_LIST_CHANGED);
    } else {
        if (debug_qmi)
            dbg_time("nobody care this qmi msg!!");
    }
    pthread_mutex_unlock(&s_commandmutex);
}

int requestSetEthMode(void) {
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    int err;

    pRequest = ComposeQMUXMsg(QMUX_TYPE_WDS_ADMIN, QMIWDS_ADMIN_SET_DATA_FORMAT_REQ, WdaSetDataFormat, NULL);
    err = QmiThreadSendQMI(pRequest, &pResponse);
    
    if (err < 0 || pResponse == NULL) {
        dbg_time("%s err = %d", __func__, err);
        return err;
    }

    pMUXMsg = &pResponse->MUXMsg;
    if (le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult) || le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError)) {
        dbg_time("%s QMUXResult = 0x%x, QMUXError = 0x%x", __func__,
            le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult), le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError));
        return le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError);       
    }

    free(pResponse);
    return 0;       
}

int requestGetPINStatus(SIM_Status *pSIMStatus) {
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    int err;
    PQMIDMS_UIM_PIN_STATUS pPin1Status = NULL;
    //PQMIDMS_UIM_PIN_STATUS pPin2Status = NULL;

    pRequest = ComposeQMUXMsg(QMUX_TYPE_DMS, QMIDMS_UIM_GET_PIN_STATUS_REQ, NULL, NULL);
    err = QmiThreadSendQMI(pRequest, &pResponse);

    if (err < 0 || pResponse == NULL) {
        dbg_time("%s err = %d", __func__, err);
        return err;
    }

    pMUXMsg = &pResponse->MUXMsg;
    if (le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult) || le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError)) {
        dbg_time("%s QMUXResult = 0x%x, QMUXError = 0x%x", __func__,
            le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult), le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError));
        return le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError);       
    }

    pPin1Status = (PQMIDMS_UIM_PIN_STATUS)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x11);
    //pPin2Status = (PQMIDMS_UIM_PIN_STATUS)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x12);

    if (pPin1Status != NULL) {
        if (pPin1Status->PINStatus == QMI_PIN_STATUS_NOT_VERIF) {
            *pSIMStatus = SIM_PIN;
        } else if (pPin1Status->PINStatus == QMI_PIN_STATUS_BLOCKED) {
            *pSIMStatus = SIM_PUK;
        } else if (pPin1Status->PINStatus == QMI_PIN_STATUS_PERM_BLOCKED) {
            *pSIMStatus = SIM_BAD;
        }
    }

    free(pResponse);
    return 0;       
}


int requestGetSIMStatus(SIM_Status *pSIMStatus) { //RIL_REQUEST_GET_SIM_STATUS
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    int err;

__requestGetSIMStatus:
    pRequest = ComposeQMUXMsg(QMUX_TYPE_DMS, QMIDMS_UIM_GET_STATE_REQ, NULL, NULL);
    err = QmiThreadSendQMI(pRequest, &pResponse);

    if (err < 0 || pResponse == NULL) {
        dbg_time("%s err = %d", __func__, err);
        return err;
    }

    pMUXMsg = &pResponse->MUXMsg;
    if (le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult) || le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError)) {
        dbg_time("%s QMUXResult = 0x%x, QMUXError = 0x%x", __func__,
            le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult), le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError));
        if (QMI_ERR_OP_DEVICE_UNSUPPORTED == le16_to_cpu(pResponse->MUXMsg.QMUXMsgHdrResp.QMUXError)) {
            sleep(1);
            goto __requestGetSIMStatus;
        }
        return le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError);       
    }

//UIM state. Values:
// 0x00  UIM initialization completed
// 0x01  UIM is locked or the UIM failed
// 0x02  UIM is not present
// 0x03  Reserved
// 0xFF  UIM state is currently
//unavailable
    if (pResponse->MUXMsg.UIMGetStateResp.UIMState == 0x00) {
        *pSIMStatus = SIM_READY;
    } else if (pResponse->MUXMsg.UIMGetStateResp.UIMState == 0x01) {
        *pSIMStatus = SIM_ABSENT;
        err = requestGetPINStatus(pSIMStatus);
    } else if ((pResponse->MUXMsg.UIMGetStateResp.UIMState == 0x02) || (pResponse->MUXMsg.UIMGetStateResp.UIMState == 0xFF)) {
        *pSIMStatus = SIM_ABSENT;
    } else {
        *pSIMStatus = SIM_ABSENT;
    }
    dbg_time("%s SIMStatus: %s", __func__, (*pSIMStatus == SIM_READY) ? "SIM_READY" : "SIM_ABSENT");

    free(pResponse);
    return 0;       
}

int requestEnterSimPin(const CHAR *pPinCode) {
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    int err;

    pRequest = ComposeQMUXMsg(QMUX_TYPE_DMS, QMIDMS_UIM_VERIFY_PIN_REQ, DmsUIMVerifyPinReqSend, (void *)pPinCode);
    err = QmiThreadSendQMI(pRequest, &pResponse);

    if (err < 0 || pResponse == NULL) {
        dbg_time("%s err = %d", __func__, err);
        return err;
    }

    pMUXMsg = &pResponse->MUXMsg;
    if (le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult) || le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError)) {
        dbg_time("%s QMUXResult = 0x%x, QMUXError = 0x%x", __func__,
            le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult), le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError));
        return le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError);       
    }

    free(pResponse);
    return 0;       
}

int requestRegistrationState(UCHAR *pPSAttachedState) {
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    int err;
    PQMINAS_CURRENT_PLMN_MSG pCurrentPlmn;
    PSERVING_SYSTEM pServingSystem;
    PQMINAS_DATA_CAP pDataCap;
    USHORT MobileCountryCode = 0;
    USHORT MobileNetworkCode = 0;
    const char *pDataCapStr = "UNKNOW"; 
    
    pRequest = ComposeQMUXMsg(QMUX_TYPE_NAS, QMINAS_GET_SERVING_SYSTEM_REQ, NULL, NULL);
    err = QmiThreadSendQMI(pRequest, &pResponse);
    
    if (err < 0 || pResponse == NULL) {
        dbg_time("%s err = %d", __func__, err);
        return err;
    }

    pMUXMsg = &pResponse->MUXMsg;
    if (le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult) || le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError)) {
        dbg_time("%s QMUXResult = 0x%x, QMUXError = 0x%x", __func__,
            le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult), le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError));
        return le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError);       
    }

    pCurrentPlmn = (PQMINAS_CURRENT_PLMN_MSG)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x12);
    if (pCurrentPlmn) {
        MobileCountryCode = le16_to_cpu(pCurrentPlmn->MobileCountryCode);
        MobileNetworkCode = le16_to_cpu(pCurrentPlmn->MobileNetworkCode);           
    }

    *pPSAttachedState = 0;
    pServingSystem = (PSERVING_SYSTEM)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x01);
    if (pServingSystem) {
    //Packet-switched domain attach state of the mobile.
    //0x00    PS_UNKNOWN ?Unknown or not applicable
    //0x01    PS_ATTACHED ?Attached
    //0x02    PS_DETACHED ?Detached
        *pPSAttachedState = pServingSystem->RegistrationState;
        if (pServingSystem->RegistrationState == 0x01) //0x01 ¨C REGISTERED ¨C Registered with a network
            *pPSAttachedState  = pServingSystem->PSAttachedState;
        else {
            MobileCountryCode = MobileNetworkCode = 0;
            *pPSAttachedState  = 0x02;
        }
    }

    pDataCap = (PQMINAS_DATA_CAP)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x011);
    if (pDataCap && pDataCap->DataCapListLen) {
        switch (pDataCap->DataCap) {
             case 0x01: pDataCapStr = "GPRS"; break;
             case 0x02: pDataCapStr = "EDGE"; break;
             case 0x03: pDataCapStr = "HSDPA"; break;
             case 0x04: pDataCapStr = "HSUPA"; break;
             case 0x05: pDataCapStr = "UMTS"; break;
             case 0x06: pDataCapStr = "1XRTT"; break;
             case 0x07: pDataCapStr = "1XEVDO"; break;
             case 0x08: pDataCapStr = "1XEVDO_REVA"; break;
             case 0x09: pDataCapStr = "GPRS"; break;
             case 0x0A: pDataCapStr = "1XEVDO_REVB"; break;
             case 0x0B: pDataCapStr = "LTE"; break;
             case 0x0C: pDataCapStr = "HSDPA"; break;
             case 0x0D: pDataCapStr = "HSDPA"; break;
             default: pDataCapStr = "UNKNOW"; break;
        }
    }

    dbg_time("%s MCC: %d, MNC: %d, PS: %s, DataCap: %s", __func__, 
        MobileCountryCode, MobileNetworkCode, (*pPSAttachedState == 1) ? "Attached" : "Detached" , pDataCapStr); 

    free(pResponse);
    return 0;       
}

int requestQueryDataCall(UCHAR  *pConnectionStatus) {
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    int err;
    PQMIWDS_PKT_SRVC_TLV pPktSrvc;

    pRequest = ComposeQMUXMsg(QMUX_TYPE_WDS, QMIWDS_GET_PKT_SRVC_STATUS_REQ, NULL, NULL);
    err = QmiThreadSendQMI(pRequest, &pResponse);
    
    if (err < 0 || pResponse == NULL) {
        dbg_time("%s err = %d", __func__, err);
        return err;
    }

    pMUXMsg = &pResponse->MUXMsg;
    if (le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult) || le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError)) {
        dbg_time("%s QMUXResult = 0x%x, QMUXError = 0x%x", __func__,
            le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult), le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError));
        return le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError);       
    }

    *pConnectionStatus = QWDS_PKT_DATA_DISCONNECTED;
    pPktSrvc = (PQMIWDS_PKT_SRVC_TLV)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x01);
    if (pPktSrvc) {
        *pConnectionStatus = pPktSrvc->ConnectionStatus;
        if ((le16_to_cpu(pPktSrvc->TLVLength) == 2) && (pPktSrvc->ReconfigReqd == 0x01))
            *pConnectionStatus = QWDS_PKT_DATA_DISCONNECTED;
    }

    if (*pConnectionStatus == QWDS_PKT_DATA_DISCONNECTED)
        WdsConnectionIPv4Handle = 0;

    dbg_time("%s ConnectionStatus: %s", __func__, (*pConnectionStatus == QWDS_PKT_DATA_CONNECTED) ? "CONNECTED" : "DISCONNECTED");

    free(pResponse);
    return 0;       
}

int requestSetupDataCall(void) {
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    int err;

    pRequest = ComposeQMUXMsg(QMUX_TYPE_WDS, QMIWDS_START_NETWORK_INTERFACE_REQ, WdsStartNwInterfaceReq, NULL);
    err = QmiThreadSendQMITimeout(pRequest, &pResponse, 120 * 1000);
    
    if (err < 0 || pResponse == NULL) {
        dbg_time("%s err = %d", __func__, err);
        return err;
    }

    pMUXMsg = &pResponse->MUXMsg;
    if (le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult) || le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError)) {
        dbg_time("%s QMUXResult = 0x%x, QMUXError = 0x%x", __func__,
            le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult), le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError));
        return le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError);       
    }

    WdsConnectionIPv4Handle = le32_to_cpu(pResponse->MUXMsg.StartNwInterfaceResp.Handle);
    dbg_time("%s WdsConnectionIPv4Handle: 0x%08x", __func__, WdsConnectionIPv4Handle);

    free(pResponse);
    return 0;       
}

int requestDeactivateDefaultPDP(void) {
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    int err;

    pRequest = ComposeQMUXMsg(QMUX_TYPE_WDS, QMIWDS_STOP_NETWORK_INTERFACE_REQ , WdsStopNwInterfaceReq, NULL);
    err = QmiThreadSendQMI(pRequest, &pResponse);
    
    if (err < 0 || pResponse == NULL) {
        dbg_time("%s err = %d", __func__, err);
        return err;
    }

    pMUXMsg = &pResponse->MUXMsg;
    if (le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult) || le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError)) {
        dbg_time("%s QMUXResult = 0x%x, QMUXError = 0x%x", __func__,
            le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult), le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError));
        return le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError);       
    }

    WdsConnectionIPv4Handle = 0;
    free(pResponse);
    return 0;       
}

int requestSetProfile(const char *apn,  const char *user, const char *password, const char *auth) {
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    int err;
    const char *argv[] = {apn, user, password, auth, NULL};

    pRequest = ComposeQMUXMsg(QMUX_TYPE_WDS, QMIWDS_MODIFY_PROFILE_SETTINGS_REQ, WdsModifyProfileSettingsReq, argv);
    err = QmiThreadSendQMI(pRequest, &pResponse);
    
    if (err < 0 || pResponse == NULL) {
        dbg_time("%s err = %d", __func__, err);
        return err;
    }

    pMUXMsg = &pResponse->MUXMsg;
    if (le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult) || le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError)) {
        dbg_time("%s QMUXResult = 0x%x, QMUXError = 0x%x", __func__,
            le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult), le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError));
        return le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError);       
    }

    free(pResponse);
    return 0;       
}

int requestGetProfile(const char **pp_apn,  const char **pp_user, const char **pp_password, const char **pp_auth) {
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    int err;
    char *apn = NULL;
    char *user = NULL;
    char *password = NULL;
    char *auth = NULL;
    PQMIWDS_APNNAME pApnName;
    PQMIWDS_USERNAME pUserName;
    PQMIWDS_PASSWD pPassWd;
    PQMIWDS_AUTH_PREFERENCE pAuthPref;

    if (pp_apn) *pp_apn = NULL;
    if (pp_user) *pp_user = NULL;
    if (pp_password) *pp_password = NULL;
    if (pp_auth) *pp_auth = NULL;

    pRequest = ComposeQMUXMsg(QMUX_TYPE_WDS, QMIWDS_GET_DEFAULT_SETTINGS_REQ, WdsGetDefaultSettingsReqSend, NULL);
    err = QmiThreadSendQMI(pRequest, &pResponse);
    
    if (err < 0 || pResponse == NULL) {
        dbg_time("%s err = %d", __func__, err);
        return err;
    }

    pMUXMsg = &pResponse->MUXMsg;
    if (le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult) || le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError)) {
        dbg_time("%s QMUXResult = 0x%x, QMUXError = 0x%x", __func__,
            le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult), le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError));
        return le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError);       
    }

    pApnName = (PQMIWDS_APNNAME)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x14);
    pUserName = (PQMIWDS_USERNAME)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x1B);
    pPassWd = (PQMIWDS_PASSWD)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x1C);
    pAuthPref = (PQMIWDS_AUTH_PREFERENCE)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x1D);

    if (pApnName && le16_to_cpu(pApnName->TLVLength))
        apn = strndup((const char *)(&pApnName->ApnName), le16_to_cpu(pApnName->TLVLength));
    if (pUserName && pUserName->UserName)
        user = strndup((const char *)(&pUserName->UserName), le16_to_cpu(pUserName->TLVLength));
    if (pPassWd && le16_to_cpu(pPassWd->TLVLength))
        password = strndup((const char *)(&pPassWd->Passwd), le16_to_cpu(pPassWd->TLVLength));
    if (pAuthPref && le16_to_cpu(pAuthPref->TLVLength)) {
        auth = malloc(2);
        auth[0] = '0' + pAuthPref->AuthPreference;
        auth[1] = 0;
    }

    if (pp_apn) *pp_apn = apn;
    if (pp_user) *pp_user = user;
    if (pp_password) *pp_password = password;
    if (pp_auth) *pp_auth = auth;

    dbg_time("%s %s/%s/%s/%s", __func__, apn, user, password, auth);

    free(pResponse);
    return 0;       
}

int requestBaseBandVersion(const char **pp_reversion) {
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    PDEVICE_REV_ID revId;
    int err;

    if (pp_reversion) *pp_reversion = NULL;

    pRequest = ComposeQMUXMsg(QMUX_TYPE_DMS, QMIDMS_GET_DEVICE_REV_ID_REQ, NULL, NULL);
    err = QmiThreadSendQMI(pRequest, &pResponse);
    
    if (err < 0 || pResponse == NULL) {
        dbg_time("%s err = %d", __func__, err);
        return err;
    }

    pMUXMsg = &pResponse->MUXMsg;
    if (le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult) || le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError)) {
        dbg_time("%s QMUXResult = 0x%x, QMUXError = 0x%x", __func__,
            le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult), le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError));
        return le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError);       
    }

    revId = (PDEVICE_REV_ID)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x01);

    if (revId && le16_to_cpu(revId->TLVLength)) {
        char *DeviceRevisionID = strndup((const char *)(&revId->RevisionID), le16_to_cpu(revId->TLVLength));
        dbg_time("%s %s", __func__, DeviceRevisionID);
        if (pp_reversion) *pp_reversion = DeviceRevisionID;
    }

    free(pResponse);
    return 0;       
}
