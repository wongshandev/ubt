
#define __YEELINK_APP__

#ifdef __YEELINK_APP__
#include "YeeLink.h"
#include "bt_ps.h"
#include "bt_gps.h"
extern void bt_ublox_put_data(kal_uint8 *_data);
extern char * ubloxAssitBuff;
YeeLinkStruct gYeeLinkSoc={0};
extern btGPSInfo GPSInfo;
extern U8 * soc_rev_buff;
extern char *risendBuff;
void bt_get_imsi_req(void);
kal_uint32 YeeLink_GetGPRSAccount(void)
{
	kal_uint32 data_account;
	srv_dtcnt_sim_type_enum sim_id = SRV_DTCNT_SIM_TYPE_1;

	srv_dtcnt_get_sim_preference(&sim_id);
	cbm_register_app_id(&gYeeLinkSoc.app_id);
	data_account =cbm_encode_data_account_id(CBM_DEFAULT_ACCT_ID, (cbm_sim_id_enum)sim_id, gYeeLinkSoc.app_id, MMI_FALSE);

	return data_account;
}
extern void atcmd_set_rev_packet_heard(U8 *ptr,U32 lenth);
extern U32 soc_rev_lenth;
static void YeeLink_Notify_Internal(void* Content)
{
	#define YEELINK_MAX_RCV_BUFFER_SIZE (1024+512)
	YeeLinkStruct *p = &gYeeLinkSoc;
	app_soc_notify_ind_struct *soc_notify = (app_soc_notify_ind_struct *) Content;
	S32 ret = 0;
	YEELINK_SOC_CONECT_RESULT soc_result;
	if(p->Soc_hand < 0)
	{
		p->CallBack(soc_result,(void *)ubloxAssitBuff);
		soc_result = YEELINK_FAIL;
		return;
	}
	switch (soc_notify->event_type)
	{
		case SOC_WRITE:
		{
			//写
		}
		break;
		case SOC_READ:
		{
			char *data_section = NULL;
			int buffer_len=0;
			char tmpHeard[256]={0};
			if(soc_rev_buff == NULL)
			{
				soc_rev_buff = (U8 *)med_alloc_ext_mem(MAX_SOC_BUFFER_SIZE*5);
			}
			soc_result = YEELINK_READ_DATA;
			
			//读
			do{
				ret = soc_recv(p->Soc_hand, (void *)(soc_rev_buff+buffer_len), YEELINK_MAX_RCV_BUFFER_SIZE, 0);
				if(ret>0)
				buffer_len +=  ret;
				//bt_print("收到[%d]->[%s]",ret,ubloxAssitBuff);
			}while((ret > 0));
			soc_rev_lenth = buffer_len;
			sprintf(tmpHeard,"+RIRD: 0,1,1,%d,",buffer_len);
			memmove(soc_rev_buff+strlen(tmpHeard),soc_rev_buff,buffer_len);
			memcpy(soc_rev_buff,tmpHeard,strlen(tmpHeard));
			soc_rev_lenth += strlen(tmpHeard);
			bt_print("rev:[%s]",soc_rev_buff);
		}
		break;
		case SOC_CONNECT:
		{
			soc_result = YEELINK_CONECT_SUCCEED;
			#if 0
			//连接上
			ret = soc_send(p->Soc_hand, (U8*) (p->SendBuffer), strlen(p->SendBuffer), 0);
			if(ret > 0)
			{
				//成功
				//p->CallBack(YEELINK_SEND_SUCCEED,NULL);
				soc_result = YEELINK_CONECT_SUCCEED;
				memset(p->SendBuffer,0,strlen(p->SendBuffer));
			}
			else
			{	//发送失败
				soc_result = YEELINK_RECONECT;
				SetProtocolEventHandler(YeeLink_Notify_Internal,MSG_ID_APP_SOC_NOTIFY_IND);
				//p->CallBack(YEELINK_FAIL,NULL);
			}
			#endif
			p->isActiveConnect = TRUE;
			bt_print("soc connect!");
		}
		break;
		case SOC_ACCEPT:
		{
			//接受
		}
		break;
		case SOC_CLOSE:
		{
			//关闭
			//p->CallBack(YEELINK_CLOSED,NULL);
			//p->CallBack=NULL;
			p->Soc_hand=-1;
			soc_result = YEELINK_CLOSED;
			p->isActiveConnect = FALSE;
			bt_print("soc colsed!");
		}
		default:
			break;
		break;
	}
	p->CallBack(soc_result,(void *)soc_rev_buff);

}
void YeeLink_Conect_Soc(sockaddr_struct Ip_addr,U8 * Content,Yeelink_Soc_Conect_CB Call_Back)
{
	#define VALID_SOCKET(s) (s>=0)
	kal_uint32 acct_id = 0;
	YeeLinkStruct *p = &gYeeLinkSoc;
	char ret;
	int intAddr[6]={0};

	//isConect = FALSE;
	p->CallBack=Call_Back;
	
	p->_pSendbuffer = Content;
	
	if(Ip_addr.addr[0] == 0)
	{
		//参数错误
		//bt_print("参数错误:ip(%d.%d)",Ip_addr.addr[0],Ip_addr.addr[1]);
		Call_Back(YEELINK_VAIL,NULL);
		return;
	}
	if(!VALID_SOCKET(p->Soc_hand))
	{
			U8 val = 1;
			if(gYeeLinkSoc.account_id == 0)
			gYeeLinkSoc.account_id = YeeLink_GetGPRSAccount();
			
			p->Soc_hand = soc_create(SOC_PF_INET, SOC_SOCK_STREAM, 0, MOD_MMI, gYeeLinkSoc.account_id);
			//bt_print("Ip_addr %d.%d.%d.%d,p->Soc_hand %d",Ip_addr.addr[0],Ip_addr.addr[1],Ip_addr.addr[2],Ip_addr.addr[3],p->Soc_hand);
			Ip_addr.addr_len = 4;
			Ip_addr.sock_type =SOC_SOCK_STREAM;
			if(p->Soc_hand >= 0)
			{
				if (soc_setsockopt(p->Soc_hand, SOC_NBIO, &val, sizeof(val)) < 0)
				{
					//错误
					return;
				}
				val = SOC_READ | SOC_WRITE | SOC_CLOSE | SOC_CONNECT;
				if (soc_setsockopt(p->Soc_hand, SOC_ASYNC, &val, sizeof(val)) < 0)
				{
					//错误
					return;
				}
				ret = soc_connect(p->Soc_hand, &Ip_addr);
				if(ret < 0)
				{
					Call_Back(YEELINK_FAIL,NULL);
					SetProtocolEventHandler(YeeLink_Notify_Internal,MSG_ID_APP_SOC_NOTIFY_IND);
					return;
				}
				else
				{
				//	ret = soc_send(p->Soc_hand, (U8*) (Content), strlen(Content), 0);
				}
			}
	}
	else
	{
		//直接发送
		if(Content != NULL)
		ret = soc_send(p->Soc_hand, (U8*) (Content), strlen(Content), 0);
		if(ret<0)
		{
			//错误
		}
		else
		{
			if(p->CallBack)
			p->CallBack(YEELINK_SEND_SUCCEED,(void *)Content);
		}
	}
	SetProtocolEventHandler(YeeLink_Notify_Internal,MSG_ID_APP_SOC_NOTIFY_IND);
}
BOOL YeeLink_GetSocketStatus(void)
{
	YeeLinkStruct *p = &gYeeLinkSoc;
	if((p->Soc_hand >= 0)&&(p->isActiveConnect == TRUE))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}

}
BOOL YeeLink_SetAddr(sockaddr_struct _addr)
{
	YeeLinkStruct *p = &gYeeLinkSoc;
	if((_addr.addr_len <= 0)||(_addr.port <= 0)||(_addr.addr[0]==0))
	return FALSE;

	p->addr2.addr[0]=_addr.addr[0];
	p->addr2.addr[1]=_addr.addr[1];
	p->addr2.addr[2]=_addr.addr[2];
	p->addr2.addr[3]=_addr.addr[3];

	p->addr2.port=_addr.port;
	p->addr2.sock_type=_addr.sock_type;
	p->addr2.addr_len=_addr.addr_len;

	return TRUE;
}
sockaddr_struct* YeeLink_GetAddr(void)
{
	YeeLinkStruct *p = &gYeeLinkSoc;

	return 	& p->addr2;
}

