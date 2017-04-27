
/*GPRS_module.h*/

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "stm32f4xx_I2C.h"
#include "stm32f4xx_periph_conf.h"

#include <time.h>   // time functions
#include <stdint.h>
#include <string.h>
#define GPRS_INSTALL 1 // //1 INSTALL  0:UNINSTALL
#define TEST_Program 0
#define GPRS_SSL_ENABLE 1
#define TEST_SSL_SCEVER_ENABLE 0
#define SENSOR_INSTALL 0
#define DUMMY_SENSOR_INSTALL_DELAY_ENABLE 1
#define AT_TCP_RECEIVE_CTRL_ENABLE 1
#define GPRS_TEST 0
#define GPRS_TEST_TIME_SEND 0
#define DUMMY_ADVANCED_CONNECT_ENABEL 1

#define RETRY_NUMBER 		1
#define AT_GPRS_STATE                           "AT+CPIN?\r\n"
#define AT_GPRS_STRENGTH                        "AT+CSQ\r\n"
#define AT_GPRS_NET_STATE                        "AT+CREG?\r\n"
#define AT_GPRS_STICKING_STATE                        "AT+CGATT?\r\n"
#define AT_GPRS_NET_SHUTDOWN 					"AT+CIPSHUT\r\n"
#define AT_GPRS_SET_APN_EG1            "AT+CSTT=%s\r\n","\"CMNET\""

#define AT_TCP_FLOW_CTRL                        "AT+IFC=2,2\r\n"
#define AT_TCP_RECEIVE_SET                     "AT+CIPRXGET=1\r\n"
#define AT_TCP_RECEIVE_CTRL                     "AT+CIPRXGET=2,1460\r\n"
#define AT_TCP_RECEIVE_FLG                      "+CIPRXGET: 1"
//#define AT_SOCKET_WRITE                         "AT+S.SOCKW=%d,%d\r"
//#define AT_SOCKET_OPEN                        "AT+S.SOCKON=%s,%d,%s,ind\r"
#define AT_TCP_SET_APN                        "AT+CSTT=%s\r\n"
//#define AT_TCP_SET_APN                          "AT+CSTT="CMNET"\r"
#define AT_TCP_CREAT_NET						"AT+CIICR\r\n"
#define AT_TCP_CHECK_IP							"AT+CIFSR\r\n"
#define AT_TCP_SEND_SET							"AT+CIPATS=1,3\r\n"
#define AT_TCP_SSL_CHECK						"AT+CIPSSL?\r\n"
#define AT_TCP_SSL_OPEN							"AT+CIPSSL=1\r\n"
#define AT_TCP_SSL_CLOSE						"AT+CIPSSL=0\r\n"
#define AT_TCP_CREAT_TCP_YEELINK_LINK					"AT+CIPSTART=\"TCP\",\"api.yeelink.net\",\"80\"\r\n"
#define AT_TCP_CREAT_TCP_AZURE_LINK				"AT+CIPSTART=\"TCP\",\"STM32F401JHF.azure-devices.cn\",\"443\"\r\n"
#define AT_TCP_SEND								"AT+CIPSEND\r\n"
#define AT_TCP_SEND_LENGTH						"AT+CIPSEND=%d\r\n"
#define AT_TCP_CLOSE							"AT+CIPCLOSE\r\n"
#define AT_TCP_AT							"AT\r\n"
#define HTTP_HANDLE_DATA_GET "GET /v1.1/device/16326/sensor/28186/datapoints HTTP/1.1\r\n  \
								Host: api.yeelink.net\r\n  \
								Content-Length: 13\r\n \
								U-ApiKey:aac16f142f2020f4105e48b2034bd20a\r\n  \
								\r\n"
#define HTTP_HANDLE_DATA_POST "POST /v1.1/device/16326/sensor/28186/datapoints HTTP/1.1\r\n  \
								U-ApiKey:aac16f142f2020f4105e48b2034bd20a\r\n  \
								Host: api.yeelink.net\r\n  \
								Content-Length: 13\r\n \
								\r\n				\
								{\"value\":0}\r\n"
#define HTTP_HANDLE_WS_DATA_POST 	"POST /devices/STM32F401/messages/events?api-version=2016-02-03 HTTP/1.1\r\n  \
								Authorization: SharedAccessSignature sr=OtisTest.azure-devices.cn/devices/STM32F401&sig=G99tEdHDnyqn%2bnpSG4mWqUOcf27ZyQnXFTJXVJjWPt8%3d&se=1488724650&skn=\r\n  \
								Content-Type: application/json \r\n  \
								Connection: Keep-Alive \r\n  \
								User-Agent: iothubclient/1.0.11 \r\n  \
								Content-Type: application/octet-stream \r\n  \
								iothub-app-PropName: PropMsg_zu \r\n  \
								Host: OtisTest.azure-devices.cn \r\n  \
								Content-Length: 112 \r\n  \
								\r\n  \
								{\"id\":\"0080E1B83DAC\",\"name\":\"Nucleo-0080E1B83DAC\",\"ts\":\"2017-03-05T09:38:00.000000Z\",\"mtype\":\"ins\",\"temp\":26.50} \r\n  "
#define HTTP_HANDLE_WS_DATA_GET 	"GET /devices/STM32F401/messages/devicebound?api-version=2016-02-03 HTTP/1.1\r\n  \
								User-Agent: iothubclient/1.0.11 \r\n  \
								Authorization:  SharedAccessSignature sr=OtisTest.azure-devices.cn/devices/STM32F401&sig=V%2fDnzs2l5OL8RCD9Ef5P%2fK18jHJQObTkdtBqhGTkayQ%3d&se=1487905582&skn=\r\n  \
								Host: OtisTest.azure-devices.cn \r\n  \
								Content-Length: 112 \r\n  "

#define GPRS_UART                     USART1
#define GPRS_AT_TIME
//HAL_Delay(1)
#define GPRS_AT_REQUEST_RECEIVE_TIME HAL_Delay(10000)
#define GPRS_AT_RETRY_TIME HAL_Delay(1000)
#define RECEIVE_SIZE 31
#define RECEIVE_TIME_OUT 500
#define AT_RETURN_OK "OK"
#define AT_RETURN_CONNECT_OK "CONNECT OK"
#define AT_RETURN_ERROR "ERROR"
typedef enum
{
  GPRS_MODULE_SUCCESS           = 0,
  GPRS_MODULE_ERROR           = 1
} GPRS_Status_t;

/* Extern functions ---------------------------------------------------------*/
extern void GPRS_UART_Configuration(uint32_t baud_rate);
extern GPRS_Status_t GPRS_AT_Send(char *tp,char *string_tp,char retry_num_i,char receive_size);
extern GPRS_Status_t GPRS_AT_Send_sub(char *tp,char *string_tp,char receive_size);
extern void GPRS_test(void);
GPRS_Status_t GPRS_socket_open(uint8_t *curr_hostname,int curr_port_number,uint8_t *curr_protocol);
extern signed char gprs_send(unsigned char u8Gid,unsigned char u8Opcode,unsigned char *pu8CtrlBuf,unsigned short u16CtrlBufSize,
			   unsigned char *pu8DataBuf,unsigned short u16DataSize, unsigned short u16DataOffset);
