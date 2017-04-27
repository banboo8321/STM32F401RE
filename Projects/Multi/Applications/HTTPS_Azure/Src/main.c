   /**
  ******************************************************************************
  * @file    main.c
  * @author  Central LAB
  * @version V2.0.0
  * @date    24-Nov-2016
  * @brief   Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "stm32f4xx_I2C.h"
#include "stm32f4xx_periph_conf.h"

#include "cmsis_os.h"
#include <time.h>   // time functions
#include <stdint.h>
#include <string.h>
#include "main.h"
#include "InternetInterfaceDriver.h"

#include "cmsis_os.h"
#include "ThreadAPI_Port.h"
#include "azure_c_shared_utility/threadapi.h"
#include "AzureIOTSDKConfig.h"
#include "CompilerCheckError.h"
#include "timingSystem.h"
#include "InternetAdapter.h"
#include "iothub_client_sample_http.h"
#include "iothub_client_version.h"
#include "GPRS_module.h"
#include "wifi_module.h"
//#include "ssl.h"
/**
   * @mainpage FP-CLD-AZURE1 firmware package
   * <b>Introduction</b> <br>
   * FP-CLD-AZURE1 provides a software running on STM32 which offers a complete middleware to build applications based on Wi-Fi connectivity (SPW01SA) and to connect STM32 Nucleo boards 
   * with Microsoft Azure IoT services. This firmware package includes Components Device Drivers, Board Support Package and example application for STMicroelectronics X-NUCLEO-IDW01M1
   * X-NUCLEO-NFC01A1, X-NUCLEO-IKS01A1.

   * <b>Microsoft Azure IoT example applications:</b><br>
   * This application reads the sensor data values from the accelerometer, magnetometer and Gyroscope, which are transmitted to the Azure IoT Hub through WiFi. 
   * NFC tag can be used to configure Wi-Fi Access Point parameters. 
   * The application requires to register an account on Microsoft Azure (https://azure.microsoft.com/en-us/pricing/free-trial/) and then to create an instance of Azure IoT Hub (https://github.com/Azure/azure-iot-sdks/blob/master/doc/setup_iothub.md)
  
   * <b>Toolchain Support</b><br>
   * The library has been developed and tested on following toolchains:
   *        - IAR Embedded Workbench for ARM (EWARM) toolchain V7.20 + ST-Link
   *        - Keil Microcontroller Development Kit (MDK-ARM) toolchain V5.10 + ST-LINK
   *        - System Workbench for STM32 (SW4STM32) V1.2 + ST-LINK
*/



/** @addtogroup FP-CLD-AZURE1
  * @{
  */

/* Extern functions ---------------------------------------------------------*/
extern WiFi_Status_t GET_Configuration_Value(char* sVar_name,uint32_t *aValue);
/* Static functions ---------------------------------------------------------*/
 System_Status_t ReCallSSIDPasswordFromMemory(void);
 System_Status_t ResetSSIDPasswordInMemory(void);
 System_Status_t SaveSSIDPasswordToMemory(void);
 System_Status_t ReadWifiTokenFromNFC(void);
 System_Status_t WriteURLtoNFC(char* UrlForNFC);
 TInternetAdapterSettings adapterInternetSettings;
 uWifiTokenInfo UnionWifiToken;
 System_Status_t InitNucleo(void);
 System_Status_t InitNFC(void);
 System_Status_t InitWiFi(void);
 System_Status_t ActivateWiFi(void);

 System_Status_t ConfigAPSettings(void);
 System_Status_t wifi_get_AP_settings(void);
 System_Status_t  Get_MAC_Add (char *macadd);    
 System_Status_t CheckAzureIOTConfigurationParameters(void);
 void PrintAssemblyInformation(void);

