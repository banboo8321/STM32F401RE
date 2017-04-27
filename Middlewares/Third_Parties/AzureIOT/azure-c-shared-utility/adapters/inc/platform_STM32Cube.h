 /**
  ******************************************************************************
  * @file    sensors_platform_nucleo.h
  * @author  Central LAB
  * @version V1.1.0
  * @date    08-August-2016
  * @brief   Header file for sensors_platform_nucleo.c
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

#define SIZE_IOT_HUB_MSG 1024

#include "azure_c_shared_utility/crt_abstractions.h"


#include "stm32f4xx_hal.h"
#include "stm32f4xx_nucleo.h"
//#include "stm32f4xx_hal_cortex.h"


#ifdef __IKS01A2
#define MAGNETOMETER_SENSOR_AUTO (LSM303AGR_M_0)
#include "x_nucleo_iks01a2.h"
#include "x_nucleo_iks01a2_accelero.h"
#include "x_nucleo_iks01a2_gyro.h"
#include "x_nucleo_iks01a2_magneto.h"
#include "x_nucleo_iks01a2_humidity.h"
#include "x_nucleo_iks01a2_temperature.h"
#include "x_nucleo_iks01a2_pressure.h"
#else
#define MAGNETOMETER_SENSOR_AUTO (LIS3MDL_0)
#include "x_nucleo_iks01a1.h"
#include "x_nucleo_iks01a1_accelero.h"
#include "x_nucleo_iks01a1_gyro.h"
#include "x_nucleo_iks01a1_magneto.h"
#include "x_nucleo_iks01a1_humidity.h"
#include "x_nucleo_iks01a1_temperature.h"
#include "x_nucleo_iks01a1_pressure.h"
#endif

#include "wifi_interface.h"
#include "SynchronizationAgent.h"



// Enable sensor masks
#define PRESSURE_SENSOR                         0x00000001
#define TEMPERATURE_SENSOR                      0x00000002
#define HUMIDITY_SENSOR                         0x00000004
#define UV_SENSOR                               0x00000008  // for future use
#define ACCELEROMETER_SENSOR                    0x00000010
#define GYROSCOPE_SENSOR                        0x00000020
#define MAGNETIC_SENSOR                         0x0000004

#define MCR_BLUEMS_F2I_2D(in, out_int, out_dec) {out_int = (int32_t)in; out_dec= (int32_t)((in-out_int)*100);};

typedef struct {
  char strTemp[10];
  char strHum[10];
} EnvSens;

typedef struct {
  char accX[10];
  char accY[10];
  char accZ[10];
  char gyrX[10];
  char gyrY[10];
  char gyrZ[10];
} ImuSens;

static void HumTemp_Sensor_Handler(EnvSens *sens);
static void AccGyro_Sensor_Handler(ImuSens *sens);

extern int Get_MAC_Add (char *macadd);
extern WiFi_Status_t GET_Configuration_Value(char* sVar_name,uint32_t *aValue);


static EnvSens HTReading;
static ImuSens IMUReading;


float PRESSURE_Value;
float HUMIDITY_Value;
float TEMPERATURE_Value;
SensorAxes_t ACC_Value;
SensorAxes_t GYR_Value;
SensorAxes_t MAG_Value;

extern RTC_HandleTypeDef RtcHandle;
RTC_DateTypeDef sdatestructureget;
RTC_TimeTypeDef stimestructure;


void *ACCELERO_handle = NULL;
void *GYRO_handle = NULL;
void *MAGNETO_handle = NULL;
void *HUMIDITY_handle = NULL;
void *TEMPERATURE_handle = NULL;
void *PRESSURE_handle = NULL;

// LED
static GPIO_InitTypeDef  GPIO_InitStruct;


/******************* (C) COPYRIGHT 2013 STMicroelectronics *****END OF FILE****/

