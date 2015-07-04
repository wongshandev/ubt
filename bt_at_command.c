#if defined(__BLUE_TRCK__)
#include "bt_at_command.h"
#ifndef WIN32
#include "Gpio_sw.h"
#include "Uart_internal.h"
#endif
#include "SsSrvGprot.h"
#include "ata_external.h"
#include "IdleGprot.h"
#include "nbr_public_struct.h"
#include "mmi_rp_srv_gpio_def.h"
#include "nvram_user_defs.h"

extern bt_struct gBtps;
extern char buff_gps[1024];
BOOL bSetCMD_1 = FALSE;
BOOL bSetCMD_T = FALSE;
#define RINGBUFFMAX     360
BOOL isGpsConfig = FALSE;   
typedef struct _cmdSendStruct{

	BOOL isSendStep_1;
	BOOL isSendStep_2;
	char string_cmd[50];
}cmdSendStruct;

typedef struct _sendCmdStruct{

	cmdSendStruct send_cmd1;
	cmdSendStruct send_cmd2;
	char ringbuff[RINGBUFFMAX];
	char cmd_back[50];
}sendCmdStruct;

sendCmdStruct sendGpsCmd={0};
char gGPSPSMCMDbuff[20] = {0};
void atcmd_write_nvram(void);
void atcmd_read_nvram(void);
void atcmd_send_call_cusd_resault(char *_data);
void mmi_Serving_Cell_Info_start_req(S32 module_ID);
extern void srv_backlight_to_sleep_mode(void);
extern int nmea_scanf(const char *buff, int buff_sz, const char *format, ...);
extern void srv_shutdown_normal_start(MMI_ID trigger_man_app_id);
static void mmi_at_shutdown_timer_hdlr();
void atcmd_set_adc2arry_loop(void);
BOOL atcmd_put_data_string(char * string);
kal_uint8* atcmd_get_cmdT_data(kal_int32 secs,BOOL isSetData);
extern YeeLinkStruct gYeeLinkSoc;
extern void YeeLinkDeviceBack(YEELINK_SOC_CONECT_RESULT result,void *data);
extern void YeeLink_CloseSocket(void);
BT_CMD_RSP atcmd_get_bat_voltage(custom_cmdLine *cmd_line);
kal_uint8   uart_sleep_handle=0;
kal_uint8   uart_sleep_handle2=0;
kal_uint8   bUartHandleIsInit = 0;
kal_uint8   bIsStandyBy = 0;


kal_uint8	GPS_Owner_save = 0;
#define BT_GPS_UART     uart_port2
#define BT_GPS_MOD_UART_HISR				MOD_MMI
#define BT_UART_BAUD_9600					UART_BAUD_9600
#define BT_GPS_LDO							54
#define MAX_ADC_TMP_COUNT  16
kal_uint32 gAdcArray[MAX_ADC_TMP_COUNT] = {0};



void bt_set_gps_ldo_on(void)
{
	if(gBtps.GPS_isWork == FALSE)
	{
		gBtps.GPS_isWork = TRUE;
#ifndef WIN32
		GPIO_ModeSetup(BT_GPS_LDO, 0);
		GPIO_InitIO(1, BT_GPS_LDO);
		GPIO_WriteIO(1, BT_GPS_LDO);
		UART_TurnOnPower(BT_GPS_UART, KAL_TRUE);
		U_Open(BT_GPS_UART, BT_GPS_MOD_UART_HISR);
#endif	
	}
}
void bt_set_gps_ldo_off(void)
{
#ifndef WIN32
	if(gBtps.GPS_isWork == TRUE)
	{
		gBtps.GPS_isWork = FALSE;
#ifndef WIN32
		GPIO_ModeSetup(BT_GPS_LDO, 0);
		GPIO_InitIO(1, BT_GPS_LDO);
		GPIO_WriteIO(0, BT_GPS_LDO);
#endif
		UART_TurnOnPower(BT_GPS_UART, KAL_FALSE);
		U_Close(BT_GPS_UART, BT_GPS_MOD_UART_HISR);
	}
#endif
}
extern UART_PORT TST_PORT;
extern UART_PORT TST_PORT_L1;
extern UART_PORT PS_UART_PORT;
static kal_bool bt_is_uart_used(UART_PORT port)
{ 
    {
        return KAL_FALSE;
    }
}
static void UART_ClrTxBuffer(UART_PORT port, module_type ownerid) 
{
 
DCL_HANDLE handle; 
 UART_CTRL_CLR_BUFFER_T data; 
 data.u4OwenrId = ownerid;
 
handle = DclSerialPort_Open(port,0); 
 DclSerialPort_Control(handle,SIO_CMD_CLR_TX_BUF, (DCL_CTRL_DATA_T*)&data);
 
}
static void UART_ClrRxBuffer(UART_PORT port, module_type ownerid) 
{
 
DCL_HANDLE handle; 
 UART_CTRL_CLR_BUFFER_T data; 
 data.u4OwenrId = ownerid;
 
handle = DclSerialPort_Open(port,0); 
 DclSerialPort_Control(handle,SIO_CMD_CLR_RX_BUF, (DCL_CTRL_DATA_T*)&data);
 
}
kal_uint16 UART_GetBytes(UART_PORT port, kal_uint8 *Buffaddr, kal_uint16 Length, kal_uint8 *status, module_type ownerid)
{

	DCL_HANDLE handle;
	UART_CTRL_GET_BYTES_T data;

	data.u4OwenrId = ownerid;
	data.u2Length = Length;
	data.puBuffaddr = Buffaddr;
	data.pustatus = status;
	handle = DclSerialPort_Open(port,0);
	DclSerialPort_Control(handle,SIO_CMD_GET_BYTES, (DCL_CTRL_DATA_T*)&data);
	return data.u2RetSize;

}
static kal_uint16 UART_PutBytes(UART_PORT port, kal_uint8 *Buffaddr, kal_uint16 Length,
module_type ownerid) 
{
	DCL_HANDLE handle; 
	UART_CTRL_PUT_BYTES_T data; 

	data.u4OwenrId = ownerid; 
	data.u2Length = Length; 
	data.puBuffaddr = Buffaddr;

	handle = DclSerialPort_Open(port,0); 
	DclSerialPort_Control(handle,SIO_CMD_PUT_BYTES, (DCL_CTRL_DATA_T*)&data);  
	return data.u2RetSize;
 
}

static DCL_UINT16 UART_GetBytesAvail(DCL_DEV port) 
{ 
	 DCL_HANDLE handle; 
	 UART_CTRL_RX_AVAIL_T data; 
 
 
	handle = DclSerialPort_Open(port,0); 
	DclSerialPort_Control(handle,SIO_CMD_GET_RX_AVAIL, (DCL_CTRL_DATA_T*)&data); 
	DclSerialPort_Close(handle); 
	return data.u2RetSize;
 
}

static U32 bt_gps_read_from_uart(U8 *pbyBuf, U32 wLenMax, UART_PORT hPort, module_type hOwner)
{
	U32  wLenAvail=0; //当前数据有效接收长度
	U32  wLenRead=0; //当次读取长度
	U32  wLenRet=0;//已读取长度
	U8  byStatus=0; 
#ifndef WIN32
//收取数据，超出最大包长的数据将简单丢弃，这一层需要具体的应用协议做相应处理
	while((wLenAvail = UART_GetBytesAvail(hPort)) > 0 && (wLenRet < wLenMax))
	{
		if(wLenAvail + wLenRet > wLenMax)
		{
			wLenAvail = wLenMax - wLenRet;
		}
		wLenRead = UART_GetBytes(hPort,(pbyBuf+wLenRet), wLenAvail, &byStatus, hOwner);
		wLenRet += wLenRead;
		//bt_print("%s",pbyBuf);
	}
	if(wLenRet>0)
	UART_ClrRxBuffer(hPort,hOwner);

#endif	
	return wLenRet;
}
extern char * ubloxAssitBuff;
void bt_ublox_put_data(kal_uint8 *_data)
{
	int lenth=0;
	if(_data == NULL) return;

	#ifndef WIN32
	bt_print("send uart2 for gps:%d",*_data);
	if(sizeof(_data) == UART_PutBytes(BT_GPS_UART,_data,sizeof(_data),BT_GPS_MOD_UART_HISR))
	{
		med_free_ext_mem((void **)_data);
		bt_print("gps track OK.");
	}
	else
	{
		bt_print("~~~");
	}
	#endif
}
extern kal_uint16 U_PutBytes(UART_PORT port, kal_uint8 *Buffaddr, kal_uint16 Length, module_type ownerid );