/* Static variable ---------------------------------------------------------*/
bool b_nfc_present = true;
char WIFI_AT_Cmd_Buff_test[512];
char WIFI_AT_Cmd_Buff_test_Rx[512];
UART_HandleTypeDef UartWIFIHandle_test;
//WOLFSSL* ssl_t;
/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{  
   /* Init Nucleo Board */
  if (InitNucleo () !=  MODULE_SUCCESS )
  {  
    printf("\n\r[Init][E].Failed to Initialize Nucleo board \n\r");
    return 0; 
  }   
  else
    printf("\n\r[Init]. Nucleo board initialized. ");
  
  printf("\n\r**************************************************************************\n\r");
  printf("***                  Azure IoT SDK for STM32 Nucleo                   ****\n\r");
  printf("**************************************************************************\n\r");

 #if 0  //NFC_NO_INSTALL  /*add	by jiangdong at 2/10*/
  /* Init NFC expansion board */
  printf("\n\r[Init]. Initializing NFC... \r\n"); 
  if (InitNFC () !=  MODULE_SUCCESS )
  {  
    printf("\n\r[Init][E]. Failed to Initialize NFC board."); 
    b_nfc_present = false;
  }   
  else
    printf("\n\r[Init]. NFC expansion board initialized."); 
 #endif 
  	BSP_LED_Toggle(LED2);//PA5 /*add	by jiangdong at 2/14*/
	HAL_Delay(1000);
	BSP_LED_Toggle(LED2);//PA5
	HAL_Delay(1000);
	//wolfSSL_library_init();
	//wolfSSL_connect(ssl_t);
  /* Set AP parameters */  
  if ( (X_CLOUD_SSID!=NULL) && (X_CLOUD_KEY!=NULL) )
  {
    printf("\r\n[Init]. Using SSID and PWD writtend in the code. "); 

    adapterInternetSettings.pSSID          = X_CLOUD_SSID;
    adapterInternetSettings.pSecureKey     = X_CLOUD_KEY;
    adapterInternetSettings.authMethod     = X_CLOUD_AUTH_METHOD;
  }
  else
  {
        printf("\r\n[Init]. Starting configuration procedure for SSID and PWD....  "); 
        HAL_Delay(3000);
          
        if (ConfigAPSettings() !=  MODULE_SUCCESS )
        {  
          printf("\n\r[Init][E]. Failed to set AP settings."); 
          return 0; 
        }   
        else
          printf("\n\r[Init]. AP settings set.");         
  }      


#if APPLICATION_SCENARIO <= AZURE_ENDLESS_LOOP  
  /* Check AP and Azure IoT Hub parameters */  
  if(CheckAzureIOTConfigurationParameters()!=MODULE_SUCCESS) {
    printf("\r\n[Init][E]. Wrong Configuration. Application Exit\r\n");
    return 0;
  }
#else
   printf("\n\r[Version][E]. This scenario is only enabled by usage of pre-compiled binaries available in the zip file. Application Exit\r\n" );
   HAL_Delay(1500); 
   return 0;
#endif

   /* Init WiFi Interface  */  
   if (InitWiFi() != MODULE_SUCCESS) 
   {  
        printf("\n\r[Init][E]. Failed to initialize WiFi data structures."); 
        return 0; 
   }   
   else
        printf("\n\r[Init]. Initilized WiFi data structures ");  
#if GPRS_TEST
   GPRS_test();
#else

#endif
    /* Pre-allocate space for Threads */  
   ThreadAPI_Initialize(); 
   HAL_Delay(500); 
  
   /* Activate WiFi   */  
   if (ActivateWiFi() != MODULE_SUCCESS) 
   {  
        printf("\n\r[Init][E]. Failed to activate WiFi and connect it to AP."); 
        return 0; 
   }        
     
   /* Print assembly information */  
   PrintAssemblyInformation();
   
   HAL_Delay(3000);
#if 0//!GPRS_INSTALL

#else



  /* Create main thread for IoTHub HTTP CLient Application */
  osThreadDef(defaultTask, (os_pthread)iothub_client_sample_http_run, osPriorityNormal, 0, AZURERUNEXAMPLE_STACKSIZE);
  osThreadCreate(osThread(defaultTask), NULL);
	  
  /* Start scheduler */
  osKernelStart();
#endif
}


/**
  * @brief  Initialize Nucleo board
  * @param  None
  * @retval System_Status_t (MODULE_SUCCESS/MODULE_ERROR)
  */
 System_Status_t InitNucleo(void)
{
   System_Status_t status = MODULE_ERROR;
  
   /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
   HAL_Init();

   /* Configure the system clock */
   SystemClock_Config();

   /* configure the timers  */
   Timer_Config();
   UART_Msg_Gpio_Init();
   //USART_PRINT_MSG_Configuration(460800);/*modify by jiangdong at 2/17*/
   USART_PRINT_MSG_Configuration(115200);
#if GPRS_INSTALL
   printf("\n\r GPRS_UART_Configuration \n\r");
   UART_Configuration(115200);
   GPRS_UART_Configuration(115200);
#else
   printf("\n\r UART_Configuration \n\r");
   UART_Configuration(115200);
#endif


   BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_GPIO);
   BSP_LED_Init(LED2);/*add by jiangdong at 2/14*/
  /* I2C Initialization */
  if(I2C_Global_Init()!=HAL_OK) {
    printf("\n\r I2C Init Error \n\r");
    return status;
  }
  else
    status = MODULE_SUCCESS;
   
   return status;
 }
  
  
