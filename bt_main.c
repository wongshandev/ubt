#if defined(__BLUE_TRCK__)
#include "bt_ps.h"
#include "bt_main.h"
#include "dcl_adc.h"


static U8 btGetIMSIrsq(void * msg_ptr)
{
    mmi_smu_get_imsi_rsp_struct *imsi_rsp = NULL;
    
    mmi_frm_clear_protocol_event_handler(MSG_ID_MMI_SMU_GET_IMSI_RSP, btGetIMSIrsq);
    
    /* convert the msg_ptr to imsi_rsp type */
    imsi_rsp = (mmi_smu_get_imsi_rsp_struct *)msg_ptr;

    /* check the response is correct or not */
    if (KAL_TRUE == imsi_rsp->result)
    {
        /* get the IMSI number successful */
       bt_print("imsi rsp:%s",(S8*) imsi_rsp->imsi);
       strcpy(gBtps.btIMSI,(S8*) imsi_rsp->imsi);
    }
    else
    btGetIMSIreq();
    return 0;
}
void btGetIMSIreq(void)
{
	ilm_struct *ilm_ptr = NULL;

	if (!mmi_bootup_is_sim_valid())
	{
		bt_print("No Sim1,Dont req Imsi");
		return;
	}
	mmi_frm_set_protocol_event_handler(MSG_ID_MMI_SMU_GET_IMSI_RSP, btGetIMSIrsq, MMI_TRUE);
	ilm_ptr = allocate_ilm(MOD_MMI);
	ilm_ptr->msg_id         = MSG_ID_MMI_SMU_GET_IMSI_REQ;
	ilm_ptr->local_para_ptr = (local_para_struct *) NULL;
	ilm_ptr->peer_buff_ptr  = (peer_buff_struct *) NULL;
	SEND_ILM(MOD_MMI, MOD_L4C, MMI_L4C_SAP, ilm_ptr);
}
void  btGetIMEIRsp(void * msg_ptr)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    mmi_nw_get_imei_rsp_struct  * imei_rsp;
    
    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    ClearProtocolEventHandler(MSG_ID_MMI_NW_GET_IMEI_RSP);
    imei_rsp = (mmi_nw_get_imei_rsp_struct *)msg_ptr;
    if (KAL_TRUE == imei_rsp->result)
    {
		bt_print("IMEI rsp:%s",(S8*) imei_rsp->imei);
		memset(gBtps.btIMEI, 0, sizeof(gBtps.btIMEI));
		strcpy(gBtps.btIMEI,(S8*) imei_rsp->imei);
    }
    else
    btGetIMEIreq();
}
void btGetIMEIreq(void)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    ilm_struct  * ilm_ptr = NULL;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    /* set imei response handler */
    SetProtocolEventHandler(btGetIMEIRsp, MSG_ID_MMI_NW_GET_IMEI_RSP);

    /* allocate the ilm struct memory */
    ilm_ptr = allocate_ilm(MOD_MMI);

    /* fill the member of the ilm struct */
    ilm_ptr->msg_id         = MSG_ID_MMI_NW_GET_IMEI_REQ;
    ilm_ptr->local_para_ptr = (local_para_struct *) NULL;
    ilm_ptr->peer_buff_ptr  = (peer_buff_struct *) NULL;

    /* send the message */
    SEND_ILM(MOD_MMI, MOD_L4C, MMI_L4C_SAP, ilm_ptr);
}

