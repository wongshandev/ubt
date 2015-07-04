#ifndef ___YEELINK___
#define ___YEELINK___
#include "MMI_features.h"
#include "MMIDataType.h"
#include "app2soc_struct.h"
#include "soc_api.h"
#include "em_struct.h"
#include "GlobalResDef.h"
#include "DataAccountStruct.h"
#include "DateTimeType.h" 
#include "app_datetime.h"
#include "TimerEvents.h"

#define MAX_SOC_BUFFER_SIZE (1024+512)
#define MIN_IPADDR_STREN_LEN   7

typedef enum{
	YEELINK_CONECT_SUCCEED = 1,//���ӳɹ���
	
	YEELINK_RECONECT,//���ɹ�������.
	
	YEELINK_FAIL,//ʧ��
	
	YEELINK_VAIL,//��������
	
	YEELINK_READ_DATA,//��������
	
	YEELINK_SEND_SUCCEED,//���ͳɹ�
	
	YEELINK_CLOSED,//�ر�
}YEELINK_SOC_CONECT_RESULT;

typedef void (*Yeelink_Soc_Conect_CB)(YEELINK_SOC_CONECT_RESULT result,void *data);


typedef struct _YeeLinkStruct{

	sockaddr_struct addr2;
	
	signed Soc_hand;

	BOOL isActiveConnect;

	U8 app_id;

	U8  *_pSendbuffer;

	U8  *_pRecverBuffer;

	Yeelink_Soc_Conect_CB CallBack;

	float f_data;

	kal_uint32 account_id;

}YeeLinkStruct;
extern void YeeLink_Post_Info(float Data,int v);
extern void YeeLink_Conect_Soc(sockaddr_struct Ip_addr,U8 * Content,Yeelink_Soc_Conect_CB Call_Back);
extern void YeeLink_GetHostByName(const kal_uint8 *hostname,kal_uint8 * host);
#endif
