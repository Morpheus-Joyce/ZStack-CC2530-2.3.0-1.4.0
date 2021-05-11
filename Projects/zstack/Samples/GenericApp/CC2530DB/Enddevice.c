/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
* 文件名  ： Enddevice
* 作者    ： liuping
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
//演示如何实现点对点通信
const cId_t GenericApp_ClusterList[GENERICAPP_MAX_CLUSTERS] =
{
  GENERICAPP_CLUSTERID
};
//简单设备描述
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

endPointDesc_t GenericApp_epDesc;  //节点描述符
byte GenericApp_TaskID;    //任务优先级
byte GenericApp_TransID;    //数据发送序列号
devStates_t GenericApp_NwkState;  //保存节点状态的变量
/* 函数声明---------------------------------------------------------------*/
/* 外部变量引用 ----------------------------------------------------------*/



/* 函数 ------------------------------------------------------------------*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* 函数名：GenericApp_Init
* 参数：byte task_id
* 返回：void
* 作者：liuping、panmeiqin、suyuqi、zhouwenrui
* 时间：2021/5/10
* 描述：初始化函数
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void GenericApp_Init( byte task_id )  
{
  GenericApp_TaskID = task_id;  //初始化任务优先级
  GenericApp_NwkState = DEV_INIT;
  GenericApp_TransID = 0;  //数据发送序列号初始化
   GenericApp_epDesc.endPoint = GENERICAPP_ENDPOINT;  //节点描述初始化
  GenericApp_epDesc.task_id = &GenericApp_TaskID;
  GenericApp_epDesc.simpleDesc
            = (SimpleDescriptionFormat_t *)&GenericApp_SimpleDesc;
  GenericApp_epDesc.latencyReq = noLatencyReqs;
  afRegister( &GenericApp_epDesc );  //将节点描述符进行注册
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* 函数名：GenericApp_ProcessEvent
* 参数：byte task_id, UINT16 events
* 返回：UINT16
* 作者：liuping、panmeiqin、suyuqi、zhouwenrui
* 时间：2021/5/10
* 描述：消息处理函数
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
UINT16 GenericApp_ProcessEvent( byte task_id, UINT16 events )
{
  afIncomingMSGPacket_t *MSGpkt;  //定义了一个指向接收消息结构体的指针
   if ( events & SYS_EVENT_MSG )
  {
    MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( GenericApp_TaskID );  //从消息队列上接收消息
    while ( MSGpkt )
    {
      switch ( MSGpkt->hdr.event )
      {
          case ZDO_STATE_CHANGE:
          GenericApp_NwkState = (devStates_t)(MSGpkt->hdr.status);
          if (GenericApp_NwkState == DEV_END_DEVICE)
          {
              GenericApp_SendTheMessage();
          }
          break;
      default:
          break;
      }
      osal_msg_deallocate( (uint8 *)MSGpkt );
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
* 作者：liuping、panmeiqin、suyuqi、zhouwenrui
* 时间：2021/5/10
* 描述：数据发送
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void GenericApp_SendTheMessage ( void )
{
    unsigned char theMessageData[4]="LED";
    afAddrType_t my_DstAddr;
    my_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;
    my_DstAddr.endPoint = GENERICAPP_ENDPOINT;
    my_DstAddr.addr.shortAddr =0x0000;
    AF_DataRequest ( &my_DstAddr,&GenericApp_epDesc,
                    GENERICAPP_CLUSTERID,
                    3,
                    theMessageData,
                    &GenericApp_TransID,
                    AF_DISCV_ROUTE,
                    AF_DEFAULT_RADIUS );
    HalLedBlink (HAL_LED_2,0,50,500);
}
              