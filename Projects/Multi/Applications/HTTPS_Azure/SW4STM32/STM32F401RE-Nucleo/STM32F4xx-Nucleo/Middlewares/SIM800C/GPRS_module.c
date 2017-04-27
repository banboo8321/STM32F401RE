/**
 ******************************************************************************
 * @file    AT_ctrl_packet.c
 * @author  jiang dong
 * @version V1.0.0
 * @date    8-2-2017
 * @brief   This file provides packets of 2G module
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2017 OTIS</center></h2>
 *
 *
 ******************************************************************************
 */
//GPRS_AT_Cmd_Transmit()
//{}
#include "GPRS_module.h"
#include "wifi_module.h"

#define LINK_YEELINK 0
//#define SEND_CMD memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));sprintf((char*)GPRS_AT_Cmd_Buff_test,azurelink13);GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,1,100);HAL_Delay(1000)
#define SEND_CMD
//HAL_Delay(1)
char retry_count = 0;
char GPRS_AT_Cmd_Buff_test[512];
char GPRS_AT_Cmd_Buff_test_Rx[512];
UART_HandleTypeDef UartGPRSHandle_test;
#if LINK_YEELINK
const char yeelink1[]="POST /v1.1/device/16326/sensor/28186/datapoints HTTP/1.1\r\n";
const char yeelink2[]="Host: api.yeelink.NET\r\n";
const char yeelink3[]="Accept: */*\r\n";
const char yeelink4[]="U-ApiKey:aac16f142f2020f4105e48b2034bd20a\r\n";
const char yeelink5[]="Content-Length:13\r\n";
const char yeelink6[]="Content-Type: application/x-www-form-urlencoded\r\n";
const char yeelink7[]="Connection: close\r\n\r\n";
char yeelink8[]="{\"value\":0}\r\n";
const char yeelink9[]="\r\n";
const char yeelink10[]="\x1A\r\n";
#else
const char azurelink1[]="POST /devices/STM32F401JHF/messages/events?api-version=2016-02-03 HTTP/1.1\r\n";
const char azurelink2[]="iothub-to: /devices/STM32F401JHF/messages/events\r\n";
const char azurelink3[]="Authorization: SharedAccessSignature sr=STM32F401JHF.azure-devices.cn/devices/STM32F401JHF&sig=JNNuKZ%2f%2bHAXboLnjoxeteGTBu1KxH2SbGNRFlK%2fL8qY%3d&se=1490930679&skn=\r\n";
const char azurelink4[]="Accept: application/json\r\n";
const char azurelink5[]="Connection: Keep-Alive\r\n";
const char azurelink6[]="User-Agent: iothubclient/1.0.11\r\n";
const char azurelink7[]="Content-Type: application/octet-stream\r\n";
const char azurelink8[]="iothub-app-PropName: PropMsg_zu\r\n";
const char azurelink9[]="Host: STM32F401JHF.azure-devices.cn\r\n";
const char azurelink10[]="Content-Length: 168\r\n";
const char azurelink11[]="\r\n";
const char azurelink12[]="{\"id\":\"0080E1B83DAC\",\"name\":\"Nucleo-0080E1B83DAC\",\"ts\":\"2017-03-31T02:24:39.000000Z\",\"mtype\":\"ins\",\"temp\":,\"accX\":0,\"accY\":0,\"accZ\":0,\"gyrX\":0,\"gyrY\":0,\"gyrZ\":0}";//delete\r\n
const char azurelink13[]="\x1A\r\n";
const char azurelink14[]="GET /devices/STM32F401JHF/messages/devicebound?api-version=2016-02-03 HTTP/1.1\r\n";
const char azurelink15[]="Content-Length: 0\r\n";
#endif
void GPRS_UART_Configuration(uint32_t baud_rate)
{
	UartGPRSHandle_test.Instance			 = GPRS_UART;
	UartGPRSHandle_test.Init.BaudRate		 = baud_rate;
	UartGPRSHandle_test.Init.WordLength 	 = UART_WORDLENGTH_8B;
	UartGPRSHandle_test.Init.StopBits		 = UART_STOPBITS_1;
	UartGPRSHandle_test.Init.Parity 		 = UART_PARITY_NONE ;
	UartGPRSHandle_test.Init.HwFlowCtl		 = UART_HWCONTROL_RTS;//UART_HWCONTROL_NONE;
	UartGPRSHandle_test.Init.Mode			 = UART_MODE_TX_RX;
	UartGPRSHandle_test.Init.OverSampling	 = UART_OVERSAMPLING_16;
	if(HAL_UART_DeInit(&UartGPRSHandle_test) != HAL_OK)
	{
		Error_Handler();
	}
	if(HAL_UART_Init(&UartGPRSHandle_test) != HAL_OK)
	{
		Error_Handler();
	}
	printf("[Test] UartGPRS init ok!");
}
GPRS_Status_t GPRS_AT_Send(char *tp,char *string_tp,char retry_num_i,char receive_size)
{
	//char retry_num= RETRY_NUMBER;
	GPRS_Status_t AT_return_Status;
	char retry_num;
	retry_num = retry_num_i;
	AT_return_Status = GPRS_AT_Send_sub(tp,string_tp, receive_size);
	if(GPRS_MODULE_SUCCESS != AT_return_Status)
	{
#if DEBUG_PRINT
	   printf("[2G_AT]GPRS_TEST1 error,retry 1 time!\r\n" );
#endif
		if(1 == retry_num)
		{
			return GPRS_MODULE_ERROR;
		}
		GPRS_AT_RETRY_TIME;
		AT_return_Status = GPRS_AT_Send_sub(tp,string_tp, receive_size);
		if(GPRS_MODULE_SUCCESS != AT_return_Status)
		{
#if DEBUG_PRINT
	   printf("[2G_AT]GPRS_TEST1 error,retry 2 time!\r\n" );
#endif
			if(2 == retry_num)
			{
				return GPRS_MODULE_ERROR;
			}
			GPRS_AT_RETRY_TIME;
			AT_return_Status = GPRS_AT_Send_sub(tp,string_tp, receive_size);
			if(GPRS_MODULE_SUCCESS != AT_return_Status)
			{
#if DEBUG_PRINT
	   printf("[2G_AT]GPRS_TEST1 error,retry 3 time!\r\n" );
#endif
				 return GPRS_MODULE_ERROR;
			}
			else
			{
				 return GPRS_MODULE_SUCCESS;
			}
		}
		else
		{
			 return GPRS_MODULE_SUCCESS;
		}
	}
	else
	{
		 return GPRS_MODULE_SUCCESS;
	}
}
char temp= 0;
GPRS_Status_t GPRS_AT_Send_sub(char *tp,char *string_tp,char receive_size)
{

	 if(HAL_UART_Transmit(&UartGPRSHandle_test, tp, strlen(tp), 1000)!= HAL_OK)
		 {
		   Error_Handler();
		   #if DEBUG_PRINT
		   printf("HAL_UART_Transmit Error");
		   #endif
		   //return GPRS_HAL_UART_ERROR;
		 }
	 //GPRS_AT_TIME;
	 memset((uint8_t *)GPRS_AT_Cmd_Buff_test_Rx,0x00,sizeof(GPRS_AT_Cmd_Buff_test_Rx));
	 temp= HAL_UART_Receive(&UartGPRSHandle_test,(uint8_t *)GPRS_AT_Cmd_Buff_test_Rx, receive_size, RECEIVE_TIME_OUT);
	 if(temp != HAL_OK)
	 {
	#if DEBUG_PRINT
		   printf("HAL_UART_Receive Error");
	#endif
		   return GPRS_MODULE_ERROR;
	 }
	 else if(NULL == strstr((uint8_t *)GPRS_AT_Cmd_Buff_test_Rx, string_tp))
	 {
			   return GPRS_MODULE_ERROR;
	 }
	 else
	 {
		   printf("\n\r[2G_AT]GPRS_TEST1 OK!");
		   return GPRS_MODULE_SUCCESS;
	 }
	 return GPRS_MODULE_ERROR;
}

