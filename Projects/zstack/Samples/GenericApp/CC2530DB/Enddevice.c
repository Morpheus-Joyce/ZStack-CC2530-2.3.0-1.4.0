/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
* 文件名  ： Enddevice
* 作者    ： Suyuqi
* 版本    ： V1.0.0
* 时间    ： 2021/05/10
* 简要    ： 文件说明  
********************************************************************
* 副本
*
*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
/* 头文件 ----------------------------------------------------------------*/
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
/* 宏定义 ----------------------------------------------------------------*/
/* 结构体或枚举 ----------------------------------------------------------*/
//与协议栈里的数据的定义的格式保持一致
const cId_t GenericApp_ClusterList[GENERICAPP_MAX_CLUSTERS] = 
{
    GENERICAPP_CLUSTERID
};
//描述一个ZigBee设备节点
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
//定义四个变量：节点描述符GenericApp_epDesc，任务优先级GenericApp_TaskID，
//数据发送序列号GenericApp_TransID,保存节点状态的变量GenericApp_NwkState
endPointDesc_t GenericApp_epDesc;
byte GenericApp_TaskID;
byte GenericApp_TransID;
devStates_t GenericApp_NwkState;
/* 函数声明---------------------------------------------------------------*/
//消息处理函数GeneriApp_MessageMSGCB
void GenericApp_MessageMSGCB( afIncomingMSGPacket_t *pckt );
//数据发送函数GenericAPP_SendTheMessage
void GenericApp_SendTheMessage( void );
/* 外部变量引用 ----------------------------------------------------------*/




/* 函数 ------------------------------------------------------------------*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* 函数名：GenericApp_Init
* 参数：byte task_id
* 返回：void
* 作者：Suyuqi
* 时间：2021/05/10
* 描述：函数说明
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void GenericApp_Init(byte task_id)
{
    //初始化任务优先级
    GenericApp_TaskID                   =task_id;
    //将设备状态初始化为DEV_INIT，表示该节点没有连接到ZigBee网络
    GenericApp_NwkState                 =DEV_INIT;
    //将发送数据包的序号初始化为0
    GenericApp_TransID                  = 0;
    //对节点描述符进行初始化
    GenericApp_epDesc.endPoint          = GENERICAPP_ENDPOINT;
    GenericApp_epDesc.task_id           = &GenericApp_TaskID;
    GenericApp_epDesc.simpleDesc        =
        (SimpleDescriptionFormat_t *)&GenericApp_SimpleDesc;
    GenericApp_epDesc.latencyReq        = noLatencyReqs;
    //使用afRegister函数将系统描述符进行注册
    afRegister( &GenericApp_epDesc );
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* 函数名：GenericApp_ProcessEvent
* 参数：byte task_id, UINT16 events
* 返回：UINT16
* 作者：Suyuqi
* 时间：2021/05/10
* 描述：消息处理函数
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
UINT16 GenericApp_ProcessEvent(byte task_id, UINT16 events)
{
        //定义一个指向接收消息结构体的指针MSGpkt
    afIncomingMSGPacket_t *MSGpkt;
    if ( events & SYS_EVENT_MSG )
    {
        //使用osal_msg_receive函数从消息队列上接收消息，包含接收到的无线数据包
        MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( GenericApp_TaskID );
        while ( MSGpkt )
        {
            switch ( MSGpkt->hdr.event )
            {
            case ZDO_STATE_CHANGE:
                //读取节点的设备类型
                GenericApp_NwkState = (devStates_t)(MSGpkt->hdr.status);
                //对节点类型进行判断
                if (GenericApp_NwkState == DEV_END_DEVICE)
                {
                    //是终端节点实现无线数据发送
                    GenericApp_SendTheMessage();
                }
                break;
            default:
                break;
            }
            //释放空间
            osal_msg_deallocate( (uint8 *)MSGpkt );
            //处理完一个消息后再从消息队列中接收消息，然后进行处理，直到所有消息处理完
            MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( GenericApp_TaskID );
        }
        return (events ^ SYS_EVENT_MSG);
    }
    return 0;
}
    
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* 函数名：GenericApp_SendTheMessage
* 参数：void
* 返回：void
* 作者：Suyuqi
* 时间：2021/05/10
* 描述：数据发送函数
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


