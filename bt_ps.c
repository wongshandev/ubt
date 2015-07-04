#if defined(__BLUE_TRCK__)
#include "bt_ps.h"
#include "bt_main.h"

void bt_print(char* fmt,...);

bt_struct gBtps = {0};


void smsSendTimerOut(void)
{
	if(gBtps.btSms.bSendResult == FALSE)
	{
		gBtps.btSms.bSendResult = TRUE;	
		atcmd_send_msg_complate_faild();
	}
}
void btSmsSendCallBack(srv_sms_callback_struct *cb)
{	
	static int send_count = 0;

	if(gBtps.btSms.bSendResult == FALSE)
	{
		if(cb->result == MMI_TRUE)
		{
			gBtps.btSms.bSendResult = TRUE;
			bt_print("send msg result OK");
			atcmd_send_msg_complate_successfull();
		}
		else 
		{
			bt_print("send msg result faild:%d",gBtps.signal);
			atcmd_send_msg_complate_faild();
		}
	}
	srv_sms_delete_msg_list(
	SRV_SMS_BOX_INBOX|SRV_SMS_BOX_OUTBOX|SRV_SMS_BOX_DRAFTS|SRV_SMS_BOX_UNSENT|SRV_SMS_BOX_SIM,
	SRV_SMS_SIM_1,
	NULL,
	NULL);
}
//
BOOL bt_at_send_sms(char * AscNumber,char * AscContent)
{
	char TemNumber[20] = {0};
	char TemContent[512] = {0};
	srv_sms_callback_struct cb;
	
	if((AscNumber == NULL)||(AscContent == NULL)||(strlen(AscNumber) < 3))
	return FALSE;

	if(gBtps.btSms.bSendResult == TRUE)
	{
		gBtps.btSms.bSendResult = FALSE;
	}
	else
	{
		cb.result = FALSE;
		//bt_print("上一次没有发送成功，要等超时(40 S)后再发送本次不执行发送短信操作。");
		btSmsSendCallBack(&cb);
		return FALSE;
	}
	bt_print("send msg:%s,content:%s signal:%d",AscNumber,AscContent,gBtps.signal);
	mmi_chset_convert(CHSET_ASCII, CHSET_UCS2,(char *)AscContent,(char *)TemContent,sizeof(AscContent));
	mmi_asc_to_ucs2((char *)TemNumber, (char *)AscNumber);
       //30秒后超时
	StartTimer(BT_SMS_SEND_TIME_OUT_ID,1000*35,smsSendTimerOut);
	srv_sms_send_ucs2_text_msg((S8*)TemContent,(S8*)TemNumber,BT_SIM_CARD_USE,btSmsSendCallBack,NULL);
	return TRUE;
}
/* 大写转换小写*/
char * str_big_to_low(char * _data)
{
	size_t _count = 0;
	
	if(_data == NULL) return NULL;
	for(_count=0;_count<strlen(_data);_count++)
	{
		if((*(_data+_count) >= 'A')&&((*(_data+_count))<= 'Z'))
		{
			*(_data+_count) += 32;
		}
	}
	return _data;
}
void btDealMsgCmd(char * _phone, char * _content)
{
	#define IS_CMD(cmd) (NULL != strstr(_content,cmd))
	
	int result = 0;

	if((strlen(_phone) > 20)||(strlen(_phone) < 3)||strlen(_content)>200)
	{
		bt_print("invild sms");
		return;
	}

}

/*New messege incoming*/
void bt_new_msg_ind(char * rev_num,char * rev_content)
{
	char rev_sms[200]={0};
	char * low_msg_content = NULL;
	char * msg_num      = rev_num;
	char * addr = NULL;
	//+CMT:"+8613798335220""ABCDEFG" 
	bt_print("incoming sms[%s :%s]",rev_num,rev_content);

	sprintf(rev_sms,"+CMT:%s,%s",rev_num,rev_content);
	atcmd_put_data_string(rev_sms);
	if((rev_num == NULL)||(rev_content == NULL))
	{
		bt_print("Msg is Vaild! return.");
		return;
	}
	low_msg_content = str_big_to_low(rev_content);
	btDealMsgCmd(rev_num,low_msg_content);
}