void bt_ublox_put_cmd_data(kal_uint8 *_pdata,kal_uint16 lenth)
{
	int lenth_send=0;
	if(_pdata == NULL) return;

	#ifndef WIN32
	bt_print("send uart2 for gps lenth :%d",lenth);
	if(lenth_send = UART_PutBytes(BT_GPS_UART,_pdata,lenth,BT_GPS_MOD_UART_HISR))
	{
		UART_ClrTxBuffer(BT_GPS_UART,BT_GPS_MOD_UART_HISR);
		bt_print("ublox put cmd OK lenth_send %d",lenth_send);
	}
	else
	{
		bt_print("~~~");
	}
	#endif
}
#define IS_FULL   (RINGBUFFMAX<=(wStart+lenth))
U32  wStart = 0;
U32  wEnd = 0;
void bt_gps_push_ringbuff(char * addr,int lenth)
{
	if(IS_FULL)
	{
		wStart = wEnd = 0;
		memset(sendGpsCmd.ringbuff,0,sizeof(sendGpsCmd.ringbuff));
		memcpy(sendGpsCmd.ringbuff,addr,lenth);
	}
	else
	{
		memcpy(sendGpsCmd.ringbuff+wStart,addr,lenth);
		wStart += lenth;
	}
}
char* bt_gps_pop_ringbuff_onebyone(void)
{
	int i;
	char *heard = NULL;
	char *end = NULL;
	static char content[200] = {0};
	char TMPringbuff[RINGBUFFMAX] ={0};
	int cks_a=0,cks_b=0,v;


	memset(content,0,sizeof(content));
	
	heard = strstr(sendGpsCmd.ringbuff,"$");
	if(heard != NULL)
	{
		
		end = strstr(sendGpsCmd.ringbuff,"\n");
		if(end != NULL)
		{
			int lenth_w = end-heard+1;

			if(wStart > lenth_w)
			wStart -= lenth_w;
			
			memcpy(content,heard,lenth_w);
			memcpy(TMPringbuff,sendGpsCmd.ringbuff+lenth_w,wStart);
			memset(sendGpsCmd.ringbuff,0,sizeof(sendGpsCmd.ringbuff));
			memcpy(sendGpsCmd.ringbuff,TMPringbuff,wStart);
			return content;
		}
	}
	
	for(i=0;i<wStart;i++)
	{
		
		if((sendGpsCmd.ringbuff[i]==0xb5)&&(sendGpsCmd.ringbuff[i+1]==0x62))
		{
			for(v=2;v<8;v++)
			{
				cks_a = cks_a + sendGpsCmd.ringbuff[v];
				cks_b = cks_b + cks_a;
			}
			cks_a &= 0xff;
			cks_b &= 0xff;
			if((cks_a == sendGpsCmd.ringbuff[10-2])&&(cks_b == sendGpsCmd.ringbuff[10-1]))
			{
				memcpy(content,&sendGpsCmd.ringbuff[i],10);
				memcpy(TMPringbuff,sendGpsCmd.ringbuff+10,wStart);
				memset(&sendGpsCmd.ringbuff[i],0,sizeof(sendGpsCmd.ringbuff));
				memcpy(&sendGpsCmd.ringbuff[i],TMPringbuff,wStart);
				wStart -= 10;

				return content;
			}
		}
	}
	return NULL;
	
}
void bt_parse_cmd_loop(void)
{}

void bt_send_step2_gps(char *tmpPopBuff)
{
	if(tmpPopBuff == NULL) return;

	bt_print("cmd2.isSendStep_1 %d ",sendGpsCmd.send_cmd2.isSendStep_1);
	if((sendGpsCmd.send_cmd1.isSendStep_1 == TRUE)&&(tmpPopBuff[3] == 0x01))
	{
		kal_uint8 cmd_data[]={0xB5,0x62,0x06,0x3B,0x2C,0x00,0x01,0x06,0x00,0x00,0x00,0x90,0x02,0x00,0x10,0x27,0x00,0x00,0xC0,0xD4,0x01,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x2C,0x01,0x00,0x00,0x4F,0xC1,0x03,0x00,0x86,0x02,0x00,0x00,0xFE,0x00,0x00,0x00,0x64,0x40,0x01,0x00,0x3D,0x32};
		bt_print("send cmd1 step 2 0x%02x",cmd_data[14],cmd_data[15]);
		bt_ublox_put_cmd_data(cmd_data,sizeof(cmd_data));
		sendGpsCmd.send_cmd1.isSendStep_1 = FALSE;
		sendGpsCmd.send_cmd1.isSendStep_2 = TRUE;
	}
	else if((sendGpsCmd.send_cmd1.isSendStep_2 == TRUE)&&(tmpPopBuff[3] == 0x01))
	{
		memset(&sendGpsCmd,0,sizeof(sendGpsCmd));
		bt_print("send cmd1 step 2   OK");
		atcmd_put_data_string("OK");
		StartTimer(BT_SMS_SEND_TIME_OUT_ID, 1, atcmd_write_nvram);
		memset(sendGpsCmd.cmd_back,0,strlen(sendGpsCmd.cmd_back));
		strcpy(sendGpsCmd.cmd_back,"+GPSPSM: 1");
		StartTimer(BT_SMS_SEND_TIME_OUT_ID, 1, atcmd_write_nvram);	
	}
	else if((sendGpsCmd.send_cmd2.isSendStep_1 == TRUE)&&(tmpPopBuff[3] == 0x01))
	{
		kal_uint8 cmd_data[]={0xB5,0x62,0x06,0x3B,0x2C,0x00,0x01,0x06,0x00,0x00,0x00,0x90,0x02,0x00,0x10,0x27,0x00,0x00,0xC0,0xD4,0x01,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x2C,0x01,0x00,0x00,0x4F,0xC1,0x03,0x00,0x86,0x02,0x00,0x00,0xFE,0x00,0x00,0x00,0x64,0x40,0x01,0x00,0x3D,0x32};

		bt_print("send cmd2 step 2 0x%02x",cmd_data[14],cmd_data[15]);
		memcpy(cmd_data,atcmd_get_cmdT_data(0,0),sizeof(cmd_data));
		bt_ublox_put_cmd_data(cmd_data,sizeof(cmd_data));
		sendGpsCmd.send_cmd2.isSendStep_1 = FALSE;
		sendGpsCmd.send_cmd2.isSendStep_2 = TRUE;
	}
	else if((sendGpsCmd.send_cmd2.isSendStep_2 == TRUE)&&(tmpPopBuff[3] == 0x01))
	{
		memset(&sendGpsCmd,0,sizeof(sendGpsCmd));
		bt_print("send cmd2 step 2   OK");
		atcmd_put_data_string("OK");
		memset(sendGpsCmd.cmd_back,0,strlen(sendGpsCmd.cmd_back));
		strcpy(sendGpsCmd.cmd_back,"+GPSPSM: ");
		strcat(sendGpsCmd.cmd_back,gGPSPSMCMDbuff);
		StartTimer(BT_SMS_SEND_TIME_OUT_ID, 1, atcmd_write_nvram);
	}
	else if(tmpPopBuff[3] == 0x01)
	{
		if(isGpsConfig != TRUE)
		atcmd_put_data_string("OK");
		memset(sendGpsCmd.cmd_back,0,strlen(sendGpsCmd.cmd_back));
		strcpy(sendGpsCmd.cmd_back,"+GPSPSM: 0");
		StartTimer(BT_SMS_SEND_TIME_OUT_ID, 1, atcmd_write_nvram);
	}
	else
	{
		memset(&sendGpsCmd,0,sizeof(sendGpsCmd));
		atcmd_put_data_string("ERROR");
	}

}
 void bt_gps_uart_read_ext(void* msg)
 {
 	int count;
	U32  wLenRet;
	U32  wLenCPY;
	#define MAX_READ_BUFF_LENTH  512
	char read_buff[512] = {0};
	uart_ready_to_read_ind_struct *uart_rtr_ind = (uart_ready_to_read_ind_struct*)msg;

#ifndef WIN32
	if(BT_GPS_MOD_UART_HISR != U_GetOwnerID(BT_GPS_UART))
	{
		U_SetOwner(BT_GPS_UART, BT_GPS_MOD_UART_HISR);
	}
#endif
	if(BT_GPS_UART == uart_rtr_ind->port)
	{
		
		wLenRet = bt_gps_read_from_uart(read_buff, 512, BT_GPS_UART, BT_GPS_MOD_UART_HISR);

		for(count=0;count<wLenRet;count++)
		{
			if((read_buff[count] == 0xb5)&&(read_buff[count+1] == 0x62))
			{
				#define MAX_MERGE_LENTH   MAX_READ_BUFF_LENTH
				int v = 0;
				int cks_a=0,cks_b=0;
				char merge_read_buff[MAX_MERGE_LENTH] = {0};

				//bt_print("bt_gps_uart_read_ext:0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x",
				//read_buff[count],read_buff[count+1],read_buff[count+2],read_buff[count+3],read_buff[count+4],read_buff[count+5],read_buff[count+6],read_buff[count+7],
				//read_buff[count+8],read_buff[count+9]);
				if(wLenRet >= 10)
				{
					bt_send_step2_gps(read_buff+count);
				}

			}
		}
		if(wLenRet > 0)
		{
			memset(buff_gps,0,sizeof(buff_gps));
			memcpy(buff_gps,read_buff,sizeof(read_buff));
			bt_gps_parse_loop();
		}

	}
	if(uart_port1 == uart_rtr_ind->port)
	{
		memset(read_buff,0,strlen(read_buff));
		wLenRet = bt_gps_read_from_uart(read_buff, 512, uart_port1, BT_GPS_MOD_UART_HISR);
		bt_print("uart1:%s",read_buff);
	}
 }

 void UART_SetBaudRate(UART_PORT port, UART_baudrate baudrate, module_type ownerid)
 {
 
	 DCL_HANDLE handle;
	 UART_CTRL_BAUDRATE_T data;
	 data.u4OwenrId = ownerid;
	 data.baudrate = baudrate;
	 
	 handle = DclSerialPort_Open(port,0);
	 DclSerialPort_Control(handle,SIO_CMD_SET_BAUDRATE, (DCL_CTRL_DATA_T*)&data);
 
 }

 kal_bool UART_Open(UART_PORT port, module_type ownerid)
 {
	 DCL_HANDLE handle;
	 UART_CTRL_OPEN_T data;
	 
	 DCL_STATUS status;
	 data.u4OwenrId = ownerid;
	 handle = DclSerialPort_Open(port,0);
	 status = DclSerialPort_Control(handle,SIO_CMD_OPEN, (DCL_CTRL_DATA_T*)&data);
	  if(STATUS_OK != status)
		 return KAL_FALSE;
	 
 }
 void UART_Close(UART_PORT port, module_type ownerid)
{
	DCL_HANDLE handle;
	UART_CTRL_CLOSE_T data;
	data.u4OwenrId = ownerid;
	 handle = DclSerialPort_Open(port,0);
	DclSerialPort_Control(handle,SIO_CMD_CLOSE, (DCL_CTRL_DATA_T*)&data);		 
}
 static void UART_SetOwner(UART_PORT port, module_type ownerid) 
 {
  
 DCL_HANDLE handle; 
  UART_CTRL_OWNER_T data; 
  data.u4OwenrId = ownerid;
  
 handle = DclSerialPort_Open(port,0); 
  DclSerialPort_Control(handle,SIO_CMD_SET_OWNER, (DCL_CTRL_DATA_T*)&data);
  
 }
 static module_type UART_GetOwnerID(UART_PORT port) 
 {
	DCL_HANDLE handle; 
	UART_CTRL_OWNER_T data;

	handle = DclSerialPort_Open(port,0); 
	DclSerialPort_Control(handle,SIO_CMD_GET_OWNER_ID, (DCL_CTRL_DATA_T*)&data); 
	return (module_type)(data.u4OwenrId); 
 }
 void UART_SetDCBConfig(UART_PORT port, UARTDCBStruct *UART_Config, module_type ownerid) 
 { 
	DCL_HANDLE handle; 
	UART_CTRL_DCB_T data; 

	data.u4OwenrId = ownerid; 
	data.rUARTConfig.u4Baud = UART_Config->baud; 
	data.rUARTConfig.u1DataBits = UART_Config->dataBits; 
	data.rUARTConfig.u1StopBits = UART_Config->stopBits; 
	data.rUARTConfig.u1Parity = UART_Config->parity; 
	data.rUARTConfig.u1FlowControl = UART_Config->flowControl; 
	data.rUARTConfig.ucXonChar = UART_Config->xonChar; 
	data.rUARTConfig.ucXoffChar = UART_Config->xoffChar; 
	data.rUARTConfig.fgDSRCheck = UART_Config->DSRCheck;

	handle = DclSerialPort_Open(port,0); 
	DclSerialPort_Control(handle,SIO_CMD_SET_DCB_CONFIG, (DCL_CTRL_DATA_T*)&data); 
	DclSerialPort_Close(handle); 
 }
