 /**
  ******************************************************************************
  * @file    InternetAdapter.c
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

#include "azure_c_shared_utility/threadapi.h"


#include "wifi_interface.h"
#include "wifi_module.h"
#include "InternetAdapter.h"
#include "SocketMapping.h"
#include "SocketOS.h"
#include "AzureIOTSDKConfig.h"
#include "GPRS_module.h"
/** @addtogroup MIDDLEWARES
* @{
*/ 


/** @addtogroup AZURE_SDK
* @{
*/ 


/** @addtogroup AZURE_ADAPTER_HTTP
* @{
*/ 



#include <string.h>
#include <stdlib.h>

static  wifi_config             config;
static  uint32_t                macaddstart[64];


static char*  ssid          = 0;
static char*  seckey        = 0;

static volatile short           isSSIDConnected = 0;
static volatile short           isWiFiOn = 0;

static volatile short           isWiFiStarted   = 0;
static uint8_t  tlsSocketID;
static WiFi_Priv_Mode mode;


#if ENABLE_INTERNETADAPTER_PATCH>0
#define TIMEOUT_WAIT_SOCKETCLOSE        5000
#define TIMEOUT_SINGLESLEEP_SOCKETCLOSE 100
        
typedef struct __TInternetAdapterSocketInfo_t
{
  SOCKETHANDLE  socketHandle;
  TSocketStatus socketStatus;
}TInternetAdapterSocketInfo;
#endif

#if ENABLE_INTERNETADAPTER_PATCH>0
static volatile TInternetAdapterSocketInfo listSocketInfo[AZUREHTTP_MAX_NUMBER_SOCKET];
#endif

#define TIMEOUT_WIFI_INIT               1000 // 5 seconds timeout to initialize wifi and connect to AP

static TInternetInterfaceResult InternetAdapterRealConnectionProcedure(void);

/**
  * @brief  Initialize Access Point parameters 
  * @param  const TInternetAdapterSettings* pSettings : data structure containing WiFi access point parameters
  * @retval TInternetInterfaceResult (SUCCESS/FAIL)
  */
TInternetInterfaceResult   InternetAdapterInitialize(const TInternetAdapterSettings* pSettings)
{
  ssid          = (char *)calloc(strlen(pSettings->pSSID)+1,sizeof(uint8_t));
  seckey        = (char *)calloc(strlen(pSettings->pSecureKey)+1,sizeof(uint8_t));
  tlsSocketID   = INVALID_SOCKET_HANDLE;
  isWiFiStarted = 0;
  
  if(ssid==0 || seckey==0)
  {
    return INTERNETINTERFACERESULT_FAILED;
  }

  if (strcmp(pSettings->authMethod,"NONE")==0)
    mode = None;
  else if(strcmp(pSettings->authMethod,"WEP")==0)
    mode = WEP;
  else 
    mode = WPA_Personal; // FIXME. NDEF does not read Encryption settings 
  
  memcpy(ssid,pSettings->pSSID,strlen(pSettings->pSSID));
  memcpy(seckey,pSettings->pSecureKey,strlen(pSettings->pSecureKey));
  
  #if ENABLE_INTERNETADAPTER_PATCH>0
  memset((void*)listSocketInfo,0,sizeof(listSocketInfo));
  #endif
  
  return INTERNETINTERFACERESULT_SUCCESS;
}

/**
  * @brief  Initialize Wi-Fi  
  * @param  void
  * @retval TInternetInterfaceResult (SUCCESS/FAIL)
  */