void bt_get_base_loc_rsp(void *info)
{
    mmi_em_status_ind_struct *msg = (mmi_em_status_ind_struct*) info;
    int count = 0;
	 
	switch (msg->em_info)
    {
        case RR_EM_LAI_INFO:
        {
        	int *pIndex = &gBtps.loc_index;
            rr_em_lai_info_struct *pack_ptr;
            kal_uint16 mm_pdu_len;

			SetProtocolEventHandler(NULL, MSG_ID_MMI_EM_UPDATE_RSP);
			ClearProtocolEventHandler(MSG_ID_MMI_EM_STATUS_IND); 
	        /* Get the msg->info where MM peer msg will be stored for transmission */
    		pack_ptr = (rr_em_lai_info_struct*) get_pdu_ptr(msg->info, &mm_pdu_len);

			if(gBtps.loc_index >= MAX_LOC_COUNT)
			{
				gBtps.loc_index = 0;
				memset(gBtps.loc,0,sizeof(gBtps.loc));
			}
			gBtps.loc_index = 0;
			memcpy((void*)gBtps.loc[gBtps.loc_index].mcc, (void*)pack_ptr->mcc, 3);
			memcpy((void*)gBtps.loc[gBtps.loc_index].mnc, (void*)pack_ptr->mnc, 3);
			memcpy((void*)gBtps.loc[gBtps.loc_index].lac, (void*)pack_ptr->lac, 2);
			gBtps.loc[gBtps.loc_index].rac= pack_ptr->rac;
			gBtps.loc[gBtps.loc_index].cell_id = pack_ptr->cell_id;
		//gBtps.loc_index++;
            break;
        }
        default:
            break;
    }
	 free_peer_buff(msg->info);
	 return;
}
void bt_get_base_loc_req(void)
{
	MYQUEUE Message;
	S32 i;
	mmi_em_update_req_struct *em_start_req=NULL;

	em_start_req = (mmi_em_update_req_struct *)OslConstructDataPtr(sizeof(mmi_em_update_req_struct));
	if(em_start_req == NULL)
	{
		return;
	}
	for (i=0;i<EM_INFO_REQ_NUM;i++)
	{
	  em_start_req->info_request[i] = EM_NC;
	}
	em_start_req->info_request[RR_EM_LAI_INFO]=EM_ON;
	Message.oslMsgId = MSG_ID_MMI_EM_UPDATE_REQ;
	Message.oslDataPtr = (oslParaType*) em_start_req;
	Message.oslPeerBuffPtr = NULL;
	Message.oslSrcId = MOD_MMI;
	Message.oslDestId = MOD_L4C;
	SetProtocolEventHandler(NULL, MSG_ID_MMI_EM_UPDATE_RSP);
	SetProtocolEventHandler((PsIntFuncPtr)bt_get_base_loc_rsp, MSG_ID_MMI_EM_STATUS_IND); 
	OslMsgSendExtQueue(&Message);
}
void bt_set_rssi_in_qdBm(kal_int32 rssiVaule)
{
	gBtps.rssi_in_qdbm = rssiVaule;
}
extern void mmi_Serving_Cell_Info_start_req(S32 module_ID)  ;
extern kal_int32            t_vbat_temp;
extern kal_bool BMT_Current_Voltage(DCL_ADC_CHANNEL_TYPE_ENUM ch, kal_uint32 *voltage, double *adc_value);
extern kal_bool bmt_get_adc_channel_voltage(DCL_ADC_CHANNEL_TYPE_ENUM ch, kal_uint32 *voltage);

void btGetLOCreq(void)
{
	// 3秒一次更新LOC
	//bt_get_base_loc_req();
	//mmi_Serving_Cell_Info_start_req(1);

	kal_uint32 voltage=0;
	kal_uint32 tem_value=0;


    //bmt_get_adc_channel_voltage(DCL_VBAT_ADC_CHANNEL, &voltage);

	//StartTimer(BT_GET_LOC_TIMER_ID,1000*3,btGetLOCreq);
}
extern bt_struct gBtps;
extern void uart_init(void);

void btEntryMainPro(void)
{
	//bt_gps_parse_loop();
	//bt_ublox_agps_get_host_ip();
	//#ifndef WIN32
	//btGetLOCreq();
	//#endif
	//btGetIMEIreq();

   //Init UART
   //uart_init();
}


#endif
