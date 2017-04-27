 /**
  ******************************************************************************
  * @file    SynchronizationAgent.h
  * @author  Central LAB
  * @version V1.1.0
  * @date    08-August-2016
  * @brief   Header file for SynchronizationAgent.c 
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

#ifndef __SYNCHRONIZATION_AGENT_H
#define __SYNCHRONIZATION_AGENT_H

#include <stdint.h>

//#define SYNCHRONIZATIONAGENT_DEFAULT_ENDPOINT_IPADDRESS                "0.pool.ntp.org"
//#define SYNCHRONIZATIONAGENT_DEFAULT_ENDPOINT_IPADDRESS_BACKUP         "1.pool.ntp.org"

#define SYNCHRONIZATIONAGENT_DEFAULT_ENDPOINT_IPADDRESS_TEST                "time.nist.gov"
#define SYNCHRONIZATIONAGENT_DEFAULT_ENDPOINT_IPADDRESS_TEST_IP                "216.229.0.179"
#define SYNCHRONIZATIONAGENT_DEFAULT_ENDPOINT_IPADDRESS                "time-d.nist.gov"
#define SYNCHRONIZATIONAGENT_DEFAULT_ENDPOINT_IPADDRESS_BACKUP         "time-c.nist.gov"
#define SYNCHRONIZATIONAGENT_DEFAULT_ENDPOINT_TCPPORT           37
#define SYNCHRONIZATIONAGENT_DEFAULT_NTP_RETRY                 2


typedef enum
{ 
  NTP_SUCCESS           = 0,
  NTP_ERROR             = 1,
  NTP_TIMEOUT           = 2
} NTP_Status_t;


//void SynchronizationAgentStart(const char* ipAddress,short tcpPort);
int SynchronizationNTPStart(const char* ipAddress,short tcpPort);


#endif

/******************* (C) COPYRIGHT 2013 STMicroelectronics *****END OF FILE****/