/**
  * @brief  Initialize NFC board
  * @param  None
  * @retval System_Status_t (MODULE_SUCCESS/MODULE_ERROR)
  */
 System_Status_t InitNFC(void)
{
   System_Status_t status = MODULE_ERROR;
   //int i_fail_counter = 0;
   
    /* Initialize the M24SR component */
     while (TT4_Init() != SUCCESS){
       HAL_Delay (100);
       //i_fail_counter++;
       // try for one second to initialize NFC
       //if (i_fail_counter > 10)
         return status;
     }
  
  status = MODULE_SUCCESS;
  return status;
}

/**
  * @brief  Initialize WiFi board
  * @param  None
  * @retval System_Status_t (MODULE_SUCCESS/MODULE_ERROR)
  */
 System_Status_t InitWiFi(void)
{
   System_Status_t status = MODULE_ERROR;

   TimingSystemInitialize();

   if ( strnlen(adapterInternetSettings.pSSID, sizeof(char) ) < 1 ){
         printf("\r\n[Init][E]. SSID string is void \r\n");
         return status;
   }
     
   if (InternetInterfaceInitialize(&adapterInternetSettings) != INTERNETINTERFACERESULT_SUCCESS ){
    printf("\r\n[Init][E]. Failed Iniatialization Internet Interface. \r\n");
    return status;
  }
  
  status = MODULE_SUCCESS;
  return status;
}

/**
  * @brief  Activate WiFi board, connect to WiFi access point and launch NTP thread
  * @param  None
  * @retval System_Status_t (MODULE_SUCCESS/MODULE_ERROR)
  */
 System_Status_t ActivateWiFi(void)
{
   System_Status_t status = MODULE_ERROR;
 #if APPLICATION_SCENARIO >= AZURE_ENDLESS_LOOP_BINARY 
   uint8_t DisplayName[32];
   uint8_t URL_MS[64];
#endif

  /* Activate Wi-Fi */
  if ( InternetAdapterStart(CALLINGPROCEDURETYPE_MAIN) != INTERNETINTERFACERESULT_SUCCESS ){
    printf("\r\n[Init].[E]. Failed Iniatialization Internet Adapter. \r\n");
    return status;
  } 
   
  HAL_Delay(2000);
#if APPLICATION_SCENARIO >= AZURE_ENDLESS_LOOP_BINARY
  strcpy((char *)URL_MS, AZURE_DASHBOARD_PORTAL);
  status = Get_MAC_Add((char *)DisplayName);

  if(status!=MODULE_SUCCESS)
  { 
         printf("\r\n[E]. Error while retrieving MAC address \r\n");  
         return status;    
  }
  
  strcat((char *)URL_MS,(char *)DisplayName); 
  
  if (b_nfc_present == true ){
      status = WriteURLtoNFC((char *)URL_MS);
      if(status!=MODULE_SUCCESS)
      { 
              printf("\r\n[E]. Error while writing URL to NFC \r\n");  
              return status;    
      }
  }
  
  printf("\r\n[Init]. URL for data visualization. \r\n");
  printf((char *)(URL_MS));
  printf("\r\n");
 
#endif
  
  status = MODULE_SUCCESS;
  return status;
}