TInternetInterfaceResult InternetAdapterRealConnectionProcedure(void)
{
    config.power          = wifi_active;
    config.power_level    = high;
    config.dhcp           = on;               //use DHCP IP address
    config.web_server     = WIFI_TRUE;
    int   i_timeout_wifi  = TIMEOUT_WIFI_INIT;
    //config.wifi_baud_rate = 921600;
    //config.wifi_baud_rate = 115200;
     
    /* Init the wi-fi module */     
    printf("WiFi: WAIT FOR Module Activation Procedure...\r\n");
    
    if(wifi_init(&config)!=WiFi_MODULE_SUCCESS){
      return INTERNETINTERFACERESULT_FAILED;
    }
#if TEST_Program
	printf("[Test_P]:4 wait for isWiFiOn\r\n");
#endif
    while ((isWiFiOn == 0) && (i_timeout_wifi > 0) ) {
          HAL_Delay(10);
          i_timeout_wifi--;
    }
    
    if (i_timeout_wifi == 0){
        printf("\r\n[Init]. Failed to activate Wi-Fi module \r\n");
        return INTERNETINTERFACERESULT_FAILED;
    }else {
        printf("\r\n[Init]. Wi-Fi Module Activated\r\n");
         isWiFiOn = 0;
         i_timeout_wifi  = TIMEOUT_WIFI_INIT;
    }
#if GPRS_INSTALL

#else
    GET_Configuration_Value("nv_wifi_macaddr",macaddstart);

#endif
   
    printf("[Init]. WiFi MAC Address is: %s\r\n",(char*)macaddstart);
    
    printf("[Init]. Try to Connect to SSID: %s...\r\n",ssid);
    
    if(wifi_connect(ssid, seckey, mode)==WiFi_MODULE_SUCCESS)
    {
#if GPRS_INSTALL
    	virtual_WIND_24_CMD_WiFiUp();
#else


#endif


       while ((isSSIDConnected == 0) && (i_timeout_wifi > 0) ) {
          HAL_Delay(10);
          i_timeout_wifi--;
       }

        if (i_timeout_wifi == 0){
            printf("\r\n[Init]. Failed to connect to Access Point \r\n");
            return INTERNETINTERFACERESULT_FAILED;
        }else {
             printf("[Init]. WiFi connected to AccessPoint \r\n");
             return INTERNETINTERFACERESULT_SUCCESS;
        }
    }
    
    printf("[Init][E]. WiFi Failed to connect to Access Point\r\n");
    
    return INTERNETINTERFACERESULT_FAILED;
}

TInternetInterfaceResult InternetAdapterStop(void)
{
  isWiFiStarted         = 0;
  isSSIDConnected       = 0;
  isWiFiOn = 0;
  
  return INTERNETINTERFACERESULT_SUCCESS;
}

/**
  * @brief  Activate thread to retrieve NTP date when calling procedures flag is set to CALLINGPROCEDURETYPE_OSTHREAD
  * @param  TCallingProcedureType callingProcedureType (CALLINGPROCEDURETYPE_MAIN/CALLINGPROCEDURETYPE_OSTHREAD)
  * @retval TInternetInterfaceResult (SUCCESS/FAIL)
  */
TInternetInterfaceResult        InternetAdapterStart(TCallingProcedureType callingProcedureType)
{
  if(isWiFiStarted==0)
  {
    if(InternetAdapterRealConnectionProcedure()==INTERNETINTERFACERESULT_SUCCESS)
    {
#if TEST_Program
	printf("[Test_P]:3 wait for isSSIDConnected\r\n");
#endif

      while(isSSIDConnected==0)
      {
        switch(callingProcedureType)
        {
          case CALLINGPROCEDURETYPE_MAIN:
            HAL_Delay(10);
          break;
          
          case CALLINGPROCEDURETYPE_OSTHREAD:
            ThreadAPI_Sleep(10);
          break;
        }
      }
      
      //printf("WiFi: Connection Confirmed to Network\r\n");
      
      isWiFiStarted = 1;
    }
  }
  
  return isWiFiStarted==1?INTERNETINTERFACERESULT_SUCCESS:INTERNETINTERFACERESULT_FAILED;
}


/**
  * @brief  Open TCP/TLS connection
* @param  SOCKETHANDLE* pSocketHandle : handler for TCP/TLS connection
* @param  const char* ipAddress : IP address 
* @param  short tcpPort : TCP/TLS port number
* @param  TSocketConnectionType socketConnectionType : TCP/TLS
  * @retval TSocketStatus (SOCKETSTATUS_CONNECTED/SOCKETSTATUS_DISCONNECTED)
  */