void bt_set_signal_change(U8 percentage)
{
	//bt_print("信号量:%d",percentage);
	gBtps.signal = percentage;
}
void bt_incoming_call_pick_up(void)
{
	mmi_ucm_incoming_call_sendkey();
}
BOOL BtIncomingNewCall(char * number)
{
	char in_number[40] = {0};
	if(number == NULL) return FALSE;


	return TRUE;
	
	bt_print("incoming call:%s",number);\
	sprintf(in_number,"RING=%s",number);
	atcmd_put_data_string(in_number);
	StartTimer(BT_SEND_CALL_UP_TIMER_ID, 1000*3, bt_incoming_call_pick_up);
	return TRUE;
}


#define BT_WRITE_FS
#if defined(BT_WRITE_FS)
#include "DateTimeType.h"

#define FILE_BUF_SIZE	5*1024
static S8 log_is_enable = 0;
static U32 max_file_size = 50;
static kal_uint8 file_buffer[FILE_BUF_SIZE]={0};
void bt_print(char* fmt,...);
void WriteLogFile(kal_uint8* buf)
{
	UINT filesize=0;
	UINT LENG=0;
	UINT str_len=0;
	UINT str_len2=0;
	FS_HANDLE file_handle;//文件句柄
	UI_character_type path[20];

	if(buf == NULL) return;
	str_len2 = strlen(buf);
	str_len = strlen(file_buffer);
	if(str_len + str_len2 < FILE_BUF_SIZE)
	{
		strcat(file_buffer, buf);
		str_len += str_len2;
	}
	kal_wsprintf((kal_wchar*)path, "c:\\bt_log.txt");
	file_handle = FS_Open(path,FS_CREATE|FS_READ_WRITE);
	if (file_handle >= FS_NO_ERROR)
	{
		if(FS_GetFileSize(file_handle, &filesize) == FS_NO_ERROR)
		{
			if(filesize > max_file_size*1024)
			{
				FS_Close(file_handle);
				FS_Delete(path);
				WriteLogFile(buf);
				return;
			}
		}
		FS_Seek(file_handle, 0, FS_FILE_END);
		if(FS_Write(file_handle, file_buffer, str_len, &LENG) >= FS_NO_ERROR)
		{
			int sub_len = 0;
			if(LENG < str_len)
			{
				sub_len = str_len - LENG;
				memcpy(file_buffer, file_buffer+LENG, sub_len);
			}
			memset(file_buffer+(sub_len), 0, LENG);
		}
		FS_Close(file_handle);
	}
	
}


void bt_print(char* fmt,...)
{	
	MYTIME time;
	char buf[1024] = {0};
	char time_str[128] = {0};
	va_list MyList;
	
	va_start(MyList,fmt);
	vsprintf(buf, fmt, MyList);
	va_end(MyList);
	
#if defined(WIN32)
	printf("%s\r\n",buf);
	WriteLogFile(buf);
#else
	DTGetRTCTime(&time);
	sprintf(time_str, "\t[%d-%d:%d:%d]\n",
		time.nDay,
		time.nHour,
		time.nMin,
		time.nSec);
	strcat(buf,time_str);
	kal_prompt_trace(MOD_ENG,"%s",buf);
	WriteLogFile(buf);
#endif
}

#else
void bt_print(char * fmt,...)
{
	va_list MyList;
	char buf[512];

	va_start(MyList,fmt);
	vsprintf(buf, fmt, MyList);
#ifndef WIN32
	kal_prompt_trace(MOD_ENG,"%s",buf);
#else
	printf("%s\r\n",buf);
#endif
	va_end(MyList);
}
#endif

#endif

