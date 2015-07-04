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


#define MAX_LOC_COUNT 5 //5��LOC
typedef struct _bt_sms_struct{

BOOL bSendResult;
 
}bt_sms_struct;
typedef struct _at_callback_flg
{
	BOOL sms_is_sending;
}at_callback_flg;
/*
MTK ƽ̨��֧��ͨ�� SSC code ���޸� PIN/PIN2 ���������£�
**04*<old PIN>*<new PIN>*<new PIN>#
**05*<PUK>*<new PIN>*<new PIN>#
�������� SSC code ��ʽ�����������޸� PIN ���������ڣ��� **04 ��Ҫ��������ȷ�� PIN ����
�� **05 ��Ҫ��������ȷ�� PUK ��
**042*<old PIN2>*<new PIN2>*<new PIN2>#
**052*<PUK2>*<new PIN2>*<new PIN2>#
�������� SSC code ��ʽ�����������޸� PIN2 ���������ڣ��� **042 ��Ҫ��������ȷ�� PIN2
������ **052 ��Ҫ��������ȷ�� PUK2 ��
���磬����뽫һ�� SIM ���� PIN ���ɡ� 1234 ���޸�Ϊ�� 0000 ������Ҫ�� Dialer Screen ���롰
**04*<old PIN>*<new PIN>*<new PIN># �����ָ�ʽ�� SSC code ������ͼ������ͼ���Ѿ����롰
**04*1234*0000*0000 ����������һ���� # ���ͻ�ִ�� change pin ���ܡ�
*/
//mmi_ssc_sim_table �޸�PINK
typedef struct _bt_struct
{
	bt_sms_struct btSms;

	U8 signal;

	char btIMSI[20];

	char btIMEI[50];

	int loc_index;
	rr_em_lai_info_struct loc[MAX_LOC_COUNT];
	kal_int32 rssi_in_qdbm;/* ������� level �� 0~31 ֮
							�����ֵ����Ҫʹ����������Щ��Ȼ������Ӱ��� DBM�����㹫ʽ(-113 + RSSI)/2 */
	at_callback_flg   at_flag;

	BOOL			  AT_getGpsInfoFlg;//ȡGPS��Ϣʱ�и��ӳ٣���ҪFLG
	BOOL              GPS_isWork;//GPS�Ƿ���
}bt_struct;



extern bt_struct gBtps;
extern void bt_print(char* fmt,...);
extern void AT_SendLocGPS_ex(btGPSInfo *GPS_info);
#endif