#define GPS_TX_GPIO	21
#define GPS_RX_GPIO 20
#ifndef WIN32
extern UartDriver_strcut UartDriver;
static void UART_Bootup_Init( void )
{
    DCL_HANDLE handle;
    handle = DclSerialPort_Open( uart_port2, 0 );
    DclSerialPort_Control( handle, UART_CMD_BOOTUP_INIT, NULL );
}
static U8 lb_uart_ready_to_read_ind(void* msg)
{
	
}
void UART_Register_RX_cb(UART_PORT port, module_type ownerid, UART_RX_FUNC func)
{

	DCL_HANDLE handle;
	UART_CTRL_REG_RX_CB_T data;
	data.u4OwenrId = ownerid;
	data.func = func;

	handle = DclSerialPort_Open(port,0);
	DclSerialPort_Control(handle,SIO_CMD_REG_RX_CB, (DCL_CTRL_DATA_T*)&data);
}
void UART_Register_TX_cb(UART_PORT port, module_type ownerid, UART_TX_FUNC func)
{

	DCL_HANDLE handle;
	UART_CTRL_REG_TX_CB_T data;
	data.u4OwenrId = ownerid;
	data.func = func;

	handle = DclSerialPort_Open(port,0);
	DclSerialPort_Control(handle,SIO_CMD_REG_TX_CB, (DCL_CTRL_DATA_T*)&data);
}
extern UARTStruct	UARTPort[256];

void bt_put_bytes_test(void)
{
	kal_uint16 Length;
	
	UART_PutBytes(BT_GPS_UART,"123ABC",strlen("123ABC"),BT_GPS_MOD_UART_HISR);
	SetProtocolEventHandler(bt_gps_uart_read_ext, MSG_ID_UART_READY_TO_READ_IND);
	StartTimer(BT_SOC_CONNECT_TIMER_OUT_ID,1000*3,bt_put_bytes_test);
}
extern void UART_dafault_tx_cb(UART_PORT port);
extern void UART_dafault_rx_cb(UART_PORT port);
#endif
extern bt_struct gBtps;
kal_uint8 uart_buf_temp[512];
void bt_mtk_entry_sleep_ex(void)
{	

	bt_set_gps_ldo_off();
	srv_backlight_to_sleep_mode();
}
void atcmd_set_gps_config_ex(kal_uint8 index)
{
	//配置GPS输入语句
	kal_uint8 * pArray[8];

	kal_uint8 dis_GPGGA[] = {0xB5,0x62,0x06,0x01,0x06,0x00,0xF0,0x00,0x00,0x00,0x00,0x00,0xFD,0x15};
	kal_uint8 dis_GPGLL[] = {0xB5,0x62,0x06,0x01,0x06,0x00,0xF0,0x01,0x00,0x00,0x00,0x00,0xFE,0x1A};
	kal_uint8 dis_GPGSV[] = {0XB5,0x62,0x06,0x01,0x06,0x00,0xF0,0x03,0x00,0x00,0x00,0x00,0x00,0x24};
	kal_uint8 dis_GPVTG[] = {0xB5,0x62,0x06,0x01,0x06,0x00,0xF0,0x05,0x00,0x00,0x00,0x00,0x02,0x2E};
	kal_uint8 dis_GPGST[] = {0xB5,0x62,0x06,0x01,0x06,0x00,0xF0,0x07,0x00,0x00,0x00,0x00,0x04,0x38};
	kal_uint8 dis_GPGRS[] = {0XB5,0x62,0x06,0x01,0x06,0x00,0xF0,0x06,0x00,0x00,0x00,0x00,0x03,0x33};
	kal_uint8 dis_GPZDA[] = {0XB5,0x62,0x06,0x01,0x06,0x00,0xF0,0x08,0x00,0x00,0x00,0x00,0x05,0x3D};
	kal_uint8 dis_GPGSA[] = {0xB5,0x62,0x06,0x01,0x06,0x00,0xF0,0x02,0x00,0x00,0x00,0x00,0xFF,0x1F};
	pArray[0] = dis_GPGGA;
	pArray[1] = dis_GPGLL;
	pArray[2] = dis_GPGSV;
	pArray[3] = dis_GPGST;
	pArray[4] = dis_GPGRS;
	pArray[5] = dis_GPVTG;
	pArray[6] = dis_GPZDA;
	pArray[7] = dis_GPGSA;
#ifndef WIN32
	if(BT_GPS_MOD_UART_HISR != U_GetOwnerID(BT_GPS_UART))
	{
		U_SetOwner(BT_GPS_UART, BT_GPS_MOD_UART_HISR);
	}
#endif
	bt_ublox_put_cmd_data(pArray[index],sizeof(dis_GPGGA));
}
void atcmd_set_gps_config_loop(void)
{
	static int count = 0;

	if(bIsStandyBy == TRUE)
	return;

	
	if(count < 8)
	{
		isGpsConfig = TRUE;
		atcmd_set_gps_config_ex(count);
		count++;
	}
	else
	{
		isGpsConfig = FALSE;
		bt_print("GPS config finishd");
		count = 0;
		StopTimer(BT_AT_SET_SYS_SLEEP_TIMER_ID);
		return;
	}
	StartTimer(BT_AT_SET_SYS_SLEEP_TIMER_ID, 600, atcmd_set_gps_config_loop);
}
U16 atcmd_get_time_id(void)
{
	return BT_AT_SET_GPS_CMD_TIMER_ID;
}
void atcm_put_boot_up_cmd(void)
{
	atcmd_put_data_string("modem start");
}
extern void YeeLink_init_all(void);
void bt_gps_init(void)
{
	static BOOL isBootUp = FALSE;

	kal_bool status3; 
	module_type ownerid;
	UARTDCBStruct  UART_Config =
	{
	   UART_BAUD_9600,    /* baud; */
	   len_8,               /* dataBits; */
	   sb_1,                /*stopBits; */
	   pa_none,             /* parity; */
	   fc_none,             /*flow control*/
	   0x11,                /* xonChar; */
	   0x13,                /* xoffChar; */
	   KAL_FALSE
	};
	UARTDCBStruct  UART_dc ={0};
	UART_CTRL_GET_BYTES_T data;
	kal_uint8 status;
	kal_uint32 result_count, actual_written_len;

#ifndef WIN32
	GPIO_ModeSetup(20,1); //open uart2
	GPIO_ModeSetup(21,1);
	if(uart_sleep_handle == 0)
	uart_sleep_handle = L1SM_GetHandle();
	
	L1SM_SleepDisable(uart_sleep_handle);
	
	kal_sleep_task(kal_milli_secs_to_ticks(20)); // need delay 20ms 
	bIsStandyBy = FALSE;
	UART_HWInit(BT_GPS_UART);
	U_SetOwner(BT_GPS_UART, BT_GPS_MOD_UART_HISR);
	UART_TurnOnPower(BT_GPS_UART, KAL_TRUE);	
	U_Open(BT_GPS_UART, BT_GPS_MOD_UART_HISR);
	U_SetBaudRate(BT_GPS_UART, BT_UART_BAUD_9600, BT_GPS_MOD_UART_HISR);
	U_Register_TX_cb(BT_GPS_UART, BT_GPS_MOD_UART_HISR, UART_dafault_tx_cb);
	U_Register_RX_cb(BT_GPS_UART, BT_GPS_MOD_UART_HISR, UART_dafault_rx_cb);
	SetProtocolEventHandler(bt_gps_uart_read_ext, MSG_ID_UART_READY_TO_READ_IND);
	UART_ClrRxBuffer(BT_GPS_UART,BT_GPS_MOD_UART_HISR);
	UART_ClrTxBuffer(BT_GPS_UART,BT_GPS_MOD_UART_HISR);
	kal_sleep_task(kal_milli_secs_to_ticks(20)); // need delay 20ms 
	bt_set_gps_ldo_on();
	StartTimer(BT_AT_SET_SYS_SLEEP_TIMER_ID, 1000*1, atcmd_set_gps_config_loop);
	bt_print("*********all init********* handle %d",uart_sleep_handle);

	//uart 1 init
	/*
	UART_HWInit(uart_port1);
	U_SetOwner(uart_port1, BT_GPS_MOD_UART_HISR);
	UART_TurnOnPower(uart_port1, KAL_TRUE);	
	U_Open(uart_port1, BT_GPS_MOD_UART_HISR);
	U_SetBaudRate(uart_port1, BT_UART_BAUD_9600, BT_GPS_MOD_UART_HISR);
	U_Register_TX_cb(uart_port1, BT_GPS_MOD_UART_HISR, UART_dafault_tx_cb);
	U_Register_RX_cb(uart_port1, BT_GPS_MOD_UART_HISR, UART_dafault_rx_cb);
	UART_ClrRxBuffer(uart_port1,BT_GPS_MOD_UART_HISR);
	UART_ClrTxBuffer(uart_port1,BT_GPS_MOD_UART_HISR);
	*/
	
#else
	//

#endif
	gBtps.btSms.bSendResult=TRUE; 
	atcmd_set_adc2arry_loop();
	atcmd_read_nvram();
	if(!isBootUp)
	{
		isBootUp = TRUE;
		StartTimer(BT_GET_LOC_TIMER_ID, 1000*2, atcm_put_boot_up_cmd);
		
	}
	//	bt_parse_cmd_loop();

	YeeLink_init_all();
}