/**
  * @brief  Configure Access Point parmaters (SSID, PWD, Authenticaation) :
  * @brief  1) Read from FLASH
  * @brief  2) If not available in FLASH or when User Button is pressed : read from NFC
  * @brief  3) If not available in FLASH or when User Button is pressed and if nothing is written in NFC : read from serial terminal
  * @param  None
  * @retval System_Status_t (MODULE_SUCCESS/MODULE_ERROR)
  */
 System_Status_t ConfigAPSettings(void)
{
   System_Status_t status = MODULE_ERROR;
   bool   b_set_AP_pars = false;
   bool   b_read_AP_pars_NFC = false;
   
   uint8_t URL_MS[64];


   printf("\r\n[Init]. Keep pressed user button to set Wi-Fi Access Point parameters (SSID and PWD) "); 
   printf("\r\nfrom NFC (if mounted) or via serial terminal. Otherwise parameters saved to FLASH will be used.");
   HAL_Delay(4000);
   
   /* User Button Pressed --> set parameters */
   if(BSP_PB_GetState(BUTTON_KEY) != GPIO_PIN_RESET) {
          /* Read from FLASH */
          if(ReCallSSIDPasswordFromMemory()) {
              printf("\n\r[Init]. Read from FLASH:\n\r");
              printf("\tSSID =%s\n\r",WIFITOKEN.NetworkSSID);
              printf("\tKey  =%s\n\r",WIFITOKEN.NetworkKey);
              printf("\tAuthentication  =%s\n\r",WIFITOKEN.AuthenticationType);

              adapterInternetSettings.pSSID          = WIFITOKEN.NetworkSSID;
              adapterInternetSettings.pSecureKey     = WIFITOKEN.NetworkKey;
              adapterInternetSettings.authMethod     = WIFITOKEN.AuthenticationType; 
              status = MODULE_SUCCESS;
          }
          else
          { 
              printf("\n\r[Init]. No data written in FLASH.");
              b_set_AP_pars = true;
          }   
    }  
    else
       b_set_AP_pars = true;

    if (b_set_AP_pars)
    {
       if (b_nfc_present == true ){
            printf("\r\n[Init]. Reading from NFC....\n\r");       
            /* Read from FLASH. 5 sec timeout */
            status = ReadWifiTokenFromNFC();//????
            
            if(status==MODULE_SUCCESS)
            {
                printf("[Init]. Read from NFC:\n\r");
                printf("\tSSID =%s\n\r",WIFITOKEN.NetworkSSID);
                printf("\tKey  =%s\n\r",WIFITOKEN.NetworkKey);
                printf("\tAuthentication  =%s\n\r",WIFITOKEN.AuthenticationType);
                
                adapterInternetSettings.pSSID          = WIFITOKEN.NetworkSSID;
                adapterInternetSettings.pSecureKey     = WIFITOKEN.NetworkKey;
                adapterInternetSettings.authMethod     = WIFITOKEN.AuthenticationType; 
                
                b_read_AP_pars_NFC = true;
                
                /* Prevent to read wrong SSID/PWD from NFC next time */
                strcpy((char *)URL_MS, "www.st.com/stm32ode");
                status = WriteURLtoNFC((char *)URL_MS);  
                if(status!=MODULE_SUCCESS)
                { 
                      printf("\r\n[Init][E]. Error while writing URL to NFC \r\n");  
                      return status;    
                }
                
                 /* Save to FLASH */  
                 status = SaveSSIDPasswordToMemory();
                 if(status!=MODULE_SUCCESS)
                 {
                        printf("\r\n[Init][E]. Error in AP Settings\r\n");
                        return status;
                 }

                
            }     
            else 
              printf("[Init]. No Wi-Fi parameters from NFC. \n\r"); 

          }             
       
          if ( (b_nfc_present == false) || (b_read_AP_pars_NFC == false) )
          {  
              printf("\n\r[Init]. Read parameters from serial terminal."); 

               /* Read from Serial.  */
               status = wifi_get_AP_settings();        
               if(status!=MODULE_SUCCESS)
               {
                        printf("\r\n[Init][E]. Error in AP Settings\r\n");
                        return status;
               }
                  
                adapterInternetSettings.pSSID          = WIFITOKEN.NetworkSSID;
                adapterInternetSettings.pSecureKey     = WIFITOKEN.NetworkKey;
                adapterInternetSettings.authMethod     = WIFITOKEN.AuthenticationType; 

                 /* Save to FLASH */  
                 status = SaveSSIDPasswordToMemory();
                 if(status!=MODULE_SUCCESS)
                 {
                        printf("\r\n[Init][E]. Error in AP Settings\r\n");
                        return status;
                 }
          }      

        }
    
         /* Added for testing different code configurations */
         if (b_nfc_present == true ){
              strcpy((char *)URL_MS, "www.st.com/stm32ode");
              status = WriteURLtoNFC((char *)URL_MS);  
              if(status!=MODULE_SUCCESS)
              { 
                      printf("\r\n[Init][E]. Error while writing URL to NFC \r\n");  
                      return status;    
              }  
          }
   
  return status;
}




