



/*add by jiangdong at 2/20*/

#define AT_GPRS_STATE                           "AT+CPIN?\r"
#define AT_GPRS_STRENGTH                        "AT+CSQ\r"
#define AT_GPRS_NET_STATE                        "AT+CREG?\r"
#define AT_GPRS_STICKING_STATE                        "AT+CGATT?\r"



//#define AT_SOCKET_OPEN                        "AT+S.SOCKON=%s,%d,%s,ind\r"
#define AT_TCP_SET_APN                        "AT+CSTT=%s\r"
//#define AT_TCP_SET_APN                          "AT+CSTT="CMNET"\r"
#define AT_TCP_CREAT_NET						"AT+CIICR\r"
#define AT_TCP_CHECK_IP							"AT+CIFSR\r"
#define AT_TCP_CREAT_TCP_LINK					"AT+CIPSTART="TCP","42.96.164.52","80"\r"
#define AT_TCP_SEND								"AT+CIPSEND\r"
#define AT_TCP_CLOSE							"AT+CIPCLOSE\r"

#define HTTP_HANDLE_DATA_1 "POST /v1.1"
#if 0

/device/16326/sensor/28186/datapoints HTTP/1.1\r  \
						U-ApiKey:aac16f142f2020f4105e48b2034bd20a\r  \
						Host: api.yeelink.net\r  \ 
						Content-Length: 11\r \
						{:0}\r" 
#endif