BOOL atcmd_put_data_string(char * string)
{

	static kal_uint8 len=0;

	len = strlen(string);
	if(len <= 1)
	{
		string = "FAIL";
	}
	//UART_PutBytes(uart_port1,string,strlen(string),BT_GPS_MOD_UART_HISR);
	//UART_ClrTxBuffer(uart_port1,BT_GPS_MOD_UART_HISR);
	rmmi_write_to_uart((kal_uint8*)string, strlen(string), KAL_TRUE);
	if(strlen(string) == 0) 
	{
		return FALSE;
	}
}
void lb_make_call_out(char * call_number)
{
	U8* num_ucs2 = NULL;
	mmi_ucm_make_call_para_struct make_call_para;
	
	mmi_frm_scrn_set_leave_proc(GRP_ID_UCM_PRE_MO, SCR_ID_UCM_DIAL_CALL_TYPE_MENU, (mmi_proc_func)NULL);
	
	num_ucs2 = OslMalloc((SRV_UCM_MAX_NUM_URI_LEN + 1)* ENCODING_LENGTH);
	memset(num_ucs2, 0, (SRV_UCM_MAX_NUM_URI_LEN + 1) * ENCODING_LENGTH);
	
	mmi_asc_n_to_ucs2((CHAR*)num_ucs2, (CHAR*)call_number, SRV_UCM_MAX_NUM_URI_LEN);

	/* in this api, dial type must be single, so when trigger mmi_ucm_call_launch, must dial out or permit fail */
	mmi_ucm_init_call_para(&make_call_para);
	make_call_para.dial_type = SRV_UCM_VOICE_CALL_TYPE;	
	make_call_para.ucs2_num_uri = (U16*)num_ucs2;
	make_call_para.adv_para.is_ip_dial =  KAL_FALSE;
	make_call_para.adv_para.after_make_call_callback = NULL;
	make_call_para.adv_para.after_callback_user_data = NULL;
	make_call_para.adv_para.phb_data = 0x00;
	make_call_para.adv_para.module_id = SRV_UCM_MODULE_ORIGIN_COMMON;
	mmi_ucm_call_launch(0, &make_call_para);

	OslMfree(num_ucs2);
	return;
}

extern void mmi_ucm_dial_option_make_call(void);
BT_CMD_RSP atcmd_call_out(custom_cmdLine *cmd_line)
{
	mmi_ucm_make_call_para_struct make_call_para = {0};


#define IS_IN_CALL		((GetActiveScreenId() >= SCR_ID_UCM_OUTGOING)&&\
                                        (GetActiveScreenId() <SCR_ID_UCM_DUMMY) )
	char Ucs2_strNumber[20*2] = {0};
	
	if((IS_IN_CALL)||!srv_sim_ctrl_is_available(MMI_SIM1)||(strlen(cmd_line->position+cmd_line->character+1))<4)
	{
		bt_print("no simcard,Dont call out.");
		atcmd_put_data_string("FAIL");
		return CMD_RSP_FAILD;
	}

	lb_make_call_out(cmd_line->character+cmd_line->position+1);
	atcmd_put_data_string("OK");
	return CMD_RSP_OK;
	mmi_asc_to_ucs2(Ucs2_strNumber, cmd_line->character+cmd_line->position+1);
	MakeCall((CHAR*) Ucs2_strNumber);
	atcmd_put_data_string("OK");
	return CMD_RSP_OK;
}
BT_CMD_RSP atcmd_incomingcall_sendkey(custom_cmdLine *cmd_line)
{
    if ((srv_ucm_query_call_count(SRV_UCM_INCOMING_STATE, SRV_UCM_CALL_TYPE_NO_CSD, NULL) == 1) &&
        (srv_ucm_query_call_count(SRV_UCM_CALL_STATE_ALL, SRV_UCM_CALL_TYPE_ALL, NULL) == 1) &&
        (srv_ucm_is_pending_action() == MMI_FALSE))
    {
        mmi_ucm_incoming_call_sendkey();
        atcmd_put_data_string("OK");
    }
    else
    {
        atcmd_put_data_string("FAIL");
    }
	return CMD_RSP_OK;
}
BT_CMD_RSP atcmd_incomingcall_endkey(custom_cmdLine *cmd_line)
{
#define IS_IN_CALL		((GetActiveScreenId() >= SCR_ID_UCM_OUTGOING)&&\
											(GetActiveScreenId() <SCR_ID_UCM_DUMMY) )
	if(IS_IN_CALL)
	{
		mmi_ucm_end_key();
		atcmd_put_data_string("OK");
	}
	else
	{
		atcmd_put_data_string("FAIL");
	}
	return CMD_RSP_OK;
}
void atcmd_send_msg_complate_successfull(void)
{
	if(gBtps.at_flag.sms_is_sending == TRUE)
	{
		gBtps.at_flag.sms_is_sending = FALSE;
	}
	atcmd_put_data_string("OK");
}
void atcmd_send_msg_complate_faild(void)
{
	if(gBtps.at_flag.sms_is_sending == TRUE)
	{
		gBtps.at_flag.sms_is_sending = FALSE;
	}
	atcmd_put_data_string("FAIL");
}