void GPRS_test(void)
{
	char request_count = 0,i ;
	//HTTP_HEADERS_HANDLE httpHeadersHandle;
	/*start socket open*/
	memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
	sprintf((char*)GPRS_AT_Cmd_Buff_test,AT_GPRS_STATE);
	GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,"READY",RETRY_NUMBER,31);
	GPRS_AT_TIME;

	memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
	sprintf((char*)GPRS_AT_Cmd_Buff_test,AT_GPRS_STRENGTH);
	GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,RETRY_NUMBER,27);
	GPRS_AT_TIME;

	memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
	sprintf((char*)GPRS_AT_Cmd_Buff_test,AT_GPRS_NET_STATE);
	GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,RETRY_NUMBER,29);
	GPRS_AT_TIME;

	memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
	sprintf((char*)GPRS_AT_Cmd_Buff_test,AT_GPRS_STICKING_STATE);
	GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,RETRY_NUMBER,29);//??
	GPRS_AT_TIME;

	memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
	sprintf((char*)GPRS_AT_Cmd_Buff_test,AT_GPRS_NET_SHUTDOWN);
	GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,RETRY_NUMBER,29);//??
	HAL_Delay(2000);
	//?
	memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
	sprintf((char*)GPRS_AT_Cmd_Buff_test,"AT+CSTT=\"CMNET\"\r\n");
	GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,RETRY_NUMBER,22);
	GPRS_AT_TIME;

	memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
	sprintf((char*)GPRS_AT_Cmd_Buff_test,AT_TCP_CREAT_NET);
	GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,RETRY_NUMBER,16);
	GPRS_AT_TIME;

	memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
	sprintf((char*)GPRS_AT_Cmd_Buff_test,AT_TCP_CHECK_IP);
	GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,1,12);//no OK
	//
	GPRS_AT_TIME;
