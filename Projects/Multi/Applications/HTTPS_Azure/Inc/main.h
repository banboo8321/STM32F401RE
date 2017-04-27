  /**
  ******************************************************************************
  * @file    main.h
  * @author  Central LAB
  * @version V1.1.0
  * @date    08-August-2016
  * @brief   Header for main.c module
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#ifdef USE_STM32F4XX_NUCLEO
#include "stm32f4xx_hal.h"
#include "stm32f4xx_nucleo.h"
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_rcc_ex.h"
#endif  

#include "wifi_interface.h"
#include "stm32_spwf_wifi.h"
#include "ring_buffer.h"
#include "wifi_const.h"
#include "lib_TagType4.h"

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define UNION_DATA_SIZE ((32+32+6+6)>>2)
#define NDEF_WIFI       32
#ifdef USE_STM32F4XX_NUCLEO
#define BLUEMSYS_FLASH_ADD ((uint32_t)0x08060000)
#define BLUEMSYS_FLASH_SECTOR FLASH_SECTOR_7
#define APP_TIMEOUT 5000 // 5 sec
/* Magic Number for Controlling if SSID/KEY are already saved */
#endif /* USE_STM32F4XX_NUCLEO */
#define WIFI_CHECK_SSID_KEY ((uint32_t)0x12345678)
    
/* Private typedef -----------------------------------------------------------*/
typedef union
{
  uint32_t Data[UNION_DATA_SIZE];
  sWifiTokenInfo TokenInfo;
}uWifiTokenInfo;
#define WIFITOKEN UnionWifiToken.TokenInfo

typedef enum
{ 
  MODULE_SUCCESS           = 1,
  MODULE_ERROR           = 0 
} System_Status_t;





/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

#define RTC_CLOCK_SOURCE_LSI

#ifdef RTC_CLOCK_SOURCE_LSI
  #define RTC_ASYNCH_PREDIV  0x7F
  #define RTC_SYNCH_PREDIV   0x0130
#endif

#ifdef RTC_CLOCK_SOURCE_LSE
  #define RTC_ASYNCH_PREDIV  0x7F
  #define RTC_SYNCH_PREDIV   0x00FF
#endif

#if defined (USE_STM32F1xx_NUCLEO) || (USE_STM32F4XX_NUCLEO)
#define TIMx                           TIM3
#define TIMx_CLK_ENABLE()              __HAL_RCC_TIM3_CLK_ENABLE()


/* Definition for TIMx's NVIC */
#define TIMx_IRQn                      TIM3_IRQn
#define TIMx_IRQHandler                TIM3_IRQHandler
#endif

#ifdef USE_STM32L0XX_NUCLEO
#define TIMx                           TIM2
#define TIMx_CLK_ENABLE()              __TIM2_CLK_ENABLE()


/* Definition for TIMx's NVIC */
#define TIMx_IRQn                      TIM2_IRQn
#define TIMx_IRQHandler                TIM2_IRQHandler
#endif

    

    
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void Timer_Config(void);
void Timeout_SysTick_Isr(void);
//void UART_Msg_Gpio_Init(void);
//void USART_PRINT_MSG_Configuration(void);

uint32_t user_currentTimeGetTick(void);
uint32_t getElapsedMSFromLastGetTime(void);

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/