void YeeLink_CloseSocket(void)
{
	YeeLinkStruct *p = &gYeeLinkSoc;

	if(p->Soc_hand >= 0)
	soc_close(p->Soc_hand);

	p->Soc_hand = -1;
	p->isActiveConnect = FALSE;
	SetProtocolEventHandler(YeeLink_Notify_Internal,MSG_ID_APP_SOC_NOTIFY_IND);
}
//test
void YeeLinkDeviceBack(YEELINK_SOC_CONECT_RESULT result,void *data)
{
	//bt_print("回调状态 %d",result);

	if(result == YEELINK_READ_DATA)
	{
		if(data != NULL)
		{
			atcmd_CRLF_to_hex(data,soc_rev_lenth);
			bt_ublox_put_data((kal_uint8 *)data);
			med_free_ext_mem((void**)&data);
		}
	}
	else if(result == YEELINK_CONECT_SUCCEED)
	{
		bt_print("connected...");
		atcmd_put_data_string("CONNECT OK");
	}
	else if(result == YEELINK_SEND_SUCCEED)
	{
		if(data != NULL)
		{
			med_free_ext_mem((void**)&data);
			data = NULL;
		}
		atcmd_put_data_string("SEND OK");
	}
	else if(result == YEELINK_RECONECT)
	{
		//CONNECT FAIL
		atcmd_put_data_string("CONNECT FAIL");
	}
}
void YeeLink_SendData_Req(char *str_host,char *content)
{
		char CmdContent[150] = {0};
		char lat_lon_string[50] ={0};
		YeeLinkStruct *p = &gYeeLinkSoc;

		bt_print("http:%s",content);
		p->addr2.addr[0]=173;
		p->addr2.addr[1]=13;
		p->addr2.addr[2]=10;
		p->addr2.addr[3]=30;
		p->addr2.addr_len=4;
		p->addr2.port=5555;
		p->addr2.sock_type =SOC_SOCK_STREAM;
		YeeLink_Conect_Soc(p->addr2,content,YeeLinkDeviceBack);
		//简单的超时
		//StartTimer(BT_SOC_CONNECT_TIMER_OUT_ID, 1000*20, YeeLink_CloseSocket);
}
void YeeLink_GetHost_CallBack(void *InMsg)
{
	static count = 0;
	YeeLinkStruct *p = &gYeeLinkSoc;
	char str_host[20]={0};
	app_soc_get_host_by_name_ind_struct* dns_ind = (app_soc_get_host_by_name_ind_struct*)InMsg;

	if(!(count%10)&&(dns_ind->result >= 0))
	{
		sprintf(str_host,"%d.%d.%d.%d",dns_ind->addr[0],dns_ind->addr[1],dns_ind->addr[2],dns_ind->addr[3]);
		memcpy(&p->addr2.addr,&dns_ind->addr,sizeof(p->addr2.addr));
		//bt_print("解析成功:%s",str_host);
	}
	else
	{
		//bt_print("域名解析失败,再来一次");
	}
	count++;
}
void YeeLink_GetHostByName(const kal_uint8 *hostname,kal_uint8 * host)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    kal_int32 ret;
    kal_uint32 rcvd_counter = 0;

    ret = soc_gethostbyname(
            KAL_FALSE,
            MOD_MMI,
            1,
            (kal_char*) hostname,
            (kal_uint8*) host,
            (kal_uint8*) & rcvd_counter,
            0,
            YeeLink_GetGPRSAccount());
    if (ret == SOC_SUCCESS)
    {
		   //成功
		   //bt_print("解析成功%d.%d.%d.%d",host[0],host[1],host[2],host[3]);
    }
    else if (ret == SOC_WOULDBLOCK)
    {
        /* waits for APP_SOC_GET_HOST_BY_NAME_IND */
        mmi_frm_set_protocol_event_handler(MSG_ID_APP_SOC_GET_HOST_BY_NAME_IND, (PsIntFuncPtr)YeeLink_GetHost_CallBack, MMI_TRUE); 
    }
    else
    {
	//解析失败
		////bt_print("解析失败");
		//YeeLink_GetHostByName("api.yeelink.net",host);
		mmi_frm_set_protocol_event_handler(MSG_ID_APP_SOC_GET_HOST_BY_NAME_IND, (PsIntFuncPtr)YeeLink_GetHost_CallBack, MMI_TRUE); 
    }

}
U8 bt_imsi_rsp( void *inMsg )
{
    mmi_smu_get_imsi_rsp_struct *local_data = ( mmi_smu_get_imsi_rsp_struct* )inMsg;

    mmi_frm_clear_protocol_event_handler(MSG_ID_MMI_SMU_GET_IMSI_RSP, bt_imsi_rsp);
    if ( local_data->result == KAL_TRUE )
    {
    	const char card_kind[2][12] = {{"cucc"},{"cmcc"}};
    	char imsi[15] = {0};
    	int index;

    	if(strstr(( void* )((local_data->imsi )+1),"46000"))
    	{
			index = 1;
    	}
    	else
    	{
			index = 0;
    	}
    	strncpy(imsi,( void* )((local_data->imsi )+1),15);
    	//bt_print("You used[%s]IMSI:[%s]",card_kind[index],imsi);
    	strcpy(gBtps.btIMSI,imsi);

    }
	else
	{
		////bt_print("获取IMSI失败");
		bt_get_imsi_req();
	}
	return 0;
}
void bt_get_imsi_req(void)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    ilm_struct *ilm_ptr = NULL;
	if (!mmi_bootup_is_sim_valid())
	{
		//bt_print("No simcard");
		return;
	}
    mmi_frm_set_protocol_event_handler(MSG_ID_MMI_SMU_GET_IMSI_RSP, bt_imsi_rsp, MMI_TRUE);
    ilm_ptr = allocate_ilm(MOD_MMI);
    ilm_ptr->msg_id         = MSG_ID_MMI_SMU_GET_IMSI_REQ;
    ilm_ptr->local_para_ptr = (local_para_struct *) NULL;
    ilm_ptr->peer_buff_ptr  = (peer_buff_struct *) NULL;
    SEND_ILM(MOD_MMI, MOD_L4C, MMI_L4C_SAP, ilm_ptr);
}
void bt_ublox_agps_get_host_ip(void)
{
	kal_uint8 host[6]={0};
	YeeLink_GetHostByName("agps.u-blox.com",host);
	bt_get_imsi_req();
	
	//bt_print("boot up");
}
void YeeLink_Post_Info(float Data,int v)
{
	YeeLinkStruct *p = &gYeeLinkSoc;
	kal_uint8 host[6]={0};

	p->f_data=Data;
	if(p->addr2.addr[0]==0)
	YeeLink_GetHostByName("api.yeelink.net",host);
	else
	{
	}

}

void YeeLink_init_all(void)
{
	YeeLinkStruct *p = &gYeeLinkSoc;

	p->Soc_hand = -1;
}
#endif
