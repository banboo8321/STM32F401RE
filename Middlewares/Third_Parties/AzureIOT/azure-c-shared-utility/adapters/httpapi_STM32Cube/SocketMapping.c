 /**
  ******************************************************************************
  * @file    SocketMapping.c
  * @author  Central LAB
  * @version V1.1.0
  * @date    08-August-2016
  * @brief   TCP/TLS socket interface
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
#include "InternetInterface.h"
#include "string.h"
#include "SocketMapping.h"
#include "AzureIOTSDKConfig.h"
#include "SocketMappingInternalTypes.h"
#include "InternetAdapter.h"
#include "SocketOS.h"

static TSocketMapping listSocketMapping[AZUREHTTP_MAX_NUMBER_SOCKET] = {{0,0}};

static TSocketMappingResult     SocketMappingCreate(SOCKETSYSTEMHANDLE pHandle,SOCKETHANDLE socketHandle,TSocketConnectionType socketConnectionType);
//static SOCKETSYSTEMHANDLE       SocketMappingGetSystemHandle(SOCKETHANDLE socketHandle);
static SOCKETHANDLE             SocketMappingGetSocketHandle(SOCKETSYSTEMHANDLE pHandle);
static TSocketConnectionType    SocketMappingGetSocketType(SOCKETSYSTEMHANDLE pHandle);
static void                     SocketMappingRelease(SOCKETSYSTEMHANDLE pHandle,SOCKETHANDLE socketHandle);


void SocketMappingInitialize(void)
{
  size_t indexSocket;

  memset(listSocketMapping,0,sizeof(listSocketMapping));
  
  for(indexSocket=0;indexSocket<AZUREHTTP_MAX_NUMBER_SOCKET;indexSocket++)
  {
    listSocketMapping[indexSocket].isSocketSet                  = SOCKETFLAG_NONE;
  }
  
  SocketOSInitialize();
}

TSocketMappingResult     SocketMappingCheckAvailability(void)
{
  size_t indexSocket;
  
  for(indexSocket=0;indexSocket<AZUREHTTP_MAX_NUMBER_SOCKET;indexSocket++)
  {
    if(listSocketMapping[indexSocket].isSocketSet!=SOCKETFLAG_SET)
    {
      return SOCKETMAPPINGRESULT_OK;
    }
  }
  
  return SOCKETMAPPINGRESULT_NONE;
}

TSocketStatus           SocketMappingOpenConnection(SOCKETSYSTEMHANDLE pHandle,const char* ipAddress,short tcpPort,TSocketConnectionType socketConnectionType)
{
  SOCKETHANDLE socketHandle;
  
  if(InternetAdapterSocketOpenConnection(&socketHandle,ipAddress,tcpPort,socketConnectionType)==SOCKETSTATUS_CONNECTED)
  {
    SocketMappingCreate(pHandle,socketHandle,socketConnectionType);
    
    return SOCKETSTATUS_CONNECTED;
  }
  
  return SOCKETSTATUS_DISCONNECTED;
}

TSocketOperationResult  SocketMappingWriteData(SOCKETSYSTEMHANDLE pHandle,uint8_t* pBuffer,size_t sizeData)
{
  SOCKETHANDLE socketMappingHandle;
  
  socketMappingHandle = SocketMappingGetSocketHandle(pHandle);
  
  if(socketMappingHandle!=INVALID_SOCKET_HANDLE)
  {
    if(InternetAdapterSocketWriteData(socketMappingHandle,pBuffer,sizeData)==SOCKETOPERATIONRESULT_SUCCESS)
    {
      return SOCKETOPERATIONRESULT_SUCCESS;
    }
  }
  
  return SOCKETOPERATIONRESULT_FAILED;
}

TSocketOperationResult  SocketMappingReadData(SOCKETSYSTEMHANDLE pHandle,uint8_t* pBuffer,size_t maxSizeData,size_t* pSizeDataRead)
{
  size_t indexSocket;

  for(indexSocket=0;indexSocket<AZUREHTTP_MAX_NUMBER_SOCKET;indexSocket++)
  {
    if(listSocketMapping[indexSocket].isSocketSet==SOCKETFLAG_SET && (listSocketMapping[indexSocket].realHandle==pHandle))//FIXME: Trick to solve bug
    {
      return InternetAdapterSocketReadData(listSocketMapping[indexSocket].mappedSocket,pBuffer,maxSizeData,pSizeDataRead);
    }
  }
  
  return SOCKETOPERATIONRESULT_FAILED;
}

TSocketStatus           SocketMappingCloseConnection(SOCKETSYSTEMHANDLE pHandle)
{
  SOCKETHANDLE          socketMapping;
  TSocketConnectionType connectionType;
  
  socketMapping         = SocketMappingGetSocketHandle(pHandle);
  connectionType        = SocketMappingGetSocketType(pHandle);
  
  if(socketMapping!=INVALID_SOCKET_HANDLE)
  {
    if(SocketMappingGetStatus(pHandle)==SOCKETSTATUS_DISCONNECTED)
    {
      return SOCKETSTATUS_DISCONNECTED;
    }
    if(InternetAdapterSocketCloseConnection(socketMapping,connectionType)==SOCKETSTATUS_DISCONNECTED)
    {
      SocketMappingRelease(pHandle,socketMapping);
      
      return SOCKETSTATUS_DISCONNECTED;
    }    
  }
  
  return SOCKETSTATUS_NONE;
}


void SocketMappingRelease(SOCKETSYSTEMHANDLE pHandle,SOCKETHANDLE socketHandle)
{
  size_t indexSocket;  
  for(indexSocket=0;indexSocket<AZUREHTTP_MAX_NUMBER_SOCKET;indexSocket++)
  {
    if(listSocketMapping[indexSocket].isSocketSet==SOCKETFLAG_SET && listSocketMapping[indexSocket].realHandle==pHandle)
    {
      listSocketMapping[indexSocket].isSocketSet = SOCKETFLAG_NONE;
      
      SocketOSRelease(socketHandle);
      
      return;
    }
  }
}

TSocketStatus SocketMappingGetStatus(SOCKETSYSTEMHANDLE pHandle)
{
  size_t indexSocket;  
  for(indexSocket=0;indexSocket<AZUREHTTP_MAX_NUMBER_SOCKET;indexSocket++)
  {
    if(listSocketMapping[indexSocket].isSocketSet==SOCKETFLAG_SET && listSocketMapping[indexSocket].realHandle==pHandle)
    {
      return listSocketMapping[indexSocket].socketStatus;
    }
  }

  return SOCKETSTATUS_NONE;
}

TSocketStatus SocketMappingSetConnectionStatusRealHandle(SOCKETSYSTEMHANDLE pHandle,TSocketStatus socketStatus)
{
  size_t indexSocket;  
  for(indexSocket=0;indexSocket<AZUREHTTP_MAX_NUMBER_SOCKET;indexSocket++)
  {
    if(listSocketMapping[indexSocket].isSocketSet==SOCKETFLAG_SET && listSocketMapping[indexSocket].realHandle==pHandle)
    {
      listSocketMapping[indexSocket].socketStatus = socketStatus;      
    }
  }

  return SOCKETSTATUS_NONE;
}

TSocketStatus SocketMappingCallbackSetConnectionStatus(SOCKETHANDLE socketHandle,TSocketStatus socketStatus)
{
  size_t indexSocket;  
  for(indexSocket=0;indexSocket<AZUREHTTP_MAX_NUMBER_SOCKET;indexSocket++)
  {
    if(listSocketMapping[indexSocket].isSocketSet==SOCKETFLAG_SET && listSocketMapping[indexSocket].mappedSocket==socketHandle)
    {
      listSocketMapping[indexSocket].socketStatus       = socketStatus; 
      listSocketMapping[indexSocket].isSocketSet        = SOCKETFLAG_NONE;//FIXME: CHECK IT!!
    }
  }

  return SOCKETSTATUS_NONE;
}

TSocketMappingResult     SocketMappingCreate(SOCKETSYSTEMHANDLE pHandle,SOCKETHANDLE socketHandle,TSocketConnectionType socketConnectionType)
{
  size_t indexSocket;
  
  for(indexSocket=0;indexSocket<AZUREHTTP_MAX_NUMBER_SOCKET;indexSocket++)
  {
    if(listSocketMapping[indexSocket].isSocketSet!=SOCKETFLAG_SET)
    {
      listSocketMapping[indexSocket].realHandle                 = pHandle;
      listSocketMapping[indexSocket].mappedSocket               = socketHandle;
      listSocketMapping[indexSocket].socketConnectionType       = socketConnectionType;
      listSocketMapping[indexSocket].isSocketSet                = SOCKETFLAG_SET;
      listSocketMapping[indexSocket].socketStatus               = SOCKETSTATUS_CONNECTED;
      
      SocketOSAssign(socketHandle);
      
      return SOCKETMAPPINGRESULT_OK;
    }
  }
  
  return SOCKETMAPPINGRESULT_NONE;
}

#if 0
SOCKETSYSTEMHANDLE  SocketMappingGetSystemHandle(SOCKETHANDLE socketHandle)
{
  size_t indexSocket;
  
  for(indexSocket=0;indexSocket<AZUREHTTP_MAX_NUMBER_SOCKET;indexSocket++)
  {
    if(listSocketMapping[indexSocket].isSocketSet==SOCKETFLAG_SET && listSocketMapping[indexSocket].mappedSocket==socketHandle)
    {
      return listSocketMapping[indexSocket].realHandle;
    }
  }
  
  return NULL;
}
#endif

SOCKETHANDLE SocketMappingGetSocketHandle(SOCKETSYSTEMHANDLE pHandle)
{
  size_t indexSocket;
  
  for(indexSocket=0;indexSocket<AZUREHTTP_MAX_NUMBER_SOCKET;indexSocket++)
  {
    if(listSocketMapping[indexSocket].isSocketSet==SOCKETFLAG_SET && listSocketMapping[indexSocket].realHandle==pHandle)
    {
      return listSocketMapping[indexSocket].mappedSocket;
    }
  }
  
  return INVALID_SOCKET_HANDLE;
}

TSocketConnectionType SocketMappingGetSocketType(SOCKETSYSTEMHANDLE pHandle)
{
  size_t indexSocket;
  
  for(indexSocket=0;indexSocket<AZUREHTTP_MAX_NUMBER_SOCKET;indexSocket++)
  {
    if(listSocketMapping[indexSocket].isSocketSet==SOCKETFLAG_SET && listSocketMapping[indexSocket].realHandle==pHandle)
    {
      return listSocketMapping[indexSocket].socketConnectionType;
    }
  }
  
  return SOCKETCONNECTIONTYPE_NONE;
}



/******************* (C) COPYRIGHT 2013 STMicroelectronics *****END OF FILE****/
