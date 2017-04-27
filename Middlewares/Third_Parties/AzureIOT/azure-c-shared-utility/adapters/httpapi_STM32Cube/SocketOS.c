 /**
  ******************************************************************************
  * @file    SocketOS.c
  * @author  Central LAB
  * @version V1.1.0
  * @date    08-August-2016
  * @brief   Azure IoT SDK interface with Wi-Fi
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
  * 
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */


#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "azure_c_shared_utility/threadapi.h"

#include <stdio.h>
#include <ctype.h>
#include "string.h"
#include "SocketOS.h"
#include "AzureIOTSDKConfig.h"
#include "FreeRTOS.h"
#include "queue.h"

#define SIZE_BUFFER_DATA 768

typedef struct _TBufferData_t
{
  uint8_t       pBuffer[SIZE_BUFFER_DATA];
  size_t        sizeData;
  SOCKETHANDLE  socketID;
  THREAD_HANDLE handleThread;
}TBufferData;

static TBufferData      listBufferData[AZUREHTTP_MAXNUMBEREVENTSOCKETREAD];
static xQueueHandle     listQueueSocketDataReceived[AZUREHTTP_MAX_NUMBER_SOCKET];
static SOCKETHANDLE     listSocketHandle[AZUREHTTP_MAX_NUMBER_SOCKET] = {0};//Linked List Socket to the listQueueSocketDataReceived
static int              indexBufferData = 0;

static TSocketMappingResult     SocketOSPushDataRead(SOCKETHANDLE socketID,uint8_t* pBuffer,size_t sizeData);
static TBufferData*             BufferDataCreate(SOCKETHANDLE socketId,uint8_t * data_ptr, uint32_t message_size);
static void                     BufferDataRelease(TBufferData* ptr);

void SocketOSInitialize(void)
{
  size_t indexSocketHandle;
  
  for(indexSocketHandle=0;indexSocketHandle<AZUREHTTP_MAX_NUMBER_SOCKET;indexSocketHandle++)
  {
    listSocketHandle[indexSocketHandle]                 = INVALID_SOCKET_HANDLE;
    listQueueSocketDataReceived[indexSocketHandle]      = xQueueCreate(AZURESOCKET_NUMBERBYTES_SINGLESOCKETRECEIVE,sizeof(uint8_t));
  }
}

void SocketOSAssign(SOCKETHANDLE socketHandle)
{
  size_t indexSocketHandle;
  
  for(indexSocketHandle=0;indexSocketHandle<AZUREHTTP_MAX_NUMBER_SOCKET;indexSocketHandle++)
  {
    if(listSocketHandle[indexSocketHandle] ==INVALID_SOCKET_HANDLE)
    {
      listSocketHandle[indexSocketHandle] = socketHandle;
      return;
    }
  }
}

void SocketOSRelease(SOCKETHANDLE socketHandle)
{
  size_t indexSocketHandle;
  
  for(indexSocketHandle=0;indexSocketHandle<AZUREHTTP_MAX_NUMBER_SOCKET;indexSocketHandle++)
  {
    if(listSocketHandle[indexSocketHandle] ==socketHandle)
    {
      listSocketHandle[indexSocketHandle] = INVALID_SOCKET_HANDLE;
      return;
    }
  }
}

TSocketOperationResult  SocketOSReadData(SOCKETHANDLE socketHandle,uint8_t* pBuffer,size_t maxSizeData,size_t* pSizeDataRead)
{
  size_t indexSocket;
  size_t indexSizeData;
	
  for(indexSocket=0;indexSocket<AZUREHTTP_MAX_NUMBER_SOCKET;indexSocket++)
  {
    if(listSocketHandle[indexSocket]==socketHandle)//FIXME: Trick to solve bug
    {
        for(indexSizeData=0;indexSizeData<maxSizeData;indexSizeData++)
        {
          if(xQueueReceive(listQueueSocketDataReceived[indexSocket],((uint8_t*)pBuffer)+indexSizeData,AZURESOCKET_MAXTIMEOUTWAITBYTE_MS)!=pdPASS)
          {
            (*pSizeDataRead) = indexSizeData;
            return (*pSizeDataRead)>0? SOCKETOPERATIONRESULT_SUCCESS:SOCKETOPERATIONRESULT_NONE;
          }
        }
      
        (*pSizeDataRead) = indexSizeData;
        return (*pSizeDataRead)>0? SOCKETOPERATIONRESULT_SUCCESS:SOCKETOPERATIONRESULT_NONE;
    }
  }
  return SOCKETOPERATIONRESULT_FAILED;
}

TSocketMappingResult SocketOSPushDataRead(SOCKETHANDLE socketID,uint8_t* pBuffer,size_t sizeData)
{
  size_t indexSocket;
  for(indexSocket=0;indexSocket<AZUREHTTP_MAX_NUMBER_SOCKET;indexSocket++)
  {
    if(listSocketHandle[indexSocket]==socketID || socketID==SOCKETHANDLE_UNIVERSAL)//FIXME: Trick to solve bug
    {
      for(size_t indexBuffer=0;indexBuffer<sizeData;indexBuffer++)
      {
        if(xQueueSend(listQueueSocketDataReceived[indexSocket],&pBuffer[indexBuffer],0)!=pdPASS)
        {
          return SOCKETMAPPINGRESULT_FAILED;
        }
      }
      return SOCKETMAPPINGRESULT_OK;
    }
  }
  return SOCKETMAPPINGRESULT_NONE;
}

TBufferData* BufferDataCreate(SOCKETHANDLE socketId,uint8_t * data_ptr, uint32_t message_size)
{
  TBufferData* pBufferData;   
  if(indexBufferData>=AZUREHTTP_MAXNUMBEREVENTSOCKETREAD)
  {
    return 0;
  }
  
  pBufferData = listBufferData+indexBufferData;

  indexBufferData++;
  
  memset(pBufferData->pBuffer,0,message_size);
  
  memcpy(pBufferData->pBuffer,data_ptr,message_size);
  pBufferData->sizeData = message_size;
  pBufferData->socketID = socketId;
  return pBufferData;
  
}

void BufferDataRelease(TBufferData* ptr)
{
  if(indexBufferData>0)
  {    
    indexBufferData--;
  }
}

extern void ThreadAPI_Release(THREAD_HANDLE threadHandle);

int taskWiFiReceiveData(void* lpBufferData)
{
  TBufferData*  pBufferData;  
  
  if(lpBufferData!=0)
  {
    pBufferData = (TBufferData*)lpBufferData;    
    SocketOSPushDataRead(pBufferData->socketID,pBufferData->pBuffer,pBufferData->sizeData);
    
    ThreadAPI_Sleep(1);
    
    ThreadAPI_Release(pBufferData->handleThread);
    
    BufferDataRelease(pBufferData); 
  }
  
  return 0;
}

void InternetAdapterCallbackSocketReceive(SOCKETHANDLE socketHandle,uint8_t * data_ptr, uint32_t message_size, uint32_t chunck_size)
{
   TBufferData*         pBuffer;
   pBuffer = BufferDataCreate(listSocketHandle[0],data_ptr, message_size);//FIXME: REVIEW
   ThreadAPI_Create(&pBuffer->handleThread,taskWiFiReceiveData,pBuffer);
}

/******************* (C) COPYRIGHT 2013 STMicroelectronics *****END OF FILE****/