BT_CMD_RSP atcmd_send_msg(custom_cmdLine *cmd_line)
{
	/*AT+CMGS=138888888,HELLOWORLD */
	char AscNumber[20] = {0};
	char AscContent[100] = {0};
	
	nmea_scanf((const char * )cmd_line->character, cmd_line->length, "AT+CMGS=%s,%s",AscNumber,AscContent);
	
	if(strlen(AscNumber)==0 || strlen(AscContent) == 0)
	goto AT_ERROR;
	//bt_print("send msg:%s--->[%s]",AscNumber,AscContent);
	if(bt_at_send_sms(AscNumber,AscContent))
	{
		gBtps.at_flag.sms_is_sending = TRUE;
	}
	//要转到回调里去，要等发送出结果，再向AT回复
	//return CMD_RSP_OK;

	AT_ERROR:
		atcmd_put_data_string("FAIL");
		return CMD_RSP_FAILD;
}
extern U16 srv_ss_get_result_string(srv_ss_result_enum result);
extern srv_ss_result_enum srv_ss_act_req(srv_ss_act_enum act_op, void *act_data, mmi_proc_func callback, void *user_data);
srv_ss_result_enum mmi_ss_op_send(mmi_sim_enum sim, U8 len, WCHAR *str, mmi_proc_func rsp_cb, void *user_data)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    srv_ss_operation_req_struct req;
    
    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
	bt_print("mmi_ss_op_send len %d",len);
    memset(&req, 0, sizeof(srv_ss_operation_req_struct));

    req.sim = sim;

    mmi_ucs2ncpy((CHAR*)req.string, (CHAR*)str, len);

    return srv_ss_act_req(SRV_SS_ACT_SS_OPERATION, &req, rsp_cb, user_data);
}
mmi_ret bt_send_cusd_callback(mmi_event_struct *param)
{
	bt_print("cusd evt_id %d",param->evt_id);

    return MMI_RET_OK;
}
BT_CMD_RSP bt_send_cusd(char *_data)
{
	WCHAR wchar_ussd[400] = {0};
	BOOL result;

	//"*126*12318655ABCabc#"
	if(_data == NULL) return;
	mmi_asc_to_ucs2((CHAR *)wchar_ussd, _data);
	bt_print(" %s",_data);
    result = mmi_ss_op_send(MMI_SIM1, (U8)mmi_ucs2strlen((CHAR*)wchar_ussd), wchar_ussd, bt_send_cusd_callback, NULL);
	bt_print("bt_send_cusd result %d",result);
    if (result != SRV_SS_RESULT_OK)
    {
		atcmd_put_data_string("Error");

		srv_ss_set_ps_event_handler(NULL, MSG_ID_MMI_SS_ABORT_RSP);
		
		mmi_frm_send_ilm((oslModuleType)mmi_frm_sim_to_l4c_mod(MMI_SIM1), MSG_ID_MMI_SS_ABORT_REQ, NULL, NULL);
		return CMD_RSP_OK;
    }
}
void srv_atcmd_uart_rsp(void *info, int mod_src)
{
	bt_print("put uart %s",(CHAR *)info);
	atcmd_put_data_string((CHAR*)info);
	//atcmd_put_data_string("OK");
}
void atcmd_send_call_cusd_resault(char *_data)
{
	if(_data == NULL) return;

	atcmd_put_data_string((CHAR*)_data);
}
extern void YeeLink_SendData_Req(char *str_host,char *content);
BT_CMD_RSP atcmd_send_call_cusd(custom_cmdLine *cmd_line)
{
#define IS_IN_CALL		((GetActiveScreenId() >= SCR_ID_UCM_OUTGOING)&&\
											(GetActiveScreenId() <SCR_ID_UCM_DUMMY) )
		char ussd_string[200] = {0};

		bt_print("cmd_line string:%s",cmd_line->character);
		if((IS_IN_CALL)||!srv_sim_ctrl_is_available(MMI_SIM1))
		{
			bt_print("USSD? in call or No simcard");
			atcmd_put_data_string("ERROR");
			return CMD_RSP_OK;
		}
		nmea_scanf(cmd_line->character, strlen(cmd_line->character), "AT+CUSD=1,\"%s\"",ussd_string);
		if(strlen(ussd_string) == 0)
		{
			atcmd_send_call_cusd_resault("OK");
			return CMD_RSP_OK;
		}
		bt_print("USSD string:%s",ussd_string);

		//wongshan
		//bt_send_cusd(ussd_string);
		//YeeLink_SendData_Req("180.97.33.108",ussd_string);

		//atcmd_put_data_string("OK");
		return CMD_RSP_OK;

}
void AT_GetGPSInfoTimerOut(void)
{
	//if(gBtps.AT_getGpsInfoFlg == TRUE)
	{
		char buff[50] ={0};
		gBtps.AT_getGpsInfoFlg = FALSE;

		bt_print("Get GPS timerout ,Send loc to AT command");
		strcpy(buff,"+GPS: , , ,");
		atcmd_send_call_cusd_resault(buff);
		atcmd_send_call_cusd_resault("OK");
	}
}
extern btGPSInfo GPSInfo;
BT_CMD_RSP atcmd_get_gps_info(custom_cmdLine *cmd_line)
{
	gBtps.AT_getGpsInfoFlg = TRUE;

	bt_print("Get GPS info AT command");
	GPS_ActiveCallBack_ex(&GPSInfo);
	return CMD_RSP_OK;
}
BT_CMD_RSP atcmd_set_sys_sleep(custom_cmdLine *cmd_line)
{
	if(!strcmp(cmd_line->character,"AT+STANDBY"))
	{
		atcmd_put_data_string("OK");
		bt_set_gps_ldo_off();

		L1SM_SleepEnable(uart_sleep_handle);

		bIsStandyBy = TRUE;
		StartTimer(BT_AT_GET_GPS_INFO_TIMER_OUT_ID,1000,bt_mtk_entry_sleep_ex);
	}
	return CMD_RSP_OK;
}
kal_uint32 atcmd_get_median_adcvaule(kal_uint32 * _array,int len)
{
	int i,j;
	kal_uint32 bTemp = 0;
	kal_uint32 aTemp[MAX_ADC_TMP_COUNT] = {0};
	kal_uint32 aTemp_row[MAX_ADC_TMP_COUNT] = {0};

	memcpy(aTemp_row,_array,sizeof(kal_uint32)*MAX_ADC_TMP_COUNT);
	
	for (j = 0; j < len - 1; j ++)
	{
		for (i = 0; i < len - j - 1; i ++)
		{
			if (aTemp_row[i] > aTemp_row[i + 1])
			{
				// 互换
				bTemp = aTemp_row[i];
				aTemp_row[i] = aTemp_row[i + 1];
				aTemp_row[i + 1] = bTemp;
			}
		}
	}
	//去掉前低三位
	memcpy(aTemp,aTemp_row+3,sizeof(kal_uint32)*(MAX_ADC_TMP_COUNT-3));
	//去掉后高三位
	memset(aTemp_row,0,sizeof(kal_uint32)*MAX_ADC_TMP_COUNT);
	memcpy(aTemp_row,aTemp,sizeof(kal_uint32)*(MAX_ADC_TMP_COUNT-3-3));

	//再求平均值
	bTemp = 0;
	for(i=0;i<MAX_ADC_TMP_COUNT;i++)
	{
		bTemp += aTemp_row[i];
	}
	bTemp = bTemp/(MAX_ADC_TMP_COUNT - 3-3);
	return bTemp;
}
void atcmd_set_adc2arry_loop(void)
{
	int i = 0;
	kal_uint32 CurADC = 0;
	static int _index = 0;
	kal_uint32 gAdcArray_compare[MAX_ADC_TMP_COUNT] = {0};
	custom_cmdLine cmd;
	
	#ifdef WIN32
	CurADC = 10+rand()%10;
	memcpy(gAdcArray_compare+1,gAdcArray,sizeof(kal_uint32)*(MAX_ADC_TMP_COUNT-1));
	gAdcArray_compare[0] = CurADC;
	#else
	bmt_get_adc_channel_voltage(DCL_VBAT_ADC_CHANNEL, &CurADC);
	memcpy(gAdcArray_compare+1,gAdcArray,sizeof(kal_uint32)*(MAX_ADC_TMP_COUNT-1));
	gAdcArray_compare[0] = CurADC/1000;
	#endif
	memcpy(gAdcArray,gAdcArray_compare,sizeof(gAdcArray));

	if(gAdcArray[2] == 0)
	{
		for(i=0;i<MAX_ADC_TMP_COUNT;i++)
		{
			gAdcArray[i] = gAdcArray_compare[0];
		}
	}
	StartTimer(BT_AT_ADD_TIMER_1,500,atcmd_set_adc2arry_loop);
}