TSocketStatus   InternetAdapterSocketOpenConnection(SOCKETHANDLE* pSocketHandle,const char* ipAddress,short tcpPort,TSocketConnectionType socketConnectionType)
{
#if ENABLE_INTERNETADAPTER_PATCH>0
  uint8_t indexSocket;
#endif
  if(wifi_socket_client_open((uint8_t*)ipAddress, tcpPort, socketConnectionType==SOCKETCONNECTIONTYPE_TLS? (uint8_t*)"s":(uint8_t*)"t", pSocketHandle)==WiFi_MODULE_SUCCESS)
  {
#if 0    
    if(socketConnectionType==SOCKETCONNECTIONTYPE_TLS)
    {
      tlsSocketID = *pSocketHandle;
    }
#endif
    
    #if ENABLE_INTERNETADAPTER_PATCH>0
    for(indexSocket=0;indexSocket<AZUREHTTP_MAX_NUMBER_SOCKET;indexSocket++)
    {
      if(listSocketInfo[indexSocket].socketHandle==0)
      {
        listSocketInfo[indexSocket].socketHandle = (*pSocketHandle);
        listSocketInfo[indexSocket].socketStatus = SOCKETSTATUS_CONNECTED;
        
        break;
      }
    }
    #endif
    
    return SOCKETSTATUS_CONNECTED;
  }
  
  return SOCKETSTATUS_DISCONNECTED;
}


/**
  * @brief  Read data from socket 
* @param  SOCKETHANDLE* pSocketHandle : handler for TCP/TLS connection
* @param uint8_t* pBuffer : buffer containing received data
* @param size_t maxSizeData : maximum size for buffer 
* @param size_t* pSizeDataRead : number of bytes read
  * @retval TSocketOperationResult (SUCCESS/FAILED)
  */
TSocketOperationResult  InternetAdapterSocketReadData(SOCKETHANDLE socketHandle,uint8_t* pBuffer,size_t maxSizeData,size_t* pSizeDataRead)
{  
  
  return SocketOSReadData(socketHandle,pBuffer,maxSizeData,pSizeDataRead);
}

/**
  * @brief  Write data to socket 
* @param  SOCKETHANDLE* pSocketHandle : handler for TCP/TLS connection
* @param uint8_t* pBuffer : buffer containing data to be written to socket 
* @param size_t sizeData : number of bytes t owrite
  * @retval TSocketStatus (SUCCESS/FAILED)
  */
TSocketOperationResult  InternetAdapterSocketWriteData(SOCKETHANDLE socketHandle,uint8_t* pBuffer,size_t sizeData)
{
  
  if(wifi_socket_client_write(socketHandle,(uint16_t)sizeData,(char*)pBuffer)==WiFi_MODULE_SUCCESS)
  {
    return SOCKETOPERATIONRESULT_SUCCESS;
  }
  
  return SOCKETOPERATIONRESULT_FAILED;
}


/**
  * @brief  Close TCP/TLS connection
* @param  SOCKETHANDLE* pSocketHandle : handler for TCP/TLS connection
* @param  TSocketConnectionType socketConnectionType : TCP/TLS
  * @retval TSocketStatus (SOCKETSTATUS_CONNECTED/SOCKETSTATUS_DISCONNECTED)
  */
