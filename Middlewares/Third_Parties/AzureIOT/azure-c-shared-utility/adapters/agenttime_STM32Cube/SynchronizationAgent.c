 /**
  ******************************************************************************
  * @file    SynchronizationAgent.c
  * @author  Central LAB
  * @version V1.1.0
  * @date    08-August-2016
  * @brief   Functions to retrieve data from NTP server 
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
#include "azure_c_shared_utility/agenttime.h"

#include "InternetInterface.h"
#include "SynchronizationAgent.h"
#include "timingSystem.h"
#include "AzureIOTSDKConfig.h"
#include "GPRS_module.h"
#include "wifi_module.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


/** @addtogroup MIDDLEWARES
* @{
*/ 


/** @addtogroup AZURE_SDK
* @{
*/ 

/** @defgroup  AZURE_ADAPTER_NTP
  * @brief Interface between Microsoft Azure IoT SDK and Wi-Fi board
  * @{
  */


#define SIZE_BUFFER_ANSWERTIME  4
#define CONVERSION_EPOCHFACTOR  2208988800ul

typedef struct _TTCPConnectionCredential_t
{
  char* pIPAddress;
  short tcpPort;
}TTCPConnectionCredential;

static TTCPConnectionCredential         tcpConnectionCredential;
static int                              InitNTPSync(void * pointerArg);
static TTCPConnectionCredential*        CreateTCPConnectionCredential(const char* ipAddress,short tcpPort);
static void                             ReleaseTCPConnectionCredential(TTCPConnectionCredential* pTCPConnectionCredential);

/**
 * @brief  Convert NTP time to epoch time
* @param  uint8_t* pBufferTimingAnswer : pointer to buffere containing the NTP date
* @param  size_t sizeBuffer : size of buffer
* @retval time_t : epoch time after conversion
 */
time_t SynchronizationAgentConvertNTPTime2EpochTime(uint8_t* pBufferTimingAnswer,size_t sizeBuffer)
{
  uint32_t      valueNumericExtracted;
  time_t        epochTime;

  epochTime = (time_t)-1;
  
  if(sizeBuffer>=SIZE_BUFFER_ANSWERTIME)
  {
    valueNumericExtracted       = ((pBufferTimingAnswer[0]<<24 )|(pBufferTimingAnswer[1]<<16)|(pBufferTimingAnswer[2]<<8)| pBufferTimingAnswer[3]);
    epochTime                   = (time_t)(valueNumericExtracted-CONVERSION_EPOCHFACTOR);
  }
  
  return epochTime;
}


/**
 * @brief  Connect with NTP server 
* @param  uint8_t* ipAddress : IP address for NTP server
* @param  short tcpPort : TCP port number
* @retval void
 */
//void SynchronizationAgentStart(const char* ipAddress,short tcpPort)
int SynchronizationNTPStart(const char* ipAddress,short tcpPort)
{
  TTCPConnectionCredential* pTCPConnectionCredential;
  
  pTCPConnectionCredential = CreateTCPConnectionCredential(ipAddress,tcpPort);
  
  return InitNTPSync(pTCPConnectionCredential);
}




/**
 * @brief  Function managing connection with NTP server  
* @param  void * pointerArg : arguments for thread
* @param  short tcpPort : TCP port number
* @retval int value for success(1)/failure(0)
 */
int InitNTPSync(void * pointerArg)
{
  size_t        sizeEffectiveRead;
  time_t        epochTimeToSetForSystem;
  uint8_t       bufferTimingAnswer[SIZE_BUFFER_ANSWERTIME];
  NTP_Status_t  return_status = NTP_ERROR;
  short unsigned int i_err_code;


  TTCPConnectionCredential* pTCPConnectionCredential;
  
  pTCPConnectionCredential = (TTCPConnectionCredential*)pointerArg;
 
  if(pointerArg==0 || pTCPConnectionCredential->pIPAddress==0)
  { 
    printf("[IotHub]. Error in InitNTPSync. Code line %d \r\n", __LINE__);
    return return_status;
  }  
  
  memset(bufferTimingAnswer,0,sizeof(bufferTimingAnswer));
  epochTimeToSetForSystem = 0;
 
  
  if(InternetInterfaceStart()==INTERNETINTERFACERESULT_SUCCESS)
  {
    if(InternetInterfaceSocketOpen(NULL,pTCPConnectionCredential->pIPAddress, pTCPConnectionCredential->tcpPort,SOCKETCONNECTIONTYPE_TCPSTANDARD)==SOCKETSTATUS_CONNECTED)
    {
        printf("[IotHub]. Connected with NTP Server: %s\r\n",pTCPConnectionCredential->pIPAddress);
     
        if( (i_err_code = InternetInterfaceSocketReceive(NULL,bufferTimingAnswer,SIZE_BUFFER_ANSWERTIME,&sizeEffectiveRead))==SOCKETOPERATIONRESULT_SUCCESS && sizeEffectiveRead>=SIZE_BUFFER_ANSWERTIME) {
                epochTimeToSetForSystem = SynchronizationAgentConvertNTPTime2EpochTime(bufferTimingAnswer,SIZE_BUFFER_ANSWERTIME);
#if TEST_Program
	//print("[Test_P]:Receive NTP¡¡data is %d \r\n",);
#endif
                if (TimingSystemSetSystemTime(epochTimeToSetForSystem)== 0){
                          printf("[IotHub][E]. Failed to set system time. \r\n");
                          ReleaseTCPConnectionCredential(pTCPConnectionCredential);
                } else {
                        return_status = NTP_SUCCESS;
                        printf("[IotHub]. Set UTC Time: %s\r\n",(get_ctime(&epochTimeToSetForSystem)));
                }         
        }
        else {
          printf("[IotHub][E]. Failed to connect with NTP Server: %s\r\n",pTCPConnectionCredential->pIPAddress);
          printf("[IotHub][E].Error code:%d . Bytes read:%d \r\n", i_err_code, sizeEffectiveRead);
        }
#if 0//GPRS_INSTALL

#else
        InternetInterfaceSocketClose(NULL);

#endif
    }
    else
        printf("[IotHub][E]. Failed to connected with NTP Server: %s\r\n",pTCPConnectionCredential->pIPAddress);
      
  }
  
  ReleaseTCPConnectionCredential(pTCPConnectionCredential);  
  
  return return_status;
}


TTCPConnectionCredential* CreateTCPConnectionCredential(const char* ipAddress,short tcpPort)
{
  memset(&tcpConnectionCredential,0,sizeof(TTCPConnectionCredential));
  
  tcpConnectionCredential.pIPAddress = (char*)malloc((size_t)(strlen(ipAddress)+1));
  memset(tcpConnectionCredential.pIPAddress,0,strlen(ipAddress)+1);
  
  strcpy(tcpConnectionCredential.pIPAddress,ipAddress);
  tcpConnectionCredential.tcpPort = tcpPort;
  
  return &tcpConnectionCredential;
}

void ReleaseTCPConnectionCredential(TTCPConnectionCredential* pTCPConnectionCredential)
{
  free(tcpConnectionCredential.pIPAddress);
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
