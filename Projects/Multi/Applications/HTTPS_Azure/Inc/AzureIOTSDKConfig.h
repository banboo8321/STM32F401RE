 /**
  ******************************************************************************
  * @file    AzureIoTSDKConfig.h
  * @author  Central LAB
  * @version V1.1.0
  * @date    08-August-2016
  * @brief   Configuration file
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

#ifndef __AZURE_IOT_SDK_CONFIG_H
#define __AZURE_IOT_SDK_CONFIG_H

#define AZUREIOTSDK_STM32NUCLEOF4PORTING_VERSION        "02.00.00.00"

/* RTOS related define */
#define AZURERUNEXAMPLE_STACKSIZE                       2048
#define AZURETHREADAPI_MAX_TASK_NUMBER                  4
#define AZURETHREADAPI_TIMEOUT_WAIT_SINGLEEVENT	        5
#define AZURETHREADAPI_STACKSINGLETASK                  256
#define RTOS_SIZE_STACKMINIMUM       	4
#define RTOS_MINIMUM_MARGIN          	1024

/* Network relate defines */
#define AZUREHTTP_MAX_NUMBER_SOCKET                     1               //N.B. FOR TLS IT IS POSSIBLE TO MANAGE JUST 1 SOCKET
#define AZUREHTTP_MAXNUMBEREVENTSOCKETREAD              2               
#define AZURESOCKET_NUMBERBYTES_SINGLESOCKETRECEIVE     2048
#define AZURESOCKET_MAXTIMEOUTWAITBYTE_MS               15000            // 13 sec.
#define TCP_HTTPSPORT                    443
#define TCP_SERVERPORT                   443       
#define ENABLE_INTERNETADAPTER_PATCH     0
#define ENABLE_SYNCAGENT_DEBUG           0



/* Application scenarios :       */
/*      - AZURE_HTTP_CLIENT_SAMPLE : HTTP client application provided by microsoft              */
/*      - SECURE_S2C : HTTP client + data visualization on web page on proprietary Account      */
/*                                                                                              */
#define AZURE_HTTP_CLIENT_SAMPLE 0
#define AZURE_ENDLESS_LOOP 1
#define AZURE_ENDLESS_LOOP_BINARY 2
#define AZURE_DASHBOARD_PORTAL               "stm32ode.azurewebsites.net/Home/Index/"

#define APPLICATION_SCENARIO AZURE_ENDLESS_LOOP
//#define EMBEDDED_WIFI_PARAMETERS 0
#define EMBEDDED_WIFI_PARAMETERS 1/*add	by jiangdong at 2/13*/
/* STM32 */
//The Connection String  need to be added
/*add	by jiangdong at 2/10*/
//#define AZUREIOTHUBCONNECTIONSTRING             NULL
/*add	by jiangdong at 2/13*/
//#define AZUREIOTHUBCONNECTIONSTRING         "HostName=OtisConnectivityIoTHub.azure-devices.cn;SharedAccessKeyName=iothubowner;SharedAccessKey=iSxen255hPj5aRokWhD+cUCnurDl7wlS0De5VPvjoCI="
//xuehaiyan
//#define AZUREIOTHUBCONNECTIONSTRING             "HostName=OtisConnectivityIoTHub.azure-devices.cn;DeviceId=STM32F4;SharedAccessKey=9D7d6KxmvmYtxoZJzWoXkJ6MMV5Vy61nzC2ChliIXyc="
//guijindong
#define AZUREIOTHUBCONNECTIONSTRING             "HostName=STM32F401JHF.azure-devices.cn;DeviceId=STM32F401JHF;SharedAccessKey=mD8Z2S3KPuBglQJd83tGpwZVGWgTMxyTlIDX6NM6sWo="
//hostname  OtisTest.azure-devices.cn   42.159.230.43

#if 1//EMBEDDED_WIFI_PARAMETERS
#if 1/*add	by jiangdong at 2/10*/
    #define X_CLOUD_SSID "Techwork"
    #define X_CLOUD_KEY "20509288"
    #define X_CLOUD_AUTH_METHOD "WPA2"
#else
	#if 1
	#define X_CLOUD_SSID "Michael iPhone"
	#define X_CLOUD_KEY "123456789jd"
	#define X_CLOUD_AUTH_METHOD "WPA2"
#else
	#define X_CLOUD_SSID "Netcore"
    #define X_CLOUD_KEY "15000084341"
    #define X_CLOUD_AUTH_METHOD "WPA2"
	#endif
#endif
#else
    #define X_CLOUD_SSID "MERCURY_F4A8"
    #define X_CLOUD_KEY ""
    #define X_CLOUD_AUTH_METHOD "NONE"
#endif

#endif


/******************* (C) COPYRIGHT 2013 STMicroelectronics *****END OF FILE****/