TSocketStatus   InternetAdapterSocketCloseConnection(SOCKETHANDLE socketHandle,TSocketConnectionType socketConnectionType)
{
#if ENABLE_INTERNETADAPTER_PATCH>0  
  uint8_t indexSocket=0;
  uint8_t timerWaitClose;
   
  if(socketConnectionType==SOCKETCONNECTIONTYPE_TLS)
  {
    if(wifi_socket_client_close(socketHandle)==WiFi_MODULE_SUCCESS)
    {
      return SOCKETSTATUS_DISCONNECTED;
    }
  }
  else
  {     
    timerWaitClose = 0;
     
    for(indexSocket=0;indexSocket<AZUREHTTP_MAX_NUMBER_SOCKET;indexSocket++)
    {
        if(listSocketInfo[indexSocket].socketHandle ==socketHandle)
        {
          while(listSocketInfo[indexSocket].socketStatus==SOCKETSTATUS_DISCONNECTED && timerWaitClose<(TIMEOUT_WAIT_SOCKETCLOSE/TIMEOUT_SINGLESLEEP_SOCKETCLOSE))
          {
            ThreadAPI_Sleep(TIMEOUT_SINGLESLEEP_SOCKETCLOSE);
          }
        }
    }   
    return SOCKETSTATUS_DISCONNECTED;
  }
  
  listSocketInfo[indexSocket].socketStatus = SOCKETSTATUS_DISCONNECTED;
  listSocketInfo[indexSocket].socketHandle = 0;

  #else
    if(wifi_socket_client_close(socketHandle)==WiFi_MODULE_SUCCESS)
    {
      return SOCKETSTATUS_DISCONNECTED;
    }  
  #endif
  
  return SOCKETSTATUS_DISCONNECTED;
}

void ind_wifi_on()
{
  //printf("\r\n WiFi on \r\n");
  isWiFiOn = 1; 
}

void ind_wifi_connected()
{
  //printf("\r\n WiFi connected \r\n");
  isSSIDConnected = 1;
}

void ind_wifi_resuming()
{

}


void ind_wifi_error(WiFi_Status_t error_code)
{
   printf("\r\n WiFi error \r\n");
}

void ind_wifi_connection_error(WiFi_Status_t status_code)
{
   printf("\r\n WiFi conn error \r\n");
}

	
void ind_wifi_ap_ready(void)
{
     printf("\r\n WiFi AP ready \r\n");    
}

void ind_wifi_ap_client_joined(uint8_t * client_mac_address)
{
       printf("\r\n WiFi AP joined \r\n");
}




/**
  * @brief  WiFi callback for data received
* @param  SOCKETHANDLE* pSocketHandle : handler for TCP/TLS connection
* @param  uint8_t * data_ptr : pointer to buffer containing data received 
* @param  uint32_t message_size : number of bytes in message received
* @param  uint32_t chunck_size : numeber of bytes in chunk
  * @retval void
  */
void ind_wifi_socket_data_received(SOCKETHANDLE pSocketHandle,uint8_t * data_ptr, uint32_t message_size, uint32_t chunck_size)
{
   // EQ. REMOVE
  //printf("****** Received buffer ***** \r\n");
  //printf(data_ptr);
  //printf("\r\n \r\n");  
  
  InternetAdapterCallbackSocketReceive( pSocketHandle, data_ptr, message_size, chunck_size);
}


/**
  * @brief  WiFi callback for remote server closure
* @param  uint8_t * socket_closed_id : pointer to socket id number
  * @retval void
  */
void ind_wifi_socket_client_remote_server_closed(uint8_t * socket_closed_id)
{
  #if ENABLE_INTERNETADAPTER_PATCH>0
  uint8_t indexSocket;
  #endif
    
  SocketMappingCallbackSetConnectionStatus((SOCKETHANDLE)(*socket_closed_id),SOCKETSTATUS_DISCONNECTED);
  
  #if ENABLE_INTERNETADAPTER_PATCH>0
  for(indexSocket=0;indexSocket<AZUREHTTP_MAX_NUMBER_SOCKET;indexSocket++)
  {
    if(listSocketInfo[indexSocket].socketHandle==(*socket_closed_id))
    {
      listSocketInfo[indexSocket].socketStatus = SOCKETSTATUS_DISCONNECTED;
      
      break;
    }
  }
  #endif
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


/******************* (C) COPYRIGHT 2013 STMicroelectronics *****END OF FILE****/
