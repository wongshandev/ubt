#if defined(__BLUE_TRCK__)
#include "bt_ps.h"
#include "bt_main.h"
//timer id
U16 bt_get_uart_callback_msg_id(void)
{
	return MSG_ID_UART_READY_TO_READ_IND;
}
//串口回调消息ID
U16 bt_get_base_timer_id(void)
{
	return BT_SMS_SEND_TIME_OUT_ID;
}
U16 bt_get_msg_id_nbr_cell_id(void)
{
	return MSG_ID_L4C_NBR_CELL_INFO_IND;
}
U16 bt_get_msg_id_nbr_reg_cnf(void)
{
	return MSG_ID_L4C_NBR_CELL_INFO_REG_CNF;
}
U16 bt_get_msg_id_nbr_reg_req(void)
{
	return MSG_ID_L4C_NBR_CELL_INFO_REG_REQ;
}
void bt_set_shutdown_ex(void)
{
	srv_shutdown_normal_start(APP_USBMMI);
}
void bt_frm_set_put_msg(PsIntFuncPtr funcPtr,U16 eventID)
{
	SetProtocolEventHandler(funcPtr, funcPtr);
}
void bt_frm_clear_msg(U16 eventID)
{
	ClearProtocolEventHandler(eventID); 
}
U16 bt_get_modle_mmi_id(void)
{
	return MOD_MMI;
}


void UART_ClrTxBuffer(UART_PORT port, module_type ownerid) 
{
 
signed long handle; 
 UART_CTRL_CLR_BUFFER_T data; 
 data.u4OwenrId = ownerid;
 
handle = DclSerialPort_Open(port,0); 
 DclSerialPort_Control(handle,SIO_CMD_CLR_TX_BUF, (DCL_CTRL_DATA_T*)&data);
 
}
void UART_ClrRxBuffer(UART_PORT port, module_type ownerid) 
{
 
signed long handle; 
 UART_CTRL_CLR_BUFFER_T data; 
 data.u4OwenrId = ownerid;
 
handle = DclSerialPort_Open(port,0); 
 DclSerialPort_Control(handle,SIO_CMD_CLR_RX_BUF, (DCL_CTRL_DATA_T*)&data);
 
}
kal_uint16 UART_GetBytes(UART_PORT port, kal_uint8 *Buffaddr, kal_uint16 Length, kal_uint8 *status, module_type ownerid)
{

	signed long handle;
	UART_CTRL_GET_BYTES_T data;

	data.u4OwenrId = ownerid;
	data.u2Length = Length;
	data.puBuffaddr = Buffaddr;
	data.pustatus = status;
	handle = DclSerialPort_Open(port,0);
	DclSerialPort_Control(handle,SIO_CMD_GET_BYTES, (DCL_CTRL_DATA_T*)&data);
	return data.u2RetSize;

}
kal_uint16 UART_PutBytes(UART_PORT port, kal_uint8 *Buffaddr, kal_uint16 Length,
module_type ownerid) 
{
 
	signed long handle; 
	UART_CTRL_PUT_BYTES_T data; 
	//UART_CTRL_PUT_UART_BYTE_T data; 

	data.u4OwenrId = ownerid; 
	data.u2Length = Length; 
	data.puBuffaddr = Buffaddr;

	handle = DclSerialPort_Open(port,0); 
	DclSerialPort_Control(handle,SIO_CMD_PUT_BYTES, (DCL_CTRL_DATA_T*)&data);  
	return data.u2RetSize;
 
}