BT_CMD_RSP atcmd_get_bat_voltage(custom_cmdLine *cmd_line)
{
	kal_uint32 voltage=0;
	int i;
	if(!strcmp(cmd_line->character,"AT+CBC"))
	{
		char buff[50] = {0};

	    if(gAdcArray[MAX_ADC_TMP_COUNT-1] != 0)
	    {
			voltage = atcmd_get_median_adcvaule(gAdcArray,MAX_ADC_TMP_COUNT);
			bt_print("~avg:%d",voltage);
	    }
		//4126744
		//4164000
	    //voltage = voltage/10000;
	    if(voltage > 4133)
	    {
		    sprintf(buff,"+CBC: 100%");
	    }
	    else if(voltage <= 3500)
	    {
		    sprintf(buff,"+CBC: 0%");
	    }
	   	else if(voltage <= 3626)
	    {
		    sprintf(buff,"+CBC: 5%");
	    }
	    else if(voltage <= 3658)
	    {
		    sprintf(buff,"+CBC: 10%");
	    }
	    else if(voltage <= 3691)
	    {
		    sprintf(buff,"+CBC: 15%");
	    }
	    else if(voltage <= 3708)
	    {
		    sprintf(buff,"+CBC: 20%");
	    }
	    else if(voltage <= 3722)
	    {
		    sprintf(buff,"+CBC: 25%");
	    }
	    else if(voltage <= 3730)
	    {
		    sprintf(buff,"+CBC: 30%");
	    }
	    else if(voltage <= 3739)
	    {
		    sprintf(buff,"+CBC: 35%");
	    }
	    else if(voltage <= 3758)
	    {
		    sprintf(buff,"+CBC: 40%");
	    }
	    else if(voltage <= 3772)
	    {
		    sprintf(buff,"+CBC: 45%");
	    }
	    else if(voltage <= 3793)
	    {
		    sprintf(buff,"+CBC: 50%");
	    }
	    else if(voltage <= 3822)
	    {
		    sprintf(buff,"+CBC: 55%");
	    }
	    else if(voltage <= 3852)
	    {
		    sprintf(buff,"+CBC: 60%");
	    }
	    else if(voltage <= 3881)
	    {
		    sprintf(buff,"+CBC: 65%");
	    }
	    else if(voltage <= 3922)
	    {
		    sprintf(buff,"+CBC: 70%");
	    }
	    else if(voltage <= 3950)
	    {
		    sprintf(buff,"+CBC: 75%");
	    }
	    else if(voltage <= 4005)
	    {
		    sprintf(buff,"+CBC: 80%");
	    }	    
	    else if(voltage <= 4047)
	    {
		    sprintf(buff,"+CBC: 85%");
	    }	    
	    else if(voltage <= 4089)
	    {
		    sprintf(buff,"+CBC: 90%");
	    }	   
	    else if(voltage <= 4133)
	    {
		    sprintf(buff,"+CBC: 95%");
	    }
	    else
	    {
	    	bt_print("cbc error,buff %s",buff);
		    sprintf(buff,"+CBC: ERROR");
		    return CMD_RSP_OK;
	    }
	    bt_print("-------buff %s",buff);
	    atcmd_put_data_string(buff);
    	atcmd_put_data_string("OK");
    }
    else
    	atcmd_put_data_string("ERROR");
    return CMD_RSP_OK;
}
kal_uint8* atcmd_get_cmdT_data(kal_int32 secs,BOOL isSetData)
{
	//TIME = 180 seconds -> 180000 ms -> 0x0002BF20 -> 20 BF 02 00, Little-Endian format)
	static kal_uint8 _data[]={0xB5,0x62,0x06,0x3B,0x2C,0x00,0x01,0x06,0x00,0x00,0x00,0x90,0x00,0x00,0xff,0xff,0xff,0xff,
	0xC0,0xD4,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x2C,0x01,0x00,0x00,0x4F,0xC1,0x03,0x00,0x86,0x02,0x00,0x00,0xFE,
	0x00,0x00,0x00,0x64,0x40,0x01,0x00,0xff,0xff};

	if(isSetData)
	{
		kal_uint32 sec_format = 0;
		kal_uint8 check_sum_A = 0,check_sum_B = 0,i;

		kal_uint8 secs_data[4] = {0};
		
		if(secs < 0) return 0;
		sec_format = secs*1000; //ms

		secs_data[0]=(kal_uint8)((sec_format>>0)&0xff);
		secs_data[1]=(kal_uint8)((sec_format>>8)&0xff);
		secs_data[2]=(kal_uint8)((sec_format>>16)&0xff);
		secs_data[3]=(kal_uint8)((sec_format>>24)&0xff);
		_data[14] = secs_data[0];
		_data[15] = secs_data[1];
		_data[16] = secs_data[2];
		_data[17] = secs_data[3];

		bt_print("get cmd sec:[sec:0x%x]0x%02x,0x%02x,0x%02x,0x%02x",_data[14],_data[15],_data[16],_data[17]);
		//check sum
		for(i=2;i<(sizeof(_data)-2);i++)
		{
			check_sum_A += _data[i];
			check_sum_B += check_sum_A;
		}
		check_sum_A &= 0xff;
		check_sum_B &= 0xff;
		_data[50] = check_sum_A;
		_data[51] = check_sum_B;
		bt_print("get cmd checksumA:%d,B:%d",check_sum_A,check_sum_B);
	}
	return _data;
}
void atcmd_write_nvram(void)
{
	S16 error;
	S32 return_data;
#ifndef WIN32
//	return_data = WriteRecord(NVRAM_EF_WMP_A_LID, 1, sendGpsCmd.cmd_back, NVRAM_WMP_A_SIZE,&error);
#endif
}
void atcmd_read_nvram(void)
{
	S16 error;
	S32 return_data;
	#ifndef WIN32
	//ReadRecord(NVRAM_EF_WMP_A_LID, 1, sendGpsCmd.cmd_back, NVRAM_WMP_A_SIZE,&error);
	#endif
}
BT_CMD_RSP atcmd_set_gps_cmd(custom_cmdLine *cmd_line)
{
	kal_int32 _time=0;
	if(!strcmp(cmd_line->character,"AT+GPSPSM=0"))
	{
		kal_uint8 DataArry[] = {0xB5,0x62,0x06,0x11,0x02,0x00,0x08,0x00,0x21,0x91};
		bt_ublox_put_cmd_data(DataArry,sizeof(DataArry));
	}
	else if(!strcmp(cmd_line->character,"AT+GPSPSM=1"))
	{
		kal_uint8 DataArry[] = {0xB5,0x62,0x06,0x11,0x02,0x00,0x08,0x01,0x22,0x92};

		sendGpsCmd.send_cmd1.isSendStep_1= TRUE;
		bt_ublox_put_cmd_data(DataArry,sizeof(DataArry));
	}
	else if(nmea_scanf(cmd_line->character,strlen(cmd_line->character),"AT+GPSPSM=2,%d",&_time))
	{
		kal_uint8 DataArry[] = {0xB5,0x62,0x06,0x11,0x02,0x00,0x08,0x01,0x22,0x92};

		memset(sendGpsCmd.send_cmd2.string_cmd,0,strlen(sendGpsCmd.send_cmd2.string_cmd));
		sprintf(gGPSPSMCMDbuff,"%d,%d",2,_time);

		bt_print("AT+GPSPSM=2   %s",gGPSPSMCMDbuff);
		sendGpsCmd.send_cmd2.isSendStep_1= TRUE;
		if((_time > 0)&&(_time < 65535))
		atcmd_get_cmdT_data(_time,TRUE);
		else
		{
			atcmd_put_data_string("ERROR");
			return CMD_RSP_OK;
		}
		
		bt_ublox_put_cmd_data(DataArry,sizeof(DataArry));
	}
	else if(!strcmp(cmd_line->character,"AT+GPSPSM?"))
	{
		char string_tmp[20] = {0};

		if(strlen(sendGpsCmd.cmd_back) == 0)
		{
			strcpy(sendGpsCmd.cmd_back,"+GPSPSM: 0");
		}
		strcat(string_tmp,sendGpsCmd.cmd_back);
		atcmd_put_data_string(string_tmp);
	}
	else
	{
		atcmd_put_data_string("ERROR");
	}

		return CMD_RSP_OK;
}
/*
	获取socket 状态。
*/
BT_CMD_RSP atcmd_soc_status(custom_cmdLine *cmd_line)
{
	char Temp[30]={0};

	if(!strcmp(cmd_line->character,"AT+CGATT=1"))
	{
		//打开socket，无意义??
		atcmd_put_data_string("OK");
	}
	else if(!strcmp(cmd_line->character,"AT+CGATT=0"))
	{
		//关闭socket?
		atcmd_put_data_string("OK");
		YeeLink_CloseSocket();
	}
	else if(!strcmp(cmd_line->character,"AT+CGATT?"))
	{
		if(YeeLink_GetSocketStatus() == TRUE)
		{
			atcmd_put_data_string("+CGATT: 1");
		}
		else
		{
			atcmd_put_data_string("+CGATT: 0");
		}
	}
	else
	{
		atcmd_put_data_string("ERROR");
	}

		return CMD_RSP_OK;
}
/*
	设置ip地址及apn
	eg:AT+CGDCONT=1,192.168.1.100:8080
*/
BT_CMD_RSP atcmd_CGDCONT(custom_cmdLine *cmd_line)
{
	int index = 0;
	char _addrstring[20]={0};
	char _apn[20]={0};
	sockaddr_struct _addr = {0};

	
	if(strlen(cmd_line->character) > (20+20+15)) return CMD_RSP_FAILD;
	
	if(nmea_scanf(cmd_line->character,strlen(cmd_line->character),"AT+CGDCONT=%d,\"%s\",\"%s\"",&index,_addrstring,_apn))
	{
		nmea_scanf(_addrstring,strlen(_addrstring),"%d.%d.%d.%d:%d",&_addr.addr[0],&_addr.addr[1],&_addr.addr[2],&_addr.addr[3],&_addr.port);
		_addr.addr_len=4;
		_addr.sock_type=SOC_SOCK_STREAM;
		if(YeeLink_SetAddr(_addr)==TRUE)
		{
			atcmd_put_data_string("OK");
		}
		else
		{
			atcmd_put_data_string("ERROR");
		}
		return CMD_RSP_OK;
	}
	atcmd_put_data_string("ERROR");
	return CMD_RSP_OK;
}
void bt_atcmd_cgact_timer_out(void)
{
	if(YeeLink_GetSocketStatus() == TRUE)
	{
		atcmd_put_data_string("OK");
	}
	else
	{
		atcmd_put_data_string("ERROR");
	}
}
void bt_atcmd_riopen_timer_out(void)
{
	if(YeeLink_GetSocketStatus() == TRUE)
	{
		//atcmd_put_data_string("OK");
	}
	else
	{
		YeeLink_CloseSocket();
		atcmd_put_data_string("CONNECT FAIL");
	}
}
//RIOPEN
//AT+RIOPEN=[<index>,]<mode>,<IP address>/<domainname>,<port>
//AT+RIOPEN=[1,]TCP,192.168.1.10/www.baidu.com,8080
BT_CMD_RSP atcmd_riopen(custom_cmdLine *cmd_line)
{
	int soc_index,a2;
	char mode[20]={0};
	sockaddr_struct addr={0};
	int port;
	char domain[100]={0};
	YeeLinkStruct *p = &gYeeLinkSoc;

	if(nmea_scanf(cmd_line->character,strlen(cmd_line->character),"AT+RIOPEN=[%d,]%s,%d.%d.%d.%d/%s,%d",&soc_index,mode,&addr.addr[0],&addr.addr[1],&addr.addr[2],&addr.addr[3],domain,&port))
	{
		if(!strcmp("UDP",mode))
		{
			addr.sock_type=SOC_SOCK_DGRAM;
		}
		else
		addr.sock_type=SOC_SOCK_STREAM;

		addr.port = port;
		addr.addr_len=4;
		if(YeeLink_SetAddr(addr)==TRUE)
		{
			atcmd_put_data_string("OK");

			if(YeeLink_GetSocketStatus() == TRUE)
			{
				atcmd_put_data_string("ALREAY CONNECT");
			}
			else
			{
				YeeLink_Conect_Soc(p->addr2,NULL,YeeLinkDeviceBack);
				StartTimer(BT_AT_SEND_TIMER_OUT,1000*45,bt_atcmd_riopen_timer_out);
			}
		}
		else
		{
			atcmd_put_data_string("ERROR");
		}
	}
}

