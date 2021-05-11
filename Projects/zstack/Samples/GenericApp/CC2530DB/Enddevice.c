/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
* 文件名  ： Enddevice
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
      0,
      (cId_t *)NULL,
      GENERICAPP_MAX_CLUSTERS,
      (cId_t *)GenericApp_ClusterList
};
endPointDesc_t GenericApp_epDesc; //定义节点描述符
byte GenericApp_TaskID;  //定义任务优先级
byte GenericApp_TransID;  //定义数据发送序列号
devStates_t GenericApp_NwkState;
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
    GenericApp_NwkState          = DEV_INIT;//设备初始化为DEV_INIT
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
        ( GenericApp_TaskID );
        while ( MSGpkt )
        {
            switch ( MSGpkt->hdr.event )
           {
                case ZDO_STATE_CHANGE: 
                GenericApp_NwkState = (devStates_t) (MSGpkt->hdr.status);//读取节点的设备类型
                if(GenericApp_NwkState == DEV_END_DEVICE)//对节点设备类型进行判断
                {
                    GenericApp_SendTheMessage();
                }
                break;
           default:
                break;
           }
           osal_msg_deallocate( (uint8 *)MSGpkt ); //释放消息所占据的存储空间
          //直到所有消息都处理完
           MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive 
           ( GenericApp_TaskID );
    }
    return (events ^ SYS_EVENT_MSG);
  }
      return 0;
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* 函数名：GenericApp_SendTheMessage
* 参数：void
* 返回：void
* 作者：Panmeiqin
* 时间：2021/5/10
* 描述：消息读取函数
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void GenericApp_SendTheMessage( void )
{
    unsigned char theMessageData[4] = "LED";//定义数组，用于存放要发送的数据
    afAddrType_t my_DstAddr;//定义afAddrType_t类型的变量my_DstAddr
    my_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;//将发送地址模式设置为单播
    my_DstAddr.endPoint = GENERICAPP_ENDPOINT;//初始化端口号
    my_DstAddr.addr.shortAddr  = 0x0000;//协调器的网络地址是固定的
    AF_DataRequest( &my_DstAddr, &GenericApp_epDesc,//调用数据发送函数 AF_DataRequest进行无线数据的发送
                      GENERICAPP_CLUSTERID,
                      3,
                      theMessageData,
                      &GenericApp_TransID,
                      AF_DISCV_ROUTE,
                      AF_DEFAULT_RADIUS );
    HalLedBlink(HAL_LED_2,0,50,500);//调用HalLedBlink函数，使终端节点的LED2闪烁
}
          