DCL_UINT16 UART_GetBytesAvail(DCL_DEV port) 
{ 
	 signed long handle; 
	 UART_CTRL_RX_AVAIL_T data; 
 
 
	handle = DclSerialPort_Open(port,0); 
	DclSerialPort_Control(handle,SIO_CMD_GET_RX_AVAIL, (DCL_CTRL_DATA_T*)&data); 
	DclSerialPort_Close(handle); 
	return data.u2RetSize;
 
}
 void UART_SetBaudRate(UART_PORT port, UART_baudrate baudrate, module_type ownerid)
 {
 
	 signed long handle;
	 UART_CTRL_BAUDRATE_T data;
	 data.u4OwenrId = ownerid;
	 data.baudrate = baudrate;
	 
	 handle = DclSerialPort_Open(port,0);
	 DclSerialPort_Control(handle,SIO_CMD_SET_BAUDRATE, (DCL_CTRL_DATA_T*)&data);
 
 }
  kal_bool UART_Open(UART_PORT port, module_type ownerid)
 {
	 signed long handle;
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
	signed long handle;
	UART_CTRL_CLOSE_T data;
	data.u4OwenrId = ownerid;
	 handle = DclSerialPort_Open(port,0);
	DclSerialPort_Control(handle,SIO_CMD_CLOSE, (DCL_CTRL_DATA_T*)&data);		 
}
 static void UART_SetOwner(UART_PORT port, module_type ownerid) 
 {
  
 signed long handle; 
  UART_CTRL_OWNER_T data; 
  data.u4OwenrId = ownerid;
  
 handle = DclSerialPort_Open(port,0); 
  DclSerialPort_Control(handle,SIO_CMD_SET_OWNER, (DCL_CTRL_DATA_T*)&data);
 }
 static module_type UART_GetOwnerID(UART_PORT port) 
 {
	signed long handle; 
	UART_CTRL_OWNER_T data;

	handle = DclSerialPort_Open(port,0); 
	DclSerialPort_Control(handle,SIO_CMD_GET_OWNER_ID, (DCL_CTRL_DATA_T*)&data); 
	return (module_type)(data.u4OwenrId); 
 }
 void UART_SetDCBConfig(UART_PORT port, UARTDCBStruct *UART_Config, module_type ownerid) 
 { 
	signed long handle; 
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
    signed long handle;
    handle = DclSerialPort_Open( uart_port2, 0 );
    DclSerialPort_Control( handle, UART_CMD_BOOTUP_INIT, NULL );
}
static U8 lb_uart_ready_to_read_ind(void* msg)
{
	
}
void UART_Register_RX_cb(UART_PORT port, module_type ownerid, UART_RX_FUNC func)
{

	signed long handle;
	UART_CTRL_REG_RX_CB_T data;
	data.u4OwenrId = ownerid;
	data.func = func;

	handle = DclSerialPort_Open(port,0);
	DclSerialPort_Control(handle,SIO_CMD_REG_RX_CB, (DCL_CTRL_DATA_T*)&data);
}
void UART_Register_TX_cb(UART_PORT port, module_type ownerid, UART_TX_FUNC func)
{

	signed long handle;
	UART_CTRL_REG_TX_CB_T data;
	data.u4OwenrId = ownerid;
	data.func = func;

	handle = DclSerialPort_Open(port,0);
	DclSerialPort_Control(handle,SIO_CMD_REG_TX_CB, (DCL_CTRL_DATA_T*)&data);
}


#endif
//cusd
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
}
BT_CMD_RSP bt_send_cusd(char *_data)
{
	WCHAR wchar_ussd[200] = {0};
	BOOL result;

	if(_data == NULL) return;
	mmi_asc_to_ucs2((CHAR *)wchar_ussd, _data);
	bt_print(" %s",_data);
    result = mmi_ss_op_send(MMI_SIM1, (U8)mmi_ucs2strlen((CHAR*)wchar_ussd), wchar_ussd, bt_send_cusd_callback, NULL);
	bt_print("bt_send_cusd result %d",result);
    if (result != SRV_SS_RESULT_OK)
    {
		atcmd_put_data_string("Error");
		return CMD_RSP_OK;
    }
}
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
			return CMD_RSP_FAILD;
		}
		nmea_scanf(cmd_line->character, strlen(cmd_line->character), "AT+CUSD=1,\"%s\"",ussd_string);
		if(strlen(ussd_string) == 0)
		{
			atcmd_send_call_cusd_resault("OK");
			return CMD_RSP_OK;
		}
		bt_print("USSD string:%s",ussd_string);
		bt_send_cusd(ussd_string);
		return CMD_RSP_OK;

}
#endif

