#include "MMI_features.h"

#ifndef __BT_PS_HH__
#define __BT_PS_HH__
#include "gui_typedef.h"
#include "aud_defs.h"
#include "mdi_datatype.h"
#include "mdi_audio.h"
#include "mdi_include.h"
#include "Device.h"
#include "kal_non_specific_general_types.h"
#include "gui_data_types.h"
#include "gui_virtual_keyboard_language_type.h"
#include "wgui_inline_edit.h"
#include "CustDataRes.h"
#include "GlobalResDef.h"
#include "Unicodexdcl.h"
#include "kal_release.h"
#include "string.h"
#include "DebugInitDef_Int.h"
#include "nvram_common_defs.h"
#include "mmi_rp_app_fmrdo_def.h"
#include "mmi_frm_nvram_gprot.h"
#include "gui_typedef.h"
#include "wgui_categories_util.h"
#include "AlertScreen.h"
#include "mmi_frm_events_gprot.h"
#include "mmi_frm_scenario_gprot.h"
#include "TimerEvents.h"
#include "mmi_frm_timer_gprot.h"
#include "mmi_frm_input_gprot.h"
#include "GlobalConstants.h"
#include "custom_mmi_default_value.h"
#include "fs_func.h"
#include "fs_type.h"
#include "fs_errcode.h"
#include "mmi_res_range_def.h"
#include "mmi_frm_history_gprot.h"
#include "gdi_const.h"
#include "wgui.h"
#include "gui_effect_oem.h"
#include "gui.h"
#include "wgui_categories_list.h"
#include "wgui_categories.h"
#include "ImeGprot.h"
#include "CommonScreensResDef.h"
#include "wgui_categories_popup.h"
#include "custom_events_notify.h"
#include "mmi_rp_file_type_def.h"
#include "FileMgrType.h"
#include "gui_touch_feedback.h"
#include "FMRadioDef.h"
#include "FMRadioType.h"
#include "FMRadioProt.h"
#include "FMRadioMainScreen.h"
#include "GpiosrvGprot.h"
#include "gui_font_size.h"
#include "MMIDataType.h"
#include "UcmGProt.h"
#include "IdleGprot.h"
#include "fseditorcuigprot.h"
#include "SmsSrvGprot.h"
#include "Conversions.h"
#include "med_utility.h"
#include "bt_gps.h"

#define BT_SIM_CARD_USE   SRV_SMS_SIM_1


#define MAX_LOC_COUNT 5 //5组LOC
typedef struct _bt_sms_struct{

BOOL bSendResult;
 
}bt_sms_struct;
typedef struct _at_callback_flg
{
	BOOL sms_is_sending;
}at_callback_flg;
/*
MTK 平台都支持通过 SSC code 来修改 PIN/PIN2 。具体如下：
**04*<old PIN>*<new PIN>*<new PIN>#
**05*<PUK>*<new PIN>*<new PIN>#
以上两种 SSC code 格式都可以用来修改 PIN ，区别在于，“ **04 ”要求输入正确的 PIN ，而
“ **05 ”要求输入正确的 PUK 。
**042*<old PIN2>*<new PIN2>*<new PIN2>#
**052*<PUK2>*<new PIN2>*<new PIN2>#
以上两种 SSC code 格式都可以用来修改 PIN2 ，区别在于，“ **042 ”要求输入正确的 PIN2
，而“ **052 ”要求输入正确的 PUK2 。
例如，如果想将一张 SIM 卡的 PIN 码由“ 1234 ”修改为“ 0000 ”，需要在 Dialer Screen 输入“
**04*<old PIN>*<new PIN>*<new PIN># ”这种格式的 SSC code （如下图）。下图中已经输入“
**04*1234*0000*0000 ”，再输入一个“ # ”就会执行 change pin 功能。
*/
//mmi_ssc_sim_table 修改PINK
typedef struct _bt_struct
{
	bt_sms_struct btSms;

	U8 signal;

	char btIMSI[20];

	char btIMEI[50];

	int loc_index;
	rr_em_lai_info_struct loc[MAX_LOC_COUNT];
	kal_int32 rssi_in_qdbm;/* 如果发现 level 是 0~31 之
							间的数值，需要使用下面表格将这些自然数重新影射回 DBM。计算公式(-113 + RSSI)/2 */
	at_callback_flg   at_flag;

	BOOL			  AT_getGpsInfoFlg;//取GPS信息时有个延迟，需要FLG
	BOOL              GPS_isWork;//GPS是否工作
}bt_struct;



extern bt_struct gBtps;
extern void bt_print(char* fmt,...);
extern void AT_SendLocGPS_ex(btGPSInfo *GPS_info);
#endif