/**
  * @brief  Save Access Point parameters to FLASH
  * @param  None
  * @retval System_Status_t (MODULE_SUCCESS/MODULE_ERROR)
  */
 System_Status_t SaveSSIDPasswordToMemory(void)
{
  System_Status_t status = MODULE_ERROR;

  /* Reset Before The data in Memory */
  status = ResetSSIDPasswordInMemory();

  if(status) {
    /* Store in Flash Memory */
    uint32_t Address = BLUEMSYS_FLASH_ADD;
    int32_t WriteIndex;

   /* Unlock the Flash to enable the flash control register access *************/
   HAL_FLASH_Unlock();
   
   /* Write the Magic Number */
   {
     uint32_t MagicNumber = WIFI_CHECK_SSID_KEY;
      if (HAL_FLASH_Program(TYPEPROGRAM_WORD, Address,MagicNumber) == HAL_OK){
        Address = Address + 4;
      } else {
          printf("\r\n[Init][E]. Error while writing in FLASH");
          status = MODULE_ERROR;
        }
   }
   
    /* Write the Wifi */
    for(WriteIndex=0;WriteIndex<UNION_DATA_SIZE;WriteIndex++){
      if (HAL_FLASH_Program(TYPEPROGRAM_WORD, Address,UnionWifiToken.Data[WriteIndex]) == HAL_OK){
        Address = Address + 4;
      } else {
          printf("\r\n[Init][E]. Error while writing in FLASH");
          status = MODULE_ERROR;
        }
      }
    

    /* Lock the Flash to disable the flash control register access (recommended
     to protect the FLASH memory against possible unwanted operation) *********/
    HAL_FLASH_Lock();
    
    //printf("\n\rSaveSSIDPasswordToMemory OK");
  }
  else
    printf("\n\r[Init][E]. Error while resetting FLASH memory");
    
  return status;
}

/**
  * @brief  Erase Access Point parameters from FLASH
  * @param  None
  * @retval System_Status_t (MODULE_SUCCESS/MODULE_ERROR)
  */
 System_Status_t ResetSSIDPasswordInMemory(void)
{
  /* Reset Calibration Values in FLASH */
  System_Status_t status = MODULE_ERROR;

  /* Erase First Flash sector */
  FLASH_EraseInitTypeDef EraseInitStruct;
  uint32_t SectorError = 0;

  EraseInitStruct.TypeErase = TYPEERASE_SECTORS;
  EraseInitStruct.VoltageRange = VOLTAGE_RANGE_3;
  EraseInitStruct.Sector = BLUEMSYS_FLASH_SECTOR;
  EraseInitStruct.NbSectors = 1;

  /* Unlock the Flash to enable the flash control register access *************/
  HAL_FLASH_Unlock();

  if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK){
    /* Error occurred while sector erase. 
      User can add here some code to deal with this error. 
      SectorError will contain the faulty sector and then to know the code error on this sector,
      user can call function 'HAL_FLASH_GetError()'
      FLASH_ErrorTypeDef errorcode = HAL_FLASH_GetError(); */
      printf("\n\r[Init][E]. Error while erasing FLASH memory");
  } else
      status = MODULE_SUCCESS;
  
  /* Lock the Flash to disable the flash control register access (recommended
  to protect the FLASH memory against possible unwanted operation) *********/
  HAL_FLASH_Lock();

  return status;
}

/**
  * @brief  Read Access Point parameters from FLASH
  * @param  None
  * @retval System_Status_t (MODULE_SUCCESS/MODULE_ERROR)
  */
 System_Status_t ReCallSSIDPasswordFromMemory(void)
{
  System_Status_t status = MODULE_ERROR;
  
  uint32_t Address = BLUEMSYS_FLASH_ADD;
  __IO uint32_t data32 = *(__IO uint32_t*) Address;
  if(data32== WIFI_CHECK_SSID_KEY){
    int32_t ReadIndex;

    for(ReadIndex=0;ReadIndex<UNION_DATA_SIZE;ReadIndex++){
      Address +=4;
      data32 = *(__IO uint32_t*) Address;
      UnionWifiToken.Data[ReadIndex]=data32;
    }
    status = MODULE_SUCCESS;
  }
  else
    printf("\r\n[Init][E]. FLASH Keyword not found.");
  
  return status;
}


  

