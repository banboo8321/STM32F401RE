 /**
  ******************************************************************************
  * @file    sensors_platform_nucleo.c
  * @author  Central LAB
  * @version V1.1.0
  * @date    08-August-2016
  * @brief   Initialize sensors board in HTTP Client application
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

/** @addtogroup MIDDLEWARES
* @{
*/ 


/** @addtogroup AZURE_SDK
* @{
*/ 

/** @defgroup  AZURE_ADAPTER_SENSORS
  * @brief Sensor board Interface
  * @{
  */


#include <stdio.h>
#include <stdlib.h>

#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/threadapi.h"

#include "platform_STM32Cube.h"
#include "GPRS_module.h"
static bool b_sensor_initialized = false;


/**
  * @brief  Initialize sensors 
  * @param  None
  * @retval int value for success (1) / failure (0)
  */
int platform_init(void)
{
   // Local counter for registration procedure
   short unsigned int i_local_counter = 0;
   short unsigned int i_err_code;
   bool b_ntp_sync = false;
   bool b_ntp_backup = false;
   static const char* ntpServerString = SYNCHRONIZATIONAGENT_DEFAULT_ENDPOINT_IPADDRESS_TEST;

  
   // Registration procedure.  
   printf("[IotHub]. Launching NTP procedure. \r\n");  
   while (i_local_counter < SYNCHRONIZATIONAGENT_DEFAULT_NTP_RETRY)
   {  
          if ( i_err_code = SynchronizationNTPStart(ntpServerString,SYNCHRONIZATIONAGENT_DEFAULT_ENDPOINT_TCPPORT) != NTP_SUCCESS ){
            printf("[IotHub][E]. Failed time sync with NTP server: %s. Error code: %d \r\n", ntpServerString, i_err_code);
            if (i_local_counter < (SYNCHRONIZATIONAGENT_DEFAULT_NTP_RETRY-1) ){
                  printf("[IotHub][E]. Trying to connect again with NTP server \r\n");
                   i_local_counter++;
            } else {
               // try backup
               if (!b_ntp_backup)  
                 b_ntp_backup = true;
               else
                 break;
               printf("[IotHub][E]. Connect with backup NTP \r\n");
               ntpServerString = SYNCHRONIZATIONAGENT_DEFAULT_ENDPOINT_IPADDRESS_BACKUP;
               i_local_counter = 0;  
            }   
            ThreadAPI_Sleep(5000);//5000
          }
          else {
            printf("[IotHub]. Sync with NTP server completed. \r\n");
            b_ntp_sync = true;
            break;
          }
    } 
    // Failed NTP
    if (!b_ntp_sync)
      return 1;
    
    ThreadAPI_Sleep(1500);
        
    // Init LED Nucleo
    BSP_LED_Init(LED2); 
#if SENSOR_INSTALL
  




    /* Initialize Sensor */
#ifdef __IKS01A2
      /* Try to use LSM6DS3 DIL24 if present */
      if(BSP_ACCELERO_Init( LSM303AGR_X_0, &ACCELERO_handle )!=COMPONENT_OK){        
        /* otherwise try to use LSM6DS0 on board */
        if(BSP_ACCELERO_Init( LSM6DSL_X_0, &ACCELERO_handle )!=COMPONENT_OK){
          printf("[IotHub][E]. Failed init sensors (Accelero). \r\n");
          return 2;
        }
      }    
#else
  /* Try to use IIS2DH, LSM303AGR or LSM6DSL accelerometer */
  if(BSP_ACCELERO_Init( ACCELERO_SENSORS_AUTO, &ACCELERO_handle ) != COMPONENT_OK){
    printf("[IotHub][E]. Failed init sensors (Accelero). \r\n");
    return 2;
  }        
#endif  
  
    /* Try to use LSM6DS3 if present, otherwise use LSM6DS0 */
    if(BSP_GYRO_Init( GYRO_SENSORS_AUTO, &GYRO_handle )!=COMPONENT_OK){
                printf("[IotHub][E]. Failed init sensors (Gyro). \r\n");
		return 2;
	}

    if(BSP_MAGNETO_Init( MAGNETOMETER_SENSOR_AUTO, &MAGNETO_handle )!=COMPONENT_OK){
          printf("[IotHub][E]. Failed init sensors (Magneto). \r\n");
          return 2;
    }

    /* Force to use HTS221 */
    if(BSP_HUMIDITY_Init( HTS221_H_0, &HUMIDITY_handle )!=COMPONENT_OK){
                printf("[IotHub][E]. Failed init sensors (Hum). \r\n");
		return 2;
    }

    /* Force to use HTS221 */
    if(BSP_TEMPERATURE_Init( HTS221_T_0, &TEMPERATURE_handle )!=COMPONENT_OK){
                printf("[IotHub][E]. Failed init sensors (Temp). \r\n");
		return 2;
    }

    /* Try to use LPS25HB DIL24 if present, otherwise use LPS25HB on board */
    if(BSP_PRESSURE_Init( PRESSURE_SENSORS_AUTO, &PRESSURE_handle )!=COMPONENT_OK){
                printf("[IotHub][E]. Failed init sensors (Pressure). \r\n");
		return 2;
    }
    
    /*  Enable all the sensors */
    BSP_ACCELERO_Sensor_Enable( ACCELERO_handle );
    BSP_GYRO_Sensor_Enable( GYRO_handle );
    BSP_MAGNETO_Sensor_Enable( MAGNETO_handle );
    BSP_HUMIDITY_Sensor_Enable( HUMIDITY_handle );
    BSP_TEMPERATURE_Sensor_Enable( TEMPERATURE_handle );
    BSP_PRESSURE_Sensor_Enable( PRESSURE_handle );
#else

#endif
    printf("[IotHub]. Sensors initialized. \r\n");
    b_sensor_initialized = true;

    return 0;
}