#if 0
	memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
	sprintf((char*)GPRS_AT_Cmd_Buff_test,AT_TCP_SEND_SET);
	GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,RETRY_NUMBER,16);
	GPRS_AT_TIME;
#endif
	memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
	sprintf((char*)GPRS_AT_Cmd_Buff_test,AT_TCP_SSL_OPEN);
	GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,RETRY_NUMBER,16);
	GPRS_AT_TIME;
	HAL_Delay(500);

	memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
	sprintf((char*)GPRS_AT_Cmd_Buff_test,AT_TCP_CREAT_TCP_AZURE_LINK);//yeelink"\"42.96.164.52\"","\"80\""
	GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,RETRY_NUMBER,100);
	/*end socket open*/
	HAL_Delay(5000);
	while(1)
		{
		/*start socket write*/
		memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
		sprintf((char*)GPRS_AT_Cmd_Buff_test,AT_TCP_SEND);
		GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,">",RETRY_NUMBER,1);
		/*end socket write*/
		HAL_Delay(500);
#if 0
		memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
		sprintf((char*)GPRS_AT_Cmd_Buff_test,HTTP_HANDLE_DATA_POST);
		GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,1,100);

		//sprintf((char*)GPRS_AT_Cmd_Buff_test,"%d\r\n",26);
		memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
		GPRS_AT_Cmd_Buff_test[0]= 0x1A;
		GPRS_AT_Cmd_Buff_test[1]= 0x0A;
		GPRS_AT_Cmd_Buff_test[2]= 0x0D;
		if(HAL_UART_Transmit(&UartGPRSHandle_test, (uint8_t *)GPRS_AT_Cmd_Buff_test, strlen((char*)GPRS_AT_Cmd_Buff_test), 1000)!= HAL_OK)
		{
		Error_Handler();
		#if DEBUG_PRINT
		printf("HAL_UART_Transmit Error");
		#endif
		//return GPRS_MODULE_ERROR;
		}
#else
		//yeelink1
#if LINK_YEELINK
		memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
		sprintf((char*)GPRS_AT_Cmd_Buff_test,yeelink1);
		GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,1,100);
		memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
		sprintf((char*)GPRS_AT_Cmd_Buff_test,yeelink2);
		GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,1,100);
		memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
		sprintf((char*)GPRS_AT_Cmd_Buff_test,yeelink3);
		GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,1,100);
		memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
		sprintf((char*)GPRS_AT_Cmd_Buff_test,yeelink4);
		GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,1,100);
		memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
		sprintf((char*)GPRS_AT_Cmd_Buff_test,yeelink5);
		GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,1,100);
		memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
		sprintf((char*)GPRS_AT_Cmd_Buff_test,yeelink6);
		GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,1,100);
		memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
		sprintf((char*)GPRS_AT_Cmd_Buff_test,yeelink7);
		GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,1,100);
		memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
		sprintf((char*)GPRS_AT_Cmd_Buff_test,yeelink8);
		GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,1,100);
		memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
		sprintf((char*)GPRS_AT_Cmd_Buff_test,yeelink9);
		GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,1,100);
		memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
		sprintf((char*)GPRS_AT_Cmd_Buff_test,yeelink10);
		GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,1,100);