/**
  * @brief  Read Access Point parameters from serial interface
  * @param  None
  * @retval System_Status_t (MODULE_SUCCESS/MODULE_ERROR)
  */
 System_Status_t wifi_get_AP_settings(void)
{
  System_Status_t status = MODULE_ERROR;
  uint8_t console_input[1], console_count=0;
  char console_ssid[NDEF_WIFI];
  char console_psk[NDEF_WIFI];
  WiFi_Priv_Mode mode; 
    
  
              printf("\r\n[Init]. Enter the SSID:");
              fflush(stdout);

              console_count=0;
              console_count=scanf("%s",console_ssid);
              printf("\r\n");
              // FIXME : Why 39. NDEF is 32
              if(console_count==NDEF_WIFI) 
              {
                        printf("Exceeded number of ssid characters permitted");
                        return status;
               }    

              printf("Enter the password:");
              fflush(stdout);
              console_count=0;
              
              console_count=scanf("%s",console_psk);
              printf("\r\n");
              // FIXME : Why 19. NDEF is 32
              if(console_count==NDEF_WIFI) 
                    {
                        printf("Exceeded number of psk characters permitted");
                        return status;
                    }    
              printf("Enter the authentication mode(0:Open, 1:WEP, 2:WPA2/WPA2-Personal):"); 
              fflush(stdout);
              scanf("%s",console_input);
              printf("\r\n");
              //printf("entered =%s\r\n",console_input);
              switch(console_input[0])
              {
                case '0':
                  mode = None;
                  break;
                case '1':
                  mode = WEP;
                  break;
                case '2':
                  mode = WPA_Personal;
                  break;
                default:
                  printf("\r\nWrong Entry. Priv Mode is not compatible\n");
                  return status;              
              }
              
              memcpy(WIFITOKEN.NetworkSSID, console_ssid, strlen(console_ssid));
              memcpy(WIFITOKEN.NetworkKey, console_psk, strlen(console_psk));
              if (mode == None)
                memcpy(WIFITOKEN.AuthenticationType, "NONE", strlen("NONE"));
              else if (mode == WEP)
                memcpy(WIFITOKEN.AuthenticationType, "WEP", strlen("WEP"));
              else
                memcpy(WIFITOKEN.AuthenticationType, "WPA2", strlen("WPA2"));
              
              status = MODULE_SUCCESS;
              
              return status;
}


/**
  * @brief  Check if Connection string had been set
  * @param  None
  * @retval System_Status_t (MODULE_SUCCESS/MODULE_ERROR)
  */
 System_Status_t CheckAzureIOTConfigurationParameters(void)
{
  System_Status_t status = MODULE_ERROR;
  
  if (AZUREIOTHUBCONNECTIONSTRING == NULL){
    printf("\r\n[Init][E]. The Connection String has not been set \r\n(Check AzureIOTSDKConfig.h headers)\r\n");
    printf("Please refer to User Manual to learn how to generate a connection string for Microsoft Azure IoT. \r\n");
  }
  else
      status = MODULE_SUCCESS;
  return status;
}

/**
  * @brief  Print assembly information
  * @param  None
  * @retval System_Status_t (MODULE_SUCCESS/MODULE_ERROR)
  */
 void PrintAssemblyInformation(void)
{
  printf("\r\n[Version]. FP-CLD-AZURE1 version : %s\r\n",AZUREIOTSDK_STM32NUCLEOF4PORTING_VERSION); 
  printf("[Version]. Microsoft Azure IoT SDK version %s\r\n", (char *)IoTHubClient_GetVersionString() );
  
  
#if defined(__IAR_SYSTEMS_ICC__) && defined (__ICCARM__) && defined(__VER__)
        printf("[Version]. IAR Compiler Platform: %d Compiler: %d. Version: %d\r\n",__IAR_SYSTEMS_ICC__,__ICCARM__,__VER__);
#elif defined (__CC_ARM)
	printf("[Version]. KEIL Compiler Platform\r\n");
#elif defined (__GNUC__)
	printf("[Version]. Openstm32 Compiler Platform\r\n");
#endif
  printf("[Version]. Build Date: %s Time: %s\r\n",__DATE__,__TIME__);

#if defined(__NUCLEO_BUILD)  
  printf("[Version]. STM32 Nucleo Build. \r\n");
#endif
#if defined(__IKS01A2)
  printf("[Version]. Built for X_NUCLEO_IKS01A2 sensor board. \r\n");
#else
  printf("[Version]. Built for X_NUCLEO_IKS01A1 sensor board. \r\n");
#endif   
#if APPLICATION_SCENARIO >= AZURE_ENDLESS_LOOP_BINARY  
  printf("[Version]. Starting ST testing application for Microsoft Azure. \r\n" );
#else
  printf("[Version]. Starting IoTHub HTTP Client application. \r\n" );  
#endif
#if APPLICATION_SCENARIO > AZURE_ENDLESS_LOOP_BINARY  
  printf("[Version]. ST internal unlimited version. \r\n" );
#endif
  
 
  printf("\r\n");
}