/**
  * @brief  Compose message to be transmitted to IoT Hub (date, sensors data, MAC address of the WiFi board) 
  * @param  char* textMsg : pointer to the buffer containing the message to be transmitted to IoT Hub
  * @retval int value for success (1) / failure (0)
  */
int ComposeMessageSensors(char* textMsg)
{
    char   timestr[32];
    static uint8_t DisplayName[32];
    char   strDummyTemp[10];

    if (b_sensor_initialized) {
        HumTemp_Sensor_Handler((EnvSens *) &HTReading);
        AccGyro_Sensor_Handler((ImuSens *) &IMUReading);
    }
    
    memset((void *)textMsg, 0, SIZE_IOT_HUB_MSG);

    HAL_RTC_GetTime(&RtcHandle, &stimestructure, FORMAT_BIN);
    HAL_RTC_GetDate(&RtcHandle, &sdatestructureget, FORMAT_BIN); 
	
    if ((sprintf_s(timestr, sizeof(timestr), "20%02d-%02d-%02dT%02d:%02d:%02d.000000Z", sdatestructureget.Year, sdatestructureget.Month, sdatestructureget.Date,stimestructure.Hours, stimestructure.Minutes, stimestructure.Seconds)) < 0)
    {
        printf("[IoTHub][E]. Failed to compose message \r\n");			
        return 0;  
    }	
    
    if (Get_MAC_Add ((char *) DisplayName)==0)
    {
            printf("[IoTHub][E]. Failed o retrieve MAC. \r\n");
            return 0;
    } 
    
    if (b_sensor_initialized) {
     if( (sprintf_s((char *)textMsg, SIZE_IOT_HUB_MSG, "{\"id\":\"%s\",\"name\":\"Nucleo-%s\",\"ts\":\"%s\",\"mtype\":\"ins\",\"temp\":%s,\"hum\":%s,\"accX\":%s,\"accY\":%s,\"accZ\":%s,\"gyrX\":%s,\"gyrY\":%s,\"gyrZ\":%s}", 
                                                                                      DisplayName,
                                                                                      DisplayName,
                                                                                      timestr, HTReading.strTemp, HTReading.strHum, 
                                                                                      IMUReading.accX, IMUReading.accY, IMUReading.accZ,
                                                                                      IMUReading.gyrX, IMUReading.gyrY, IMUReading.gyrZ ) ) < 0)
                                                                            {
              printf("[IoTHub][E]. Failed to compose message. \r\n");			
              return 0;  
     }
    } else {
       // EQ. Test this.
       sprintf(strDummyTemp, "%ld.%ld", 25,50);
       if( (sprintf_s((char *)textMsg, SIZE_IOT_HUB_MSG, "{\"id\":\"%s\",\"name\":\"Nucleo-%s\",\"ts\":\"%s\",\"mtype\":\"ins\",\"temp\":%s}", 
                                                                                        DisplayName,
                                                                                        DisplayName,
                                                                                        timestr, strDummyTemp ) ) < 0)
                                                                              {
                printf("[IoTHub][E]. Failed to compose message. \r\n");			
                return 0;  
       }      
    }
     
     return 1; 
}

void set_led()
{
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET); 
      BSP_LED_On(LED2);      
      return;
}            



void reset_led()
{
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);   
      BSP_LED_Off(LED2);
      
      return;
}            


void reset_board()
{
    HAL_NVIC_SystemReset();
    
    return;
}


/**
 * @brief  Handles the HUM+TEMP axes data getting/sending
 * @param  Msg - HUM+TEMP part of the stream
 * @retval None
 */
static void HumTemp_Sensor_Handler(EnvSens *sens)
{
  int32_t decPart, intPart;
  uint8_t Status;

  if(BSP_TEMPERATURE_IsInitialized(TEMPERATURE_handle,&Status)==COMPONENT_OK){
    BSP_TEMPERATURE_Get_Temp(TEMPERATURE_handle,(float *)&TEMPERATURE_Value);
    MCR_BLUEMS_F2I_2D(TEMPERATURE_Value, intPart, decPart);
    sprintf(sens->strTemp, "%ld.%ld", intPart,decPart);
  }

  if(BSP_HUMIDITY_IsInitialized(HUMIDITY_handle,&Status)==COMPONENT_OK){
    BSP_HUMIDITY_Get_Hum(HUMIDITY_handle,(float *)&HUMIDITY_Value);
    MCR_BLUEMS_F2I_2D(HUMIDITY_Value, intPart, decPart);
    sprintf(sens->strHum, "%ld.%ld", intPart,decPart);
  }
}

/**
 * @brief  Handles the ACC+GYRO axes data getting/sending
 * @param  Msg - ACC+GYRO part of the stream
 * @retval None
 */
static void AccGyro_Sensor_Handler(ImuSens *sens)
{
  BSP_ACCELERO_Get_Axes(ACCELERO_handle,&ACC_Value);
  BSP_GYRO_Get_Axes(GYRO_handle,&GYR_Value);

  sprintf(sens->accX, "%ld", ACC_Value.AXIS_X);
  sprintf(sens->accY, "%ld", ACC_Value.AXIS_Y);
  sprintf(sens->accZ, "%ld", ACC_Value.AXIS_Z);
  sprintf(sens->gyrX, "%ld", GYR_Value.AXIS_X);
  sprintf(sens->gyrY, "%ld", GYR_Value.AXIS_Y);
  sprintf(sens->gyrZ, "%ld", GYR_Value.AXIS_Z);
}

void platform_deinit(void)
{
	return;
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
