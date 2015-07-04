#ifndef __bt_at_command__
#define __bt_at_command__
#include "bt_ps.h"
#include "ata_external.h"
#include "mmi_rp_app_ucm_def.h"
#ifndef WIN32
#include "uart_internal.h"
#endif
#include "soc_api.h"

#include "YeeLink.h"
typedef enum{

CMD_RSP_FAILD,
CMD_RSP_OK,
}BT_CMD_RSP;
typedef struct  
{  
 kal_uint8 mcc[3];// MCC 
 kal_uint8 mnc[3];// MNC 
 kal_uint8 lac[2];//LAC 
 kal_uint16 cell_id;//cell ID 
 kal_int16 quarter_dbm;
} Serving_Cell_Info_struct;

typedef struct
{
	char *commandString;
	BT_CMD_RSP (*commandFunc)(custom_cmdLine *commandBuffer_p);
} bt_custom_atcmd;

typedef enum{

	BT_AT_SEND_TIMER_OUT=BT_AT_ADD_TIMER_3,


}BT_AT_TIMER;

#ifndef WIN32
extern kal_bool U_Open(UART_PORT port, module_type owner);
extern void U_Close(UART_PORT port, module_type ownerid);
extern void U_Purge(UART_PORT port, UART_buffer dir, module_type ownerid);
//extern void U_SetOwner (UART_PORT port, kal_uint8 owner);
extern void U_SetOwner (UART_PORT port, module_type owner);
extern void U_SetFlowCtrl(UART_PORT port, kal_bool XON, module_type ownerid);
extern void U_CtrlDCD(UART_PORT port, IO_level SDCD, module_type ownerid);
extern void U_ConfigEscape (UART_PORT port, kal_uint8 EscChar, kal_uint16 ESCGuardtime, module_type ownerid);
extern void U_SetDCBConfig(UART_PORT port, UARTDCBStruct *UART_Config, module_type ownerid);
extern void U_CtrlBreak(UART_PORT port, IO_level SBREAK, module_type ownerid);
extern void U_ClrRxBuffer(UART_PORT port, module_type ownerid);
extern void U_ClrTxBuffer(UART_PORT port, module_type ownerid);
extern void U_SetBaudRate(UART_PORT port, UART_baudrate baudrate, module_type ownerid);
extern module_type U_GetOwnerID(UART_PORT port);
extern void U_SetAutoBaud_Div(UART_PORT port, module_type ownerid);
extern void U_Register_TX_cb(UART_PORT port, module_type owner, UART_TX_FUNC func);
extern void U_Register_RX_cb(UART_PORT port, module_type owner, UART_RX_FUNC func);
extern void U_PutUARTBytes(UART_PORT port, kal_uint8 *data,kal_uint16 len);
extern void U_ReadDCBConfig (UART_PORT port, UARTDCBStruct *DCB);
extern void U_CtrlRI (UART_PORT port, IO_level SRI, module_type ownerid);   /*NULL for DCE*/
extern void U_CtrlDTR (UART_PORT port, IO_level SDTR, module_type ownerid);
extern void U_ReadHWStatus(UART_PORT port, IO_level *SDSR, IO_level *SCTS);
#endif
kal_bool bt_custom_atcmd_from_uart(char *cmd_string);
extern  void bt_gps_parse_loop(void);
extern void YeeLink_CloseSocket(void);
extern BOOL YeeLink_GetSocketStatus(void);
extern BOOL YeeLink_SetAddr(sockaddr_struct _addr);
extern sockaddr_struct* YeeLink_GetAddr(void);
#endif
