/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
* 文件名  ： Coordinator
* 作者    ： Suyuqi
* 版本    ： V1.0.0
* 时间    ： 2021/05/10
* 简要    ： 文件说明  
********************************************************************
* 副本
*
*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
/* 头文件 ----------------------------------------------------------------*/
#include "Coordinator.h"
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
//描述一个ZigBee设备节点，称为简单设备描述符
const SimpleDescriptionFormat_t GenericApp_SimpleDesc =
{
    GENERICAPP_ENDPOINT,
    GENERICAPP_PROFID,
    GENERICAPP_DEVICEID,
    GENERICAPP_DEVICE_VERSION,
    GENERICAPP_FLAGS,
    GENERICAPP_MAX_CLUSTERS,
    (cId_t *)GenericApp_ClusterList,
    0,
    (cId_t *)NULL
};
//节点描述符GenericApp_epDesc，任务优先级GenericApp_TaskID，
//数据发送序列号GenericApp_TransID
endPointDesc_t GenericApp_epDesc;
byte GenericApp_TaskID;
byte GenericApp_TransID;
/* 函数声明---------------------------------------------------------------*/
//消息处理函数GeneriApp_MessageMSGCB，数据发送函数GenericAPP_SendTheMessage
void GenericApp_MessageMSGCB( afIncomingMSGPacket_t *pckt );
void GenericApp_SendTheMessage( void );
/* 外部变量引用 ----------------------------------------------------------*/




/* 函数 ------------------------------------------------------------------*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* 函数名：GenericApp_Init
* 参数：void
* 返回：void
* 作者：Suyuqi
* 时间：2021/05/10
* 描述：任务初始化
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void GenericApp_Init( byte task_id )
{
    //初始化任务优先级
    GenericApp_TaskID                   =task_id;
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
* 描述：函数说明
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
                //对接收到的消息进行判断
            case AF_INCOMING_MSG_CMD:
                GenericApp_MessageMSGCB( MSGpkt );
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
* 函数名：GenericApp_MessageMSGCB
* 参数：afIncomingMSGPacket_t *pkt
* 返回：void
* 作者：Suyuqi
* 时间：2021/05/10
* 描述：消息处理函数
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void GenericApp_MessageMSGCB(afIncomingMSGPacket_t *pkt)
{
    unsigned char buffer[4] = "   ";
    switch ( pkt->clusterId )
    {
    case GENERICAPP_CLUSTERID:
        //将受到的数据拷贝到缓冲区buffer中
        osal_memcpy(buffer,pkt->cmd.Data,3);
        //判断收到的数据是不是“LED”三个字符，是则使LED2闪烁，不是则点亮LED2
        if((buffer[0] == 'L') || (buffer[1] == 'E') || (buffer[2] == 'D'))
        {
            HalLedBlink(HAL_LED_2,0,50,500);
        }
        else
        {
            HalLedSet(HAL_LED_2,HAL_LED_MODE_ON);
        }
        break;
    }
}