//AT+RISEND=1,2,512,f16asd4f5asd544
char *risendBuff = NULL;
void bt_atcmd_risend_timer_out(void)
{
	if(risendBuff == NULL)
	{
		//atcmd_put_data_string("SEND OK");
	}
	else
	{
		atcmd_put_data_string("SEND FAIL");
	}
}
//\r\n -->5c72
U32 atcmd_5c72_to_string(PU8 pInBuffer,U32 lenth)
{
	U8 *Pointer = NULL;
	U32 lenth_ALL = lenth;
	if(pInBuffer == NULL) return FALSE;

	Pointer = pInBuffer;
	do{
		if((*Pointer == 0x5C)&&(*(Pointer+1)==0x72))
		{
			lenth_ALL--;
			*Pointer = 0x0D;
			memmove(Pointer+1,Pointer+2,(lenth_ALL-(Pointer-pInBuffer)));
			*(pInBuffer+lenth_ALL)=0;//最后一位清零。
		}
		if((*Pointer == 0x5C)&&(*(Pointer+1)==0x6E))
		{
			lenth_ALL--;
			*Pointer = 0x0A;
			memmove(Pointer+1,Pointer+2,(lenth_ALL-(Pointer-pInBuffer)));
			*(pInBuffer+lenth_ALL)=0;//最后一位清零。
		}
	}while(!((++Pointer-pInBuffer)==lenth_ALL));

	return lenth_ALL;
}
// 5c72-->\r\n
/*

	pInBuffer:
*/
U32 atcmd_CRLF_to_hex(PU8 pInBuffer,U32 lenth)
{
	U8 *Pointer = NULL;
	U32 lenth_ALL = lenth;
	U32 lenth_end =0;
	if(pInBuffer == NULL) return FALSE;

	Pointer = pInBuffer;
	do{
		if(*Pointer == 0x0D)
		{
			lenth_ALL++;
			*(pInBuffer+lenth_ALL)=0;//最后一位清零。
			*Pointer = 0x5C;
			memmove(Pointer+2,Pointer+1,(lenth_ALL-(Pointer-pInBuffer)+1));
			*(Pointer+1) = 0x72;
		}
		if(*Pointer == 0x0A)
		{
			lenth_ALL++;
			*(pInBuffer+lenth_ALL)=0;//最后一位清零。
			*Pointer = 0x5C;
			memmove(Pointer+2,Pointer+1,(lenth_ALL-(Pointer-pInBuffer)+1));
			*(Pointer+1) = 0x6E;
		}
	}while(!((++Pointer-pInBuffer)==lenth_ALL));

	return lenth_ALL;
}
void atcmd_set_rev_packet_heard(U8 *ptr,U32 lenth)
{
	memset(ptr,0,lenth);
	sprintf(ptr,"+RIRD: 0,1,1,%d",lenth);
}
BT_CMD_RSP atcmd_riclose(custom_cmdLine *cmd_line)
{
	if(!strcmp("AT+RICLOSE=0",cmd_line))
	{
		bt_print("atcmd close socket!!!");
		YeeLink_CloseSocket();
		atcmd_put_data_string("OK");
	}
}
//AT+RISEND=<index>,<count>,<length>,"<content>
//AT+RISEND=1,1,20,ABCDEFG1234567890123
BT_CMD_RSP atcmd_risend(custom_cmdLine *cmd_line)
{
	int soc_index,CountPacket,lenth;
	char mode[20]={0};
	sockaddr_struct addr={0};
	int port;
	char domain[100]={0};
	char *ptr = NULL;
	YeeLinkStruct *p = &gYeeLinkSoc;
	U32 newLenth;

	if(nmea_scanf(cmd_line->character,strlen(cmd_line->character),"AT+RISEND=%d,%d,%d,",&soc_index,&CountPacket,&lenth))
	{

		ptr = strstr(cmd_line->character,",")+1;
		if(ptr != NULL)
		{
			ptr = strstr(ptr,",")+1;
			if(ptr != NULL)
			{
				ptr = strstr(ptr,",")+1;
				if(risendBuff == NULL)
				risendBuff = (U8 *)med_alloc_ext_mem(MAX_SOC_BUFFER_SIZE*5);

				memcpy(risendBuff,ptr,lenth);

				newLenth = atcmd_5c72_to_string(risendBuff,lenth);
				if(CountPacket == 1)
				{
					//send
					if(YeeLink_GetSocketStatus() == TRUE)
					{
						YeeLink_Conect_Soc(p->addr2,risendBuff,YeeLinkDeviceBack);
						StartTimer(BT_AT_SEND_TIMER_OUT,1000*45,bt_atcmd_risend_timer_out);
					}
					else
					{
						atcmd_put_data_string("ERROR");
					}
				}
			}
		}
	}
}
BT_CMD_RSP atcmd_cgact(custom_cmdLine *cmd_line)
{
	int a1,a2;
	YeeLinkStruct *p = &gYeeLinkSoc;

	
	if(nmea_scanf(cmd_line->character,strlen(cmd_line->character),"AT+CGACT=%d,%d,%d",&a1,&a2))
	{
		//a1? a2? 去链接了。
		if(YeeLink_GetSocketStatus() == TRUE)
		{
			atcmd_put_data_string("OK");
		}
		else
		{
			YeeLink_Conect_Soc(p->addr2,NULL,YeeLinkDeviceBack);
			StartTimer(BT_AT_ADD_TIMER_2,1000*45,bt_atcmd_cgact_timer_out);
		}
	}
}
BT_CMD_RSP atcmd_get_gps_openOrClose(custom_cmdLine *cmd_line)
{
	char Temp[30]={0};

	if(!strcmp(cmd_line->character,"AT+GPSPWD?"))
	{
		if(gBtps.GPS_isWork)
		{
			sprintf(Temp,"+GPSPWD: 1");
		}
		else
		{
			sprintf(Temp,"+GPSPWD: 0");
		}
		atcmd_put_data_string(Temp);
		atcmd_put_data_string("OK");
	}
	else if(!strcmp(cmd_line->character,"AT+GPSPWD=0"))
	{
		bt_set_gps_ldo_off();
		atcmd_put_data_string("OK");
	}
	else if(!strcmp(cmd_line->character,"AT+GPSPWD=1"))
	{
		bt_set_gps_ldo_on();
		atcmd_put_data_string("OK");
	}
	else
	{
		atcmd_put_data_string("ERROR");
	}

		return CMD_RSP_OK;
}
BT_CMD_RSP atcmd_set_gps(custom_cmdLine *cmd_line)
{
	return CMD_RSP_OK;
}

