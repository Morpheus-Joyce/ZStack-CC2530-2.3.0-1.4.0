/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
* �ļ���  �� Enddevice
* ����    �� Panmeiqin
* �汾    �� V1.0.0
* ʱ��    �� 2021/5/10
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
const cId_t GenericApp_ClusterList[GENERICAPP_MAX_CLUSTERS] =
{
    GENERICAPP_CLUSTERID
};
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
      (cId_t *)GenericApp_ClusterList
};
endPointDesc_t GenericApp_epDesc; //����ڵ�������
byte GenericApp_TaskID;  //�����������ȼ�
byte GenericApp_TransID;  //�������ݷ������к�
devStates_t GenericApp_NwkState;
/* ��������---------------------------------------------------------------*/
void GenericApp_MessageMSGCB( afIncomingMSGPacket_t *pckt );//������Ϣ������
void GenericApp_SendTheMessage( void );//�������ݷ��ͺ���
/* �ⲿ�������� ----------------------------------------------------------*/




/* ���� ------------------------------------------------------------------*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* ��������GenericApp_Init
* ������void
* ���أ�void
* ���ߣ�Panmeiqin
* ʱ�䣺2021/5/10
* �����������ʼ������
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void GenericApp_Init( byte task_id )
{
    GenericApp_TaskID            = task_id;  //��ʼ���������ȼ�
    GenericApp_NwkState          = DEV_INIT;//�豸��ʼ��ΪDEV_INIT
    //�������ݰ�����ų�ʼ��Ϊ0�����ն˿��Բ鿴�������ݵ���������㶪����
    GenericApp_TransID           = 0;
    //��ʼ���ڵ�������
    GenericApp_epDesc.endPoint   = GENERICAPP_ENDPOINT;
    GenericApp_epDesc.task_id    = &GenericApp_TaskID;
    GenericApp_epDesc.simpleDesc = 
           (SimpleDescriptionFormat_t *)&GenericApp_SimpleDesc;
    GenericApp_epDesc.latencyReq = noLatencyReqs;
    afRegister( &GenericApp_epDesc );//ʹ��afRegister�������ڵ�����������ע��
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* ��������GenericApp_ProcessEvent
* ������byte task_id, UINT16 events
* ���أ�void
* ���ߣ�Panmeiqin
* ʱ�䣺2021/5/10
* ��������Ϣ������
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
UINT16 GenericApp_ProcessEvent( byte task_id, UINT16 events )
{
    afIncomingMSGPacket_t *MSGpkt; //����ָ�������Ϣ�ṹ���ָ��
    if ( events & SYS_EVENT_MSG )
    {
        //ʹ��osal_msg_receive��������Ϣ�����Ͻ�����Ϣ
        MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( GenericApp_TaskID );
        ( GenericApp_TaskID );
        while ( MSGpkt )
        {
            switch ( MSGpkt->hdr.event )
           {
                case ZDO_STATE_CHANGE: 
                GenericApp_NwkState = (devStates_t) (MSGpkt->hdr.status);//��ȡ�ڵ���豸����
                if(GenericApp_NwkState == DEV_END_DEVICE)//�Խڵ��豸���ͽ����ж�
                {
                    GenericApp_SendTheMessage();
                }
                break;
           default:
                break;
           }
           osal_msg_deallocate( (uint8 *)MSGpkt ); //�ͷ���Ϣ��ռ�ݵĴ洢�ռ�
          //ֱ��������Ϣ��������
           MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive 
           ( GenericApp_TaskID );
    }
    return (events ^ SYS_EVENT_MSG);
  }
      return 0;
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* ��������GenericApp_SendTheMessage
* ������void
* ���أ�void
* ���ߣ�Panmeiqin
* ʱ�䣺2021/5/10
* ��������Ϣ��ȡ����
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void GenericApp_SendTheMessage( void )
{
    unsigned char theMessageData[4] = "LED";//�������飬���ڴ��Ҫ���͵�����
    afAddrType_t my_DstAddr;//����afAddrType_t���͵ı���my_DstAddr
    my_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;//�����͵�ַģʽ����Ϊ����
    my_DstAddr.endPoint = GENERICAPP_ENDPOINT;//��ʼ���˿ں�
    my_DstAddr.addr.shortAddr  = 0x0000;//Э�����������ַ�ǹ̶���
    AF_DataRequest( &my_DstAddr, &GenericApp_epDesc,//�������ݷ��ͺ��� AF_DataRequest�����������ݵķ���
                      GENERICAPP_CLUSTERID,
                      3,
                      theMessageData,
                      &GenericApp_TransID,
                      AF_DISCV_ROUTE,
                      AF_DEFAULT_RADIUS );
    HalLedBlink(HAL_LED_2,0,50,500);//����HalLedBlink������ʹ�ն˽ڵ��LED2��˸
}
          
