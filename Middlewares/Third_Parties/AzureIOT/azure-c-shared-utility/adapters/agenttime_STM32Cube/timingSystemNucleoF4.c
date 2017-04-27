 /**
  ******************************************************************************
  * @file    timingSystemNucleoF4.c
  * @author  Central LAB
  * @version V1.1.0
  * @date    08-August-2016
  * @brief   Interface between Azure IoT SDK and Nucleo timing
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

#include "timingSystem.h"
#include "main.h"
#include "azure_c_shared_utility/threadapi.h"


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


typedef enum __TTimeSyncStatus_t
{
  TIMESYNCSTATUS_NONE,
  TIMESYNCSTATUS_SET,
  TIMESYNCSTATUS_NOTSET,
}TTimeSyncStatus;

/*static*/ RTC_HandleTypeDef        RtcHandle;
static volatile time_t          timeSyncSystem;
static volatile TTimeSyncStatus isTimeSynchonized;

const int yytab[2][12] = {
  { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
  { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};

/**
 * @brief  Configures the RTC
 * @param  None
 * @retval None
 */
static void RTC_Config(void)
{
    /*##-1- Configure the RTC peripheral #######################################*/
    RtcHandle.Instance = RTC;

    /* Configure RTC prescaler and RTC data registers */
    /* RTC configured as follow:
  - Hour Format    = Format 12
  - Asynch Prediv  = Value according to source clock
  - Synch Prediv   = Value according to source clock
  - OutPut         = Output Disable
  - OutPutPolarity = High Polarity
  - OutPutType     = Open Drain */
    
    RtcHandle.Instance = RTC;

    /* Configure RTC prescaler and RTC data registers */
    /* RTC configured as follow:
  - Hour Format    = Format 12
  - Asynch Prediv  = Value according to source clock
  - Synch Prediv   = Value according to source clock
  - OutPut         = Output Disable
  - OutPutPolarity = High Polarity
  - OutPutType     = Open Drain */
    RtcHandle.Init.HourFormat = RTC_HOURFORMAT_12;
    RtcHandle.Init.AsynchPrediv = RTC_ASYNCH_PREDIV;
    RtcHandle.Init.SynchPrediv = RTC_SYNCH_PREDIV;
    RtcHandle.Init.OutPut = RTC_OUTPUT_DISABLE;
    RtcHandle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    RtcHandle.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
    
        
    if(HAL_RTC_Init(&RtcHandle) != HAL_OK)
    {
        /* Initialization Error */
        while(1);
    }
}

/**
 * @brief  Initialize Nucleo timing
 * @param  None
 * @retval None
 */
void    TimingSystemInitialize(void)
{
  RTC_Config();
  
  timeSyncSystem = 1446741778;
  
  isTimeSynchonized = TIMESYNCSTATUS_NONE;
}

/**
 * @brief  Set RTC time
* @param  time_t epochTimeNow : Epoch Time
 * @retval int value for success(1)/failure(0)
 */
int TimingSystemSetSystemTime(time_t epochTimeNow)
{
    RTC_DateTypeDef sdatestructure;
    RTC_TimeTypeDef stimestructure;

    time_t now = epochTimeNow;
    struct tm *calendar = gmtime(&now);
	
    if (calendar == NULL)
			return 0;
    
    /*##-3- Configure the Date #################################################*/
    sdatestructure.Year = calendar->tm_year - 100;
    sdatestructure.Month = calendar->tm_mon + 1;
    sdatestructure.Date = calendar->tm_mday;
    sdatestructure.WeekDay = calendar->tm_wday + 1;
	
    if(HAL_RTC_SetDate(&RtcHandle,&sdatestructure,FORMAT_BIN) != HAL_OK)
    {
        /* Initialization Error */
      return 0;
    }

    /*##-4- Configure the Time #################################################*/
    stimestructure.Hours                = calendar->tm_hour;
    stimestructure.Minutes              = calendar->tm_min;
    stimestructure.Seconds              = calendar->tm_sec;
    stimestructure.TimeFormat           = RTC_HOURFORMAT12_AM;
    stimestructure.DayLightSaving       = RTC_DAYLIGHTSAVING_NONE ;
    stimestructure.StoreOperation       = RTC_STOREOPERATION_RESET;

    if(HAL_RTC_SetTime(&RtcHandle,&stimestructure,FORMAT_BIN) != HAL_OK)
    {
      return 0;
    }
        
    timeSyncSystem      = epochTimeNow;
    
    gmtime(&epochTimeNow);
    isTimeSynchonized   = TIMESYNCSTATUS_SET;
    
    return 1;
}


/**
 * @brief  Get RTC time
* @param  void
* @retval time_t : time retrieved from RTC
 */
time_t 	TimingSystemGetSystemTime(void)
{
  time_t        returnTime;
  struct tm*    pCalendar;
  
  RTC_DateTypeDef sdatestructure;
  RTC_TimeTypeDef stimestructure;
  
  returnTime = 0;
  
  while(isTimeSynchonized!=TIMESYNCSTATUS_SET)
  {
    ThreadAPI_Sleep(10);
  }  
  
  pCalendar             = gmtime((const time_t*)&timeSyncSystem);
 
  if(HAL_RTC_GetTime(&RtcHandle,&stimestructure,FORMAT_BIN)==HAL_OK && HAL_RTC_GetDate(&RtcHandle,&sdatestructure,FORMAT_BIN)==HAL_OK)
  {
      pCalendar->tm_year           = sdatestructure.Year +100;
      pCalendar->tm_mon            = sdatestructure.Month-1;
      pCalendar->tm_mday           = sdatestructure.Date;
      pCalendar->tm_wday           = sdatestructure.WeekDay - 1;
      pCalendar->tm_hour           = stimestructure.Hours;
      pCalendar->tm_min            = stimestructure.Minutes;
      pCalendar->tm_sec            = stimestructure.Seconds;
      pCalendar->tm_isdst          = 0;
      #if  _DLIB_SUPPORT_FOR_AEABI
      pCalendar->__BSD_bug_filler1 = 0;
      pCalendar->__BSD_bug_filler2 = 0;
      #endif
            
      returnTime        = mktime(pCalendar);
      timeSyncSystem    = returnTime;
  }
    
  return returnTime;
}




/** @brief gmtime conversion for ARM compiler
* @param  time_t *timer : File pointer to time_t structure
* @retval struct tm * : date in struct tm format 
 */
 struct tm *gmtimeMDK(register const time_t *timer)
 {   
          static struct tm br_time;
          register struct tm *timep = &br_time;
          time_t time = *timer;
          register unsigned long dayclock, dayno;
          int year = EEPOCH_YR;
  
          dayclock = (unsigned long)time % SSECS_DAY;
          dayno = (unsigned long)time / SSECS_DAY;

          timep->tm_sec = dayclock % 60;
          timep->tm_min = (dayclock % 3600) / 60;
          timep->tm_hour = dayclock / 3600;
          timep->tm_wday = (dayno + 4) % 7;       /* day 0 was a thursday */
          while (dayno >= YYEARSIZE(year)) {
                  dayno -= YYEARSIZE(year);
                  year++;
          }
          timep->tm_year = year - YYEAR0;
          timep->tm_yday = dayno;
          timep->tm_mon = 0;
          while (dayno >= yytab[LLEAPYEAR(year)][timep->tm_mon]) {
                  dayno -= yytab[LLEAPYEAR(year)][timep->tm_mon];
                  timep->tm_mon++;
          }
          timep->tm_mday = dayno + 1;
          timep->tm_isdst = 0;
  
          return timep;
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