#else
		memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
		sprintf((char*)GPRS_AT_Cmd_Buff_test,azurelink1);
		GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,1,100);
		SEND_CMD;
		memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
		for(i=0;i<request_count;request_count++)
		{
		//get_header(HTTP_HEADERS_HANDLE httpHeadersHandle,char index, char* header);
		}
		sprintf((char*)GPRS_AT_Cmd_Buff_test,azurelink2);
		GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,1,100);
		SEND_CMD;
		memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
		sprintf((char*)GPRS_AT_Cmd_Buff_test,azurelink3);
		GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,1,100);
		SEND_CMD;
		memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
		sprintf((char*)GPRS_AT_Cmd_Buff_test,azurelink4);
		GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,1,100);
		SEND_CMD;
		memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
		sprintf((char*)GPRS_AT_Cmd_Buff_test,azurelink5);
		GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,1,100);
		SEND_CMD;
		memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
		sprintf((char*)GPRS_AT_Cmd_Buff_test,azurelink6);
		GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,1,100);
		SEND_CMD;
		memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
		sprintf((char*)GPRS_AT_Cmd_Buff_test,azurelink7);
		GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,1,100);
		SEND_CMD;
		memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
		sprintf((char*)GPRS_AT_Cmd_Buff_test,azurelink8);
		GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,1,100);
		SEND_CMD;
		memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
		sprintf((char*)GPRS_AT_Cmd_Buff_test,azurelink9);
		GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,1,100);
		SEND_CMD;
		memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
		sprintf((char*)GPRS_AT_Cmd_Buff_test,azurelink10);
		GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,1,100);
		SEND_CMD;
		sprintf((char*)GPRS_AT_Cmd_Buff_test,azurelink11);
		GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,1,100);
		SEND_CMD;
		memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
		sprintf((char*)GPRS_AT_Cmd_Buff_test,azurelink12);
		GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,1,100);
		memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
		sprintf((char*)GPRS_AT_Cmd_Buff_test,azurelink13);
		GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,1,100);
#endif
#endif
		GPRS_AT_TIME;
		HAL_Delay(15000);
		printf("\n\r[2G_AT]Send data to azure OK!");
		/*start socket write*/
		memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
		sprintf((char*)GPRS_AT_Cmd_Buff_test,AT_TCP_SEND);
		GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,">",RETRY_NUMBER,1);
		/*end socket write*/
		HAL_Delay(2500);

		memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
		sprintf((char*)GPRS_AT_Cmd_Buff_test,azurelink14);
		GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,1,100);
		SEND_CMD;
		memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
		sprintf((char*)GPRS_AT_Cmd_Buff_test,azurelink6);
		GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,1,100);
		SEND_CMD;
		memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
		sprintf((char*)GPRS_AT_Cmd_Buff_test,azurelink3);
		GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,1,100);
		SEND_CMD;
		sprintf((char*)GPRS_AT_Cmd_Buff_test,azurelink9);
		GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,1,100);
		SEND_CMD;
		memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
		sprintf((char*)GPRS_AT_Cmd_Buff_test,azurelink15);
		GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,1,100);
		memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
		sprintf((char*)GPRS_AT_Cmd_Buff_test,azurelink13);
		GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,1,100);
		GPRS_AT_TIME;
		HAL_Delay(5000);
		}
	GPRS_AT_REQUEST_RECEIVE_TIME;
	memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
	sprintf((char*)GPRS_AT_Cmd_Buff_test,AT_TCP_CLOSE);
	//GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,1,2);
}
GPRS_Status_t GPRS_module_init(void)
{
	GPRS_Status_t status = GPRS_MODULE_SUCCESS;
	memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
	sprintf((char*)GPRS_AT_Cmd_Buff_test,AT_GPRS_STATE);
	status = GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,"READY",RETRY_NUMBER,31);
	if(status == GPRS_MODULE_SUCCESS)
	  {
	    status = USART_Receive_AT_Resp( );
	    if(status != GPRS_MODULE_SUCCESS) return status;
	  }