/**
  * @brief  Write URL to NFC
  * @param  char* UrlForNFC: string containing URL to write in NFC
  * @retval System_Status_t (MODULE_SUCCESS/MODULE_ERROR)
  */
 System_Status_t WriteURLtoNFC(char* UrlForNFC)
{
  sURI_Info URI;
  System_Status_t status = MODULE_ERROR;
  
   /* Write URI */
   strcpy(URI.protocol,URI_ID_0x03_STRING);   
   strcpy(URI.URI_Message,(char *)UrlForNFC);
   strcpy(URI.Information,"\0");
              
   if(TT4_WriteURI(&URI) == SUCCESS) {
        //  printf("[Init]. URL for quickstart data visualization: \r\n");
        //  printf((char *)UrlForNFC);
        //  printf("\r\n");
          status = MODULE_SUCCESS;
   } 
 
  return status; 
}
  
/**
  * @brief  GET MAC Address from WiFi
  * @param  char* macadd : string containing MAC address
  * @retval None
  */
 System_Status_t Get_MAC_Add (char *macadd)
{
  uint8_t macaddstart[32];
  System_Status_t status = MODULE_ERROR; // 0 success
  int i,j;
#if GPRS_INSTALL
  status = MODULE_SUCCESS;
#if 1   //0080E1B83DAC
  macadd[0]='0';
  macadd[1]='0';
  macadd[2]='8';
  macadd[3]='0';
  macadd[4]='E';
  macadd[5]='1';
  macadd[6]='B';
  macadd[7]='8';
  macadd[8]='3';
  macadd[9]='D';
  macadd[10]='A';
  macadd[11]='C';
  macadd[12]='\0';

 // if (GPRS_AT_CMD(AT_TCP_AT) != WiFi_MODULE_SUCCESS)
 // {
       // printf("[E]. Error retrieving MAC address \r\n");
        //return status;
    //}
  //ThreadAPI_Sleep(1);
#endif
#else
  if (GET_Configuration_Value("nv_wifi_macaddr",(uint32_t *)macaddstart) != WiFi_MODULE_SUCCESS)
  {
      printf("[E]. Error retrieving MAC address \r\n");  
      return status; 
  }
  else
      status = MODULE_SUCCESS;



  macaddstart[17]='\0';
    
  if (status == MODULE_SUCCESS)
  {  
        for(i=0,j=0;i<17;i++){
          if(macaddstart[i]!=':'){
            macadd[j]=macaddstart[i];
            j++;  
          } 
        }
        macadd[j]='\0';
  }
#endif
  return status;
}


/**
* @brief  Retrieve SysTick to increment counter.
* @param  None
* @retval tick value
*/
uint32_t user_currentTimeGetTick()
{
   return HAL_GetTick();
}  



/**
* @brief  Read Access Point parameters from NFC 
* @param  None
* @retval System_Status_t (MODULE_SUCCESS/MODULE_ERROR)
*/
 System_Status_t ReadWifiTokenFromNFC(void) {
  System_Status_t status = MODULE_ERROR;
  uint32_t tickstart;

  tickstart = user_currentTimeGetTick();
  while ( (user_currentTimeGetTick() - tickstart ) < APP_TIMEOUT)  {
    if(TT4_ReadWifiToken(&WIFITOKEN) == SUCCESS)
      status = MODULE_SUCCESS;
  }
      
  return status;
}


/**
* @brief  Configure Nucleo clock...... 
* @param  None
* @retval None
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

  __PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);

  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/**
 * @}
 */




/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
