// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/xlogging.h"


#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "cmsis_os.h"
#include "AzureIOTSDKConfig.h"
#include "ThreadAPITypes.h"

/** @addtogroup MIDDLEWARES
* @{
*/ 

/** @addtogroup AZURE_SDK
* @{
*/ 

/** @defgroup  AZURE_ADAPTER_THREAD
  * @brief Wi-Fi User API modules
  * @{
  */

DEFINE_ENUM_STRINGS(THREADAPI_RESULT, THREADAPI_RESULT_VALUES);

static RTOSThreadHandle                 listTaskRTOSHandle[AZURETHREADAPI_MAX_TASK_NUMBER];

static void RTOSTaskGeneric(void const *argument);

void ThreadAPI_Initialize(void)
{
  int indexThread;
  
  memset(listTaskRTOSHandle,0,sizeof(listTaskRTOSHandle));
  
  for(indexThread=0;indexThread<AZURETHREADAPI_MAX_TASK_NUMBER;indexThread++)
  {
    sprintf(listTaskRTOSHandle[indexThread].nameTask,"T%d",indexThread);
    listTaskRTOSHandle[indexThread].threadDef.name              = listTaskRTOSHandle[indexThread].nameTask;         ///< Thread name 
    listTaskRTOSHandle[indexThread].threadDef.pthread           = (os_pthread)RTOSTaskGeneric;                      ///< start address of thread function
    listTaskRTOSHandle[indexThread].threadDef.tpriority         = osPriorityNormal;                                 ///< initial thread priority
    listTaskRTOSHandle[indexThread].threadDef.instances         = 0;                                                ///< maximum number of instances of that thread function
    listTaskRTOSHandle[indexThread].threadDef.stacksize         = AZURETHREADAPI_STACKSINGLETASK;                   ///< stack size requirements in bytes; 0 is default stack size
    
    listTaskRTOSHandle[indexThread].taskIndicator               = indexThread;
    listTaskRTOSHandle[indexThread].taskArgsPointer             = NULL;
    listTaskRTOSHandle[indexThread].isThreadActivated           = THREAD_STATUS_NONE;
    
    listTaskRTOSHandle[indexThread].threadId = osThreadCreate(&listTaskRTOSHandle[indexThread].threadDef,&listTaskRTOSHandle[indexThread].taskIndicator);
  }
}


THREADAPI_RESULT ThreadAPI_Create(THREAD_HANDLE* threadHandle, THREAD_START_FUNC func, void* arg)
{
  int indexThread;
  
  for(indexThread=0;indexThread<AZURETHREADAPI_MAX_TASK_NUMBER;indexThread++)
  {
    if(listTaskRTOSHandle[indexThread].isThreadActivated==0)
    {			
      listTaskRTOSHandle[indexThread].taskFunctionPointer       = func;
      listTaskRTOSHandle[indexThread].taskArgsPointer           = arg;
      (*threadHandle)                                           = &listTaskRTOSHandle[indexThread].taskIndicator;
      listTaskRTOSHandle[indexThread].isThreadActivated         = THREAD_STATUS_OK;
      
      return THREADAPI_OK;
    }
  }
  return THREADAPI_ERROR;
}

THREADAPI_RESULT ThreadAPI_Join(THREAD_HANDLE threadHandle, int *res)
{
    if (threadHandle == NULL)
    {
        LogError("(result = %s)\r\n", ENUM_TO_STRING(THREADAPI_RESULT, THREADAPI_INVALID_ARG));
        
        return THREADAPI_INVALID_ARG;
    }
    else
    {   
      (*res) = *((int*)threadHandle);
      while(osThreadGetState(listTaskRTOSHandle[(*res)].threadId)==osThreadRunning)//
      {
        osDelay(10);
      }
    }

    return THREADAPI_OK;
}

void ThreadAPI_Exit(int res)
{
  osThreadTerminate(listTaskRTOSHandle[(res)].threadId);
}

/** FIXME: FUNZIONE ESTERNA AL FRAMEWORK
*/
void ThreadAPI_Release(THREAD_HANDLE threadHandle)
{
  int index = *((int*)threadHandle);
  
  if(index>=AZURETHREADAPI_MAX_TASK_NUMBER)
  {
    return;
  }
}

void ThreadAPI_Sleep(unsigned int milliseconds)
{
  osDelay(milliseconds);
}


void RTOSTaskGeneric(void const *pointer)
{
  int indexThread;
  
  if(pointer==0)
    return;
  
  indexThread = *(int*)pointer;
  
  if(indexThread>=AZURETHREADAPI_MAX_TASK_NUMBER)
    return;
  
  while(1)
  {  
    if(listTaskRTOSHandle[indexThread].isThreadActivated==0)
    {
      osDelay(100);
    }
    else
    {
      if(listTaskRTOSHandle[indexThread].taskFunctionPointer!=0)
      {
        listTaskRTOSHandle[indexThread].taskFunctionPointer((void*)listTaskRTOSHandle[indexThread].taskArgsPointer);
        listTaskRTOSHandle[indexThread].taskFunctionPointer     = 0;
        listTaskRTOSHandle[indexThread].isThreadActivated       = THREAD_STATUS_NONE;
      }
    }
  }
}

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

