/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
* 文件名  ： Coordinator
* 作者    ： Panmeiqin
* 版本    ： V1.0.0
* 时间    ： 2021/5/10
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
      GENERICAPP_MAX_CLUSTERS,
      (cId_t *)GenericApp_ClusterList,
      0,
      (cId_t *)NULL
};
endPointDesc_t GenericApp_epDesc; //定义节点描述符
byte GenericApp_TaskID;  //定义任务优先级
byte GenericApp_TransID;  //定义数据发送序列号
/* 函数声明---------------------------------------------------------------*/
void GenericApp_MessageMSGCB( afIncomingMSGPacket_t *pckt );//声明消息处理函数
void GenericApp_SendTheMessage( void );//声明数据发送函数
/* 外部变量引用 ----------------------------------------------------------*/




/* 函数 ------------------------------------------------------------------*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* 函数名：GenericApp_Init
* 参数：void
* 返回：void
* 作者：Panmeiqin
* 时间：2021/5/10
* 描述：任务初始化函数
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void GenericApp_Init( byte task_id )
{
    GenericApp_TaskID            = task_id;  //初始化任务优先级
    //发送数据包的序号初始化为0，接收端可以查看接收数据的序号来计算丢包率
    GenericApp_TransID           = 0;
    //初始化节点描述符
    GenericApp_epDesc.endPoint   = GENERICAPP_ENDPOINT;
    GenericApp_epDesc.task_id    = &GenericApp_TaskID;
    GenericApp_epDesc.simpleDesc = 
           (SimpleDescriptionFormat_t *)&GenericApp_SimpleDesc;
    GenericApp_epDesc.latencyReq = noLatencyReqs;
    afRegister( &GenericApp_epDesc );//使用afRegister函数将节点描述符进行注册
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* 函数名：GenericApp_ProcessEvent
* 参数：byte task_id, UINT16 events
* 返回：void
* 作者：Panmeiqin
* 时间：2021/5/10
* 描述：消息处理函数
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
UINT16 GenericApp_ProcessEvent( byte task_id, UINT16 events )
{
    afIncomingMSGPacket_t *MSGpkt; //定义指向接收消息结构体的指针
    if ( events & SYS_EVENT_MSG )
    {
        //使用osal_msg_receive函数从消息队列上接收消息
        MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( GenericApp_TaskID );
        while ( MSGpkt )
        {
            switch ( MSGpkt->hdr.event )
           {
                case AF_INCOMING_MSG_CMD: //对接收到的消息进行判断
                GenericApp_MessageMSGCB(  MSGpkt ); //接收到无线数据调用该函数处理
                break;
           default:
                break;
           }
           osal_msg_deallocate( (uint8 *)MSGpkt ); //释放消息所占据的存储空间
           MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive//直到所有消息都处理完
           ( GenericApp_TaskID );
    }
    return (events ^ SYS_EVENT_MSG);
  }
      return 0;
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* 函数名：GenericApp_MessageMSGCB
* 参数：afIncomingMSGPacket_t *pkt
* 返回：void
* 作者：Panmeiqin
* 时间：2021/5/10
* 描述：消息读取函数
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void GenericApp_MessageMSGCB( afIncomingMSGPacket_t *pkt )
{
    unsigned char buffer[4] = "    ";
    switch ( pkt->clusterId )
    {
    case GENERICAPP_CLUSTERID:
            osal_memcpy(buffer,pkt->cmd.Data,3); //将收到的数据拷贝到缓冲区buffer中
             //判断接收到的数据是不是LED
            if ((buffer[0] == 'L') || (buffer[1] == 'E') || (buffer[2] == 'D'))
           {
                HalLedBlink( HAL_LED_2,0,50,500);
           }
           else
          {
                  HalLedSet ( HAL_LED_2, HAL_LED_MODE_ON);
          }
        break;
    }
}
          
 