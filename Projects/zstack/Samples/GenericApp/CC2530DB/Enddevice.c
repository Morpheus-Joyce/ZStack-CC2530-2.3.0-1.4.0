/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
* �ļ���  �� Enddevice
* ����    �� Suyuqi
* �汾    �� V1.0.0
* ʱ��    �� 2021/05/10
* ��Ҫ    �� �ļ�˵��  
********************************************************************
* ����
*
*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
/* ͷ�ļ� ----------------------------------------------------------------*/
#include "OSAL.h"
#include "AF.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "ZDProfile.h"
#include <string.h>
#include "Coordinator.h"
#include "DebugTrace.h"

#if !defined( WIN32 )
#include "OnBoard.h"
#endif

#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_uart.h"
/* �궨�� ----------------------------------------------------------------*/
/* �ṹ���ö�� ----------------------------------------------------------*/
//��Э��ջ������ݵĶ���ĸ�ʽ����һ��
const cId_t GenericApp_ClusterList[GENERICAPP_MAX_CLUSTERS] = 
{
    GENERICAPP_CLUSTERID
};
//����һ��ZigBee�豸�ڵ�
const SimpleDescriptionFormat_t GenericApp_SimpleDesc =
{
    GENERICAPP_ENDPOINT,
    GENERICAPP_PROFID,
    GENERICAPP_DEVICEID,
    GENERICAPP_DEVICE_VERSION,
    GENERICAPP_FLAGS,
    0,
    (cId_t *)NULL,
    GENERICAPP_MAX_CLUSTERS,
    (cId_t *)GenericApp_ClusterList,
};
//�����ĸ��������ڵ�������GenericApp_epDesc���������ȼ�GenericApp_TaskID��
//���ݷ������к�GenericApp_TransID,����ڵ�״̬�ı���GenericApp_NwkState
endPointDesc_t GenericApp_epDesc;
byte GenericApp_TaskID;
byte GenericApp_TransID;
devStates_t GenericApp_NwkState;
/* ��������---------------------------------------------------------------*/
//��Ϣ������GeneriApp_MessageMSGCB
void GenericApp_MessageMSGCB( afIncomingMSGPacket_t *pckt );
//���ݷ��ͺ���GenericAPP_SendTheMessage
void GenericApp_SendTheMessage( void );
/* �ⲿ�������� ----------------------------------------------------------*/




/* ���� ------------------------------------------------------------------*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* ��������GenericApp_Init
* ������byte task_id
* ���أ�void
* ���ߣ�Suyuqi
* ʱ�䣺2021/05/10
* ����������˵��
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void GenericApp_Init(byte task_id)
{
    //��ʼ���������ȼ�
    GenericApp_TaskID                   =task_id;
    //���豸״̬��ʼ��ΪDEV_INIT����ʾ�ýڵ�û�����ӵ�ZigBee����
    GenericApp_NwkState                 =DEV_INIT;
    //���������ݰ�����ų�ʼ��Ϊ0
    GenericApp_TransID                  = 0;
    //�Խڵ����������г�ʼ��
    GenericApp_epDesc.endPoint          = GENERICAPP_ENDPOINT;
    GenericApp_epDesc.task_id           = &GenericApp_TaskID;
    GenericApp_epDesc.simpleDesc        =
        (SimpleDescriptionFormat_t *)&GenericApp_SimpleDesc;
    GenericApp_epDesc.latencyReq        = noLatencyReqs;
    //ʹ��afRegister������ϵͳ����������ע��
    afRegister( &GenericApp_epDesc );
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* ��������GenericApp_ProcessEvent
* ������byte task_id, UINT16 events
* ���أ�UINT16
* ���ߣ�Suyuqi
* ʱ�䣺2021/05/10
* ��������Ϣ������
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
UINT16 GenericApp_ProcessEvent(byte task_id, UINT16 events)
{
        //����һ��ָ�������Ϣ�ṹ���ָ��MSGpkt
    afIncomingMSGPacket_t *MSGpkt;
    if ( events & SYS_EVENT_MSG )
    {
        //ʹ��osal_msg_receive��������Ϣ�����Ͻ�����Ϣ���������յ����������ݰ�
        MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( GenericApp_TaskID );
        while ( MSGpkt )
        {
            switch ( MSGpkt->hdr.event )
            {
            case ZDO_STATE_CHANGE:
                //��ȡ�ڵ���豸����
                GenericApp_NwkState = (devStates_t)(MSGpkt->hdr.status);
                //�Խڵ����ͽ����ж�
                if (GenericApp_NwkState == DEV_END_DEVICE)
                {
                    //���ն˽ڵ�ʵ���������ݷ���
                    GenericApp_SendTheMessage();
                }
                break;
            default:
                break;
            }
            //�ͷſռ�
            osal_msg_deallocate( (uint8 *)MSGpkt );
            //������һ����Ϣ���ٴ���Ϣ�����н�����Ϣ��Ȼ����д���ֱ��������Ϣ������
            MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( GenericApp_TaskID );
        }
        return (events ^ SYS_EVENT_MSG);
    }
    return 0;
}
    
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* ��������GenericApp_SendTheMessage
* ������void
* ���أ�void
* ���ߣ�Suyuqi
* ʱ�䣺2021/05/10
* ���������ݷ��ͺ���
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void GenericApp_SendTheMessage(void)
{
    unsigned char theMessageData[4] = "LED";
    afAddrType_t my_DstAddr;
    my_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;
    my_DstAddr.endPoint = GENERICAPP_ENDPOINT;
    my_DstAddr.addr.shortAddr = 0x0000;
    AF_DataRequest( &my_DstAddr, &GenericApp_epDesc,
                   GENERICAPP_CLUSTERID,
                   3,
                   theMessageData,
                   &GenericApp_TransID,
                   AF_DISCV_ROUTE,
                   AF_DEFAULT_RADIUS);
    HalLedBlink(HAL_LED_2,0,50,500);
}


