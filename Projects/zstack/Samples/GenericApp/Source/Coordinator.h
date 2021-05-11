#ifndef _COORDINATOR_H  
#define _COORDINATOR_H

/* 头文件 ----------------------------------------------------------------*/
#include<iocc2530.h>

#include "ZComDef.h"

#define GENERICAPP_ENDPOINT          10

#define GENERICAPP_PROFID            0x0f04
#define GENERICAPP_DEVICEID          0x0001
#define GENERICAPP_DEVICE_VERSION    0
#define GENERICAPP_FLAGS             0
#define GENERICAPP_MAX_CLUSTERS      1
#define GENERICAPP_CLUSTERID         1

extern void GenericApp_Init( byte task_id );
extern UINT16 GenericApp_ProcessEvent( byte task_id, UINT16 events);
/* 宏定义 ----------------------------------------------------------------*/
/* 结构体或枚举 ----------------------------------------------------------*/
/* 函数声明---------------------------------------------------------------*/
/* 外部变量引用 ----------------------------------------------------------*/


#endif // _COORDINATOR_H   