#if 0
	memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
	sprintf((char*)GPRS_AT_Cmd_Buff_test,AT_GPRS_STRENGTH);
	status = GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,RETRY_NUMBER,27);
	if(status == GPRS_MODULE_SUCCESS)
		  {
		    status = USART_Receive_AT_Resp( );
		    if(status != GPRS_MODULE_SUCCESS) return status;
		  }
	memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
	sprintf((char*)GPRS_AT_Cmd_Buff_test,AT_GPRS_NET_STATE);
	status = GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,RETRY_NUMBER,29);
	if(status == GPRS_MODULE_SUCCESS)
		  {
		    status = USART_Receive_AT_Resp( );
		    if(status != GPRS_MODULE_SUCCESS) return status;
		  }
	memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
	sprintf((char*)GPRS_AT_Cmd_Buff_test,AT_GPRS_STICKING_STATE);
	status = GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,RETRY_NUMBER,29);//??
	if(status == GPRS_MODULE_SUCCESS)
		  {
		    status = USART_Receive_AT_Resp( );
		    if(status != GPRS_MODULE_SUCCESS) return status;
		  }
	//?
	//add AT+CIPSHUT
	memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
	sprintf((char*)GPRS_AT_Cmd_Buff_test,AT_GPRS_NET_SHUTDOWN);
	status = GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,RETRY_NUMBER,9);
		if(status == GPRS_MODULE_SUCCESS)
			  {
			    status = USART_Receive_AT_Resp( );
			    HAL_Delay(2000);
			    if(status != GPRS_MODULE_SUCCESS) return status;
			  }
#endif
	return status;
}
GPRS_Status_t GPRS_socket_open(uint8_t *curr_hostname,int curr_port_number,uint8_t *curr_protocol)
{
	char *port_string;
	/*start socket open*/
#if 0
	memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
	sprintf((char*)GPRS_AT_Cmd_Buff_test,"AT+CSTT=%s\r\n","\"CMNET\"");
	GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,RETRY_NUMBER,22);

	memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
	sprintf((char*)GPRS_AT_Cmd_Buff_test,AT_TCP_CREAT_NET);
	GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,RETRY_NUMBER,16);

	memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
	sprintf((char*)GPRS_AT_Cmd_Buff_test,AT_TCP_CHECK_IP);
	GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,1,12);//no OK
#endif	//
	/*curr_protocol s:TLS t:TCP*/
	itoa(curr_port_number,port_string,10);
	memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
	sprintf((char*)GPRS_AT_Cmd_Buff_test,"AT+CIPSTART=%s,\"%s\",\"%s\"\r\n","\"TCP\"",curr_hostname,port_string);
	if(GPRS_MODULE_SUCCESS == GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_CONNECT_OK,RETRY_NUMBER,100))
	{

		return GPRS_MODULE_SUCCESS;
	}
	else
	{
		return GPRS_MODULE_ERROR;
	}
	/*end socket open*/
}
GPRS_Status_t GPRS_socket_write(void)
{
	/*start socket write*/
	memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
	sprintf((char*)GPRS_AT_Cmd_Buff_test,AT_TCP_SEND);
	if(GPRS_MODULE_SUCCESS == GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,">",2,10))
	{

		return GPRS_MODULE_SUCCESS;
	}
	else
	{
		return GPRS_MODULE_ERROR;
	}
	/*end socket write*/
}
GPRS_Status_t GPRS_socket_write_detail(uint8_t *curr_data, uint16_t DataLength)
{
	/*start socket write*/
	memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
	sprintf((char*)GPRS_AT_Cmd_Buff_test,curr_data);
	GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,curr_data,1,DataLength);
	
	memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
	GPRS_AT_Cmd_Buff_test[0]= 0x1A;
	GPRS_AT_Cmd_Buff_test[1]= 0x0A;
	GPRS_AT_Cmd_Buff_test[2]= 0x0D;
	if(GPRS_MODULE_SUCCESS == GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,"SEND OK",1,7))
	{

		return GPRS_MODULE_SUCCESS;
	}
	else
	{
		return GPRS_MODULE_ERROR;
	}
	/*end socket write*/
}
GPRS_Status_t GPRS_socket_close(void)
{
	memset(GPRS_AT_Cmd_Buff_test,0x00,sizeof(GPRS_AT_Cmd_Buff_test));
	sprintf((char*)GPRS_AT_Cmd_Buff_test,AT_TCP_CLOSE);
	if(GPRS_MODULE_SUCCESS == GPRS_AT_Send((char*)GPRS_AT_Cmd_Buff_test,AT_RETURN_OK,1,2))
	{

		return GPRS_MODULE_SUCCESS;
	}
	else
	{
		return GPRS_MODULE_ERROR;
	}
}