void AT_SendLocGPS_ex(btGPSInfo *GPS_info)
{
	if(GPS_info->sig > 0)
	{
		char PackGps[100] = {0};

		sprintf(PackGps,"+GPS: %f,%f,%f,%04d%02d%02d%02d%02d%02d,%d",GPS_info->lat,GPS_info->lon,GPS_info->speed,
																  GPS_info->utc_time.nYear,GPS_info->utc_time.nMonth,GPS_info->utc_time.nDay,
																  GPS_info->utc_time.nHour,GPS_info->utc_time.nMin,GPS_info->utc_time.nSec,GPS_info->sig);
		atcmd_put_data_string(PackGps);
		atcmd_send_call_cusd_resault("OK");
	}
	else
	{
		char buff[50] ={0};
		char fix[2] = {0};
		
		bt_print("GPS sig faild! %d (0 = Invalid; 1 = Fix; 2 = Differential, 3 = Sensitive)",GPS_info->sig);
		strcpy(buff,"+GPS: , , ,");
		sprintf(fix,"%d",GPS_info->sig);
		strcat(buff," ,");
		strcat(buff,fix);
		atcmd_send_call_cusd_resault(buff);
		atcmd_send_call_cusd_resault("OK");
	}
}
global_cell_id_struct info_cell[6]={0};
void mmi_Cell_Info_parsing_data(l4c_nbr_cell_info_ind_struct *msg_ptr)
{
	gas_nbr_cell_info_struct cell_info;
	int i,number;
	char uart_buff[1024] = {0};
	if(msg_ptr)
	{
		if (KAL_TRUE == msg_ptr->is_nbr_info_valid)
		{
			memcpy((void *)&cell_info, (void *)(&(msg_ptr->ps_nbr_cell_info_union.gas_nbr_cell_info)), sizeof(gas_nbr_cell_info_struct));
		}
		else
		{
			memset((void *)&cell_info, 0, sizeof(gas_nbr_cell_info_struct));    
		}
		info_cell[0].mcc = cell_info.serv_info.gci.mcc;
		info_cell[0].mnc = cell_info.serv_info.gci.mnc;
		info_cell[0].lac = cell_info.serv_info.gci.lac;
		info_cell[0].ci = cell_info.serv_info.gci.ci;
		number = cell_info.nbr_cell_num;
		bt_print("info_cell[0].ci=%d s_vm_sal_cell_nbr_num = %d",cell_info.serv_info.gci.ci,number);
		sprintf(uart_buff,"+ENBR: %d,%02d,%d,%d,%02d,%d,%02d,%02d",info_cell[0].mcc,info_cell[0].mnc,info_cell[0].lac,info_cell[0].ci,
		cell_info.nbr_meas_rslt.nbr_cells[cell_info.serv_info.nbr_meas_rslt_index].bsic,
		cell_info.nbr_meas_rslt.nbr_cells[cell_info.serv_info.nbr_meas_rslt_index].arfcn,
		cell_info.nbr_meas_rslt.nbr_cells[cell_info.serv_info.nbr_meas_rslt_index].rxlev,
		cell_info.ta);
		for(i = 0; i < cell_info.nbr_cell_num; i++)
		{
			char temp_buff[100] = {0};
			int v = cell_info.nbr_cell_info[i].nbr_meas_rslt_index;
			
			info_cell[i].mcc = cell_info.nbr_cell_info[i].gci.mcc;
			info_cell[i].mnc = cell_info.nbr_cell_info[i].gci.mnc;
			info_cell[i].lac = cell_info.nbr_cell_info[i].gci.lac;
			info_cell[i].ci = cell_info.nbr_cell_info[i].gci.ci;
			sprintf(temp_buff,"+ENBR: %d,%02d,%d,%d,%02d,%d,%02d,%02d",info_cell[i].mcc,info_cell[i].mnc,info_cell[i].lac,info_cell[i].ci,
																			cell_info.nbr_meas_rslt.nbr_cells[v].bsic,cell_info.nbr_meas_rslt.nbr_cells[v].arfcn,
																			cell_info.nbr_meas_rslt.nbr_cells[v].rxlev,cell_info.ta);
			strcat(uart_buff,temp_buff);
		}
		bt_print("%s",uart_buff);
		ClearProtocolEventHandler(MSG_ID_L4C_NBR_CELL_INFO_IND); 
		ClearProtocolEventHandler(MSG_ID_L4C_NBR_CELL_INFO_REG_CNF); 
		if(FALSE==atcmd_put_data_string(uart_buff))
		{
			atcmd_put_data_string("ERROR");
		}
		atcmd_put_data_string("OK");
	}
	else
	atcmd_put_data_string("ERROR");
}
BT_CMD_RSP mmi_cell_info_start_req(custom_cmdLine *cmd_line)
{
	mmi_frm_set_protocol_event_handler(MSG_ID_L4C_NBR_CELL_INFO_REG_CNF,mmi_Cell_Info_parsing_data,MMI_FALSE);
	mmi_frm_set_protocol_event_handler(MSG_ID_L4C_NBR_CELL_INFO_IND,mmi_Cell_Info_parsing_data,MMI_FALSE);
	mmi_frm_send_ilm(MOD_L4C,MSG_ID_L4C_NBR_CELL_INFO_REG_REQ, NULL, NULL);
	return CMD_RSP_OK;
}
static void mmi_at_shutdown_timer_hdlr()
{
    srv_shutdown_normal_start(0);
}
BT_CMD_RSP at_pwd_start_req(custom_cmdLine *cmd_line)
{
	atcmd_put_data_string("OK");
	bt_set_gps_ldo_off();
	StartTimer(BT_AT_GET_GPS_INFO_TIMER_OUT_ID,1500,mmi_at_shutdown_timer_hdlr);
	return CMD_RSP_OK;
}
BT_CMD_RSP atcmd_get_loc_info(custom_cmdLine *cmd_line)
{
	return CMD_RSP_OK;
}
BT_CMD_RSP atcmd_get_imei(custom_cmdLine *cmd_line)
{}
BT_CMD_RSP atcmd_get_imsi(custom_cmdLine *cmd_line)
{
	if(FALSE==atcmd_put_data_string(gBtps.btIMSI))
	{
		//bt_print("AT命令发送失败 %s",gBtps.btIMSI);
		return CMD_RSP_FAILD;
	}
	return CMD_RSP_OK;
}
Serving_Cell_Info_struct g_mmi_Neighbor_Cell_info[4][6];
void mmi_Neighbor_Cell_Info_parsing_data(void *info, int mod_src)  
{}
void mmi_Serving_Cell_Info_start_req(S32 module_ID)  
{}
  
//at command
const bt_custom_atcmd bt_custom_cmd_table[ ] =
{    
    //{"ATD",atcmd_call_out},
	//{"ATA",atcmd_incomingcall_sendkey},
	//{"ATH",atcmd_incomingcall_endkey},
	/*AT+CMGS=138888888,HELLOWORLD; */
	//{"AT+CMGS",atcmd_send_msg},
	/*AT+CUSD=*138888888*#; */
	{"AT+CUSD",atcmd_send_call_cusd},
	{"AT+GPS",atcmd_get_gps_info},
	{"AT+ENBR",mmi_cell_info_start_req},
	{"AT+GSMPWD",at_pwd_start_req},
	{"AT+GPSPWD",atcmd_get_gps_openOrClose},
	{"AT+CBC",atcmd_get_bat_voltage},
	{"AT+STANDBY",atcmd_set_sys_sleep},
	{"AT+GPSPSM",atcmd_set_gps_cmd},


	{"AT+CGATT",atcmd_soc_status},

	{"AT+CGDCONT",atcmd_CGDCONT},
	{"AT+CGACT",atcmd_cgact},

	{"AT+RIOPEN",atcmd_riopen},

	{"AT+RISEND",atcmd_risend},
    	{"AT+RICLOSE",atcmd_riclose}
	//{"AT+CGSN",atcmd_get_imei},
	//{"AT+CIMI",atcmd_get_imsi},
    {NULL, NULL}
};
extern void mmi_idle_get_service_indication_string(
    mmi_sim_enum sim,
    mmi_idle_service_indication_struct *service_indication);
kal_bool bt_custom_atcmd_from_uart(char *cmd_string)
{
		char buffer[MAX_UART_LEN+1]={0}; 
		char *cmd_name, *cmdString;
		kal_uint8 index = 0; 
		kal_uint16 length;
		kal_uint16 i;
		custom_cmdLine command_line={0};
		mmi_idle_service_indication_struct networkname;
		char network_name[20] = {0};
	
		cmd_name = buffer;
		mmi_idle_get_service_indication_string(MMI_SIM1,&networkname);

		mmi_wcs_to_asc(network_name, networkname.line1);
		bt_print("at_command[%s]  network name:%s",cmd_string,network_name);
		length = strlen(cmd_string);
		length = length > MAX_UART_LEN ? MAX_UART_LEN : length;    
		while ((cmd_string[index] != '=' ) &&	
			(cmd_string[index] != '?' ) && 
			(cmd_string[index] != 13 ) && 
			index < length)  
		{
			cmd_name[index] = cmd_string[index] ;
			index ++;
		}
		cmd_name[index] = 0 ;	

		
		for (i = 0 ; bt_custom_cmd_table[i].commandString != NULL; i++ )
		{
			cmdString = bt_custom_cmd_table[i].commandString;
			if (strcmp(cmd_name, cmdString) == 0 )
			{
				strncpy(command_line.character, cmd_string, COMMAND_LINE_SIZE + NULL_TERMINATOR_LENGTH);
				command_line.character[COMMAND_LINE_SIZE] = '\0';
				command_line.length = strlen(command_line.character);
				//去掉AT命令自带的\r　符号
				command_line.character[command_line.length-1] = '\0';
				command_line.position = index;
				if (bt_custom_cmd_table[i].commandFunc(&command_line) == CMD_RSP_OK) 
				{
					//全部放到各自处理函数里面。
					//sprintf(buffer, "OK");
					//rmmi_write_to_uart((kal_uint8*)buffer, strlen(buffer), KAL_TRUE);
					return KAL_TRUE;
				}
			}
		}
}

#endif

