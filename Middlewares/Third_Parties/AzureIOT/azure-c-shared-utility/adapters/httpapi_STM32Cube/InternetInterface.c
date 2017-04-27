/**
  ******************************************************************************
  * @file    InternetInterface.c
  * @author  Central LAB
  * @version V1.1.0
  * @date    08-August-2016
  * @brief   Wrapper to socket mapping
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


#include "InternetInterface.h"
#include "InternetInterfaceDriver.h"
#include "SocketMapping.h"
#include "InternetAdapter.h"

TInternetInterfaceResult   InternetInterfaceInitialize(const TInternetAdapterSettings* pSettings)
{
  if(InternetAdapterInitialize(pSettings)==INTERNETINTERFACERESULT_SUCCESS)
  {
    SocketMappingInitialize();

    return INTERNETINTERFACERESULT_SUCCESS;
  }
  
  return INTERNETINTERFACERESULT_FAILED;
}

TSocketStatus InternetInterfaceSocketGetConnectionStatus(SOCKETSYSTEMHANDLE pHandle)
{
  return SocketMappingGetStatus(pHandle);
}


TInternetInterfaceResult        InternetInterfaceStart(void)
{
  return InternetAdapterStart(CALLINGPROCEDURETYPE_OSTHREAD);
}

TInternetInterfaceResult        InternetInterfaceStop(void)
{
  return InternetAdapterStop();
}

TSocketStatus  InternetInterfaceSocketOpen(SOCKETSYSTEMHANDLE pHandle,const char* ipAddress,short tcpPort,TSocketConnectionType socketConnectionType)
{
  if(SocketMappingCheckAvailability()==SOCKETMAPPINGRESULT_OK)
 
  {
    if(SocketMappingOpenConnection(pHandle,ipAddress, tcpPort,socketConnectionType)==SOCKETSTATUS_CONNECTED)
    {
      return SOCKETSTATUS_CONNECTED;
    }
  }
  return SOCKETSTATUS_DISCONNECTED;
}

TSocketOperationResult InternetInterfaceSocketSend(SOCKETSYSTEMHANDLE pHandle,void* pData,size_t sizeData)
{
  if(SocketMappingGetStatus(pHandle)==SOCKETSTATUS_CONNECTED)
  {
    if(SocketMappingWriteData(pHandle,pData,sizeData)==SOCKETOPERATIONRESULT_SUCCESS)
    {
      return SOCKETOPERATIONRESULT_SUCCESS;
    }
  }
  
  return SOCKETOPERATIONRESULT_FAILED;
}

TSocketStatus  InternetInterfaceSocketClose(SOCKETSYSTEMHANDLE pHandle)
{
  return SocketMappingCloseConnection(pHandle);
}


TSocketOperationResult  InternetInterfaceSocketReceive(SOCKETSYSTEMHANDLE pHandle,void* pData,size_t maxSizeData,size_t *pSizeRead)
{
  return SocketMappingReadData(pHandle,(uint8_t*)pData,maxSizeData,pSizeRead);
}

TInternetInterfaceResult   InternetInterfaceLoadCertificate(const unsigned char* certificate, size_t size)
{
  //N.B.: IT IS POSSIBLE TO MANAGE CERTIFICATE FOR ONE-WAY MUTUAL CERTIFICATION ON TLS
  return INTERNETINTERFACERESULT_SUCCESS;
}


/******************* (C) COPYRIGHT 2013 STMicroelectronics *****END OF FILE****/
