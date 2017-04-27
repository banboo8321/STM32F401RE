// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <stdio.h>
#include <ctype.h>
#include "azure_c_shared_utility/httpapi.h"
//#include "azure_c_shared_utility/httpapiex.h"
#include "azure_c_shared_utility/httpheaders.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/strings.h"
#include "azure_c_shared_utility/threadapi.h"
//#include "azure_c_shared_utility/x509_schannel.h"

// ST 
#include "InternetInterface.h"
#include "AzureIOTSDKConfig.h"

#include "GPRS_module.h"
#define DELAY_API_THREAD ThreadAPI_Sleep(10)
//#include "ThreadAPI.h"


/** @addtogroup MIDDLEWARES
* @{
*/ 


/** @addtogroup AZURE_SDK
* @{
*/ 


/** @defgroup  AZURE_ADAPTER_HTTP
  * @brief Wi-Fi User API modules
  * @{
  */

#define CHAR_COUNT(A)   (sizeof(A) - 1)

DEFINE_ENUM_STRINGS(HTTPAPI_RESULT, HTTPAPI_RESULT_VALUES)
#define HTTP_LINE_END   "\r\n"


HTTPAPI_RESULT HTTPAPI_Init(void)
{
    LogInfo("HTTPAPI_Init::Start\r\n");
    time_t ctTime;
    ctTime = get_time(NULL);
    HTTPAPI_RESULT result;
    LogInfo("HTTAPI_Init::Time is now (UTC) %s\r\n", ctime(&ctTime));

    if (InternetInterfaceStart()!=INTERNETINTERFACERESULT_SUCCESS)
    {
        LogError("HTTPAPI_Init::Error with connecting.\r\n");
        result = HTTPAPI_INIT_FAILED;
    }
    else
    {
        LogInfo("HTTAPI_Init::Ethernet interface was connected (brought up)!\r\n");
        result = HTTPAPI_OK;
    }

    LogInfo("HTTPAPI_Init::End\r\n");

    return result;
}

void HTTPAPI_Deinit(void)
{
    
}

HTTP_HANDLE HTTPAPI_CreateConnection(const char* hostName)
{
    LogInfo("HTTPAPI_CreateConnection::Start\r\n");
    HTTP_HANDLE_DATA* handle = NULL;

    if (hostName)
    {
        LogInfo("HTTPAPI_CreateConnection::Connecting to %s\r\n", hostName);
        handle = (HTTP_HANDLE_DATA*)malloc(sizeof(HTTP_HANDLE_DATA));
        if (strcpy_s(handle->host, MAX_HOSTNAME, hostName) != 0)
        {
            LogError("HTTPAPI_CreateConnection::Could not strcpy_s\r\n");
            free(handle);
            handle = NULL;
        }
        else
        {
            handle->certificate = NULL;
        }
    }
    else
    {
        LogInfo("HTTPAPI_CreateConnection:: null hostName parameter\r\n");
    }
    LogInfo("HTTPAPI_CreateConnection::End\r\n");

    return (HTTP_HANDLE)handle;
}

void HTTPAPI_CloseConnection(HTTP_HANDLE handle)
{
    HTTP_HANDLE_DATA* h = (HTTP_HANDLE_DATA*)handle;

    if (h)
    {
        LogInfo("HTTPAPI_CloseConnection to %s\r\n", h->host);
        if (InternetInterfaceSocketClose(handle))
        {
            LogInfo("HTTPAPI_CloseConnection  h->con.close(); to %s\r\n", h->host);
            InternetInterfaceSocketClose(handle);
        }
        if (h->certificate)
        {
            free(h->certificate);
        }
        LogInfo("HTTPAPI_CloseConnection (delete h) to %s\r\n", h->host);
        free(h);
    }
}

static int readLine(HTTP_HANDLE handle,char* buf, const size_t size)
{
    // reads until \r\n is encountered. writes in buf all the characters
    // read until \r\n and returns the number of characters in the buffer.
    char* p = buf;
    char  c;
    size_t numberReadBytes;
    if (InternetInterfaceSocketReceive(handle,&c, 1,&numberReadBytes)!=SOCKETOPERATIONRESULT_SUCCESS || numberReadBytes==0)
        return -1;

    while (c != '\r') {
        if ((p - buf + 1) >= (int)size)
            return -1;
        *p++ = c;
        if (InternetInterfaceSocketReceive(handle,&c, 1,&numberReadBytes)!=SOCKETOPERATIONRESULT_SUCCESS || numberReadBytes==0)
            return -1;
    }
    *p = 0;
    if (InternetInterfaceSocketReceive(handle,&c, 1,&numberReadBytes) !=SOCKETOPERATIONRESULT_SUCCESS || numberReadBytes==0 || c != '\n') // skip \n
        return -1;
    return p - buf;
}

static int readChunk(HTTP_HANDLE handle,char* buf, size_t size)
{
    size_t cur, offset;

    // read content with specified length, even if it is received
    // only in chunks due to fragmentation in the networking layer.
    // returns -1 in case of error.
    offset = 0;
    while (size > 0)
    {
      if (InternetInterfaceSocketReceive(handle,buf + offset, size,&cur)!=SOCKETOPERATIONRESULT_SUCCESS || cur==0)
      {
        return offset;
      }
      // read cur bytes (might be less than requested)
      size -= cur;
      offset += cur;
    }

    return offset;
}

static int skipN(HTTP_HANDLE handle,size_t n, char* buf, size_t size)
{
    size_t org = n;
    // read and abandon response content with specified length
    // returns -1 in case of error.
    while (n > size)
    {
        if (readChunk(handle,(char*)buf, size) < 0)
            return -1;

        n -= size;
    }

    if (readChunk(handle,(char*)buf, n) < 0)
        return -1;

    return org;
}

#define HTTP_ANSWER_PROTOCOL "HTTP/1.1 "
//Note: This function assumes that "Host:" and "Content-Length:" headers are setup
//      by the caller of HTTPAPI_ExecuteRequest() (which is true for httptransport.c).
HTTPAPI_RESULT HTTPAPI_ExecuteRequest(HTTP_HANDLE handle, HTTPAPI_REQUEST_TYPE requestType, const char* relativePath,
    HTTP_HEADERS_HANDLE httpHeadersHandle, const unsigned char* content,
    size_t contentLength, unsigned int* statusCode,
    HTTP_HEADERS_HANDLE responseHeadersHandle, BUFFER_HANDLE responseContent)
{
	static char first_time_enter_flg = 0;
    LogInfo("HTTPAPI_ExecuteRequest::Start\r\n");
#if TEST_Program
	printf("[Test_P]: HTTPAPI_ExecuteRequest::Start\r\n");
#endif
    HTTPAPI_RESULT result=HTTPAPI_OK;
    size_t  headersCount;
    char    buf[TEMP_BUFFER_SIZE];
    int     ret;
    size_t  bodyLength = 0;
    bool    chunked = false;
    const unsigned char* receivedContent;

    const char* method = (requestType == HTTPAPI_REQUEST_GET) ? "GET"
        : (requestType == HTTPAPI_REQUEST_POST) ? "POST"
        : (requestType == HTTPAPI_REQUEST_PUT) ? "PUT"
        : (requestType == HTTPAPI_REQUEST_DELETE) ? "DELETE"
        : (requestType == HTTPAPI_REQUEST_PATCH) ? "PATCH"
        : NULL;

    if (handle == NULL ||
        relativePath == NULL ||
        httpHeadersHandle == NULL ||
        method == NULL ||
        HTTPHeaders_GetHeaderCount(httpHeadersHandle, &headersCount) != HTTP_HEADERS_OK)
    {
        result = HTTPAPI_INVALID_ARG;
        LogError("(result = %s)\r\n", ENUM_TO_STRING(HTTPAPI_RESULT, result));
        LogInfo("HTTPAPI_ExecuteRequest::End=%d\r\n", result);
#if TEST_Program
	printf("[Test_P]:HTTPAPI_ExecuteRequest::End1=%d\r\n", result);
#endif
        return result;
    }

    HTTP_HANDLE_DATA* httpHandle = (HTTP_HANDLE_DATA*)handle;

    if (InternetInterfaceSocketGetConnectionStatus(handle)!=SOCKETSTATUS_CONNECTED)
    {
        // Load the certificate
        if ((httpHandle->certificate != NULL) &&
			(!InternetInterfaceLoadCertificate((const unsigned char*)httpHandle->certificate, strlen(httpHandle->certificate) + 1)))
        {
            result = HTTPAPI_ERROR;
            LogError("Could not load certificate (result = %s)\r\n", ENUM_TO_STRING(HTTPAPI_RESULT, result));
            LogInfo("HTTPAPI_ExecuteRequest::End=%d\r\n", result);
#if TEST_Program
	printf("[Test_P]:HTTPAPI_ExecuteRequest::End2=%d\r\n", result);
#endif
            return result;
        }

        // Make the connection
#if 0//GPRS_INSTALL

        if(0 == first_time_enter_flg)
        	{
        		first_time_enter_flg = 1;
				if (InternetInterfaceSocketOpen(handle,httpHandle->host, TCP_SERVERPORT,SOCKETCONNECTIONTYPE_TLS) !=SOCKETSTATUS_CONNECTED)
				{
#if 0
					result = HTTPAPI_ERROR;
					LogError("Could not connect (result = %s)\r\n", ENUM_TO_STRING(HTTPAPI_RESULT, result));
					LogInfo("HTTPAPI_ExecuteRequest::End=%d\r\n", result);

					ThreadAPI_Sleep(300);

					return result;
#endif
					ThreadAPI_Sleep(3000);
				}
				else
				{
					LogInfo("HTTPAPI_CreateConnection::Connection to %s successful!\r\n", httpHandle->host);
				}
        	}
#else
#if TEST_SSL_SCEVER_ENABLE
#define TCP_SERVERPOR_TEST                   8433
       // httpHandle->host = "192.168.115.195";//:8433.
        if (InternetInterfaceSocketOpen(handle,"192.168.114.155", TCP_SERVERPOR_TEST,SOCKETCONNECTIONTYPE_TLS) !=SOCKETSTATUS_CONNECTED)
#else
#define        	TCP_SERVERPOR 443
#if TEST_Program
	printf("[Test_P]:InternetInterfaceSocketOpen");
#endif
        		if (InternetInterfaceSocketOpen(handle,httpHandle->host, TCP_SERVERPOR,SOCKETCONNECTIONTYPE_TLS) !=SOCKETSTATUS_CONNECTED)
#endif
        		{
					result = HTTPAPI_ERROR;
					LogError("Could not connect (result = %s)\r\n", ENUM_TO_STRING(HTTPAPI_RESULT, result));
					LogInfo("HTTPAPI_ExecuteRequest::End=%d\r\n", result);
#if TEST_Program
	printf("[Test_P]:HTTPAPI_ExecuteRequest::End3=%d\r\n", result);
#endif
					ThreadAPI_Sleep(300);

					return result;
				}
				else
				{
					LogInfo("HTTPAPI_CreateConnection::Connection to %s successful!\r\n", httpHandle->host);
#if TEST_Program
	printf("[Test_P]:HTTPAPI_CreateConnection::Connection to %s successful!\r\n\r\n",  httpHandle->host);
#endif
				}
#if GPRS_INSTALL
      // ThreadAPI_Sleep(3000);//ThreadAPI_Sleep(1500);
#endif

#endif
    }
    	//add flg to can send
    //Send request
    if ((ret = snprintf(buf, sizeof(buf), "%s %s HTTP/1.1\r\n", method, relativePath)) < 0
        || ret >= sizeof(buf))
    {
        result = HTTPAPI_STRING_PROCESSING_ERROR;
        LogError("(result = %s)\r\n", ENUM_TO_STRING(HTTPAPI_RESULT, result));
        LogInfo("HTTPAPI_ExecuteRequest::End=%d\r\n", result);
#if TEST_Program
	printf("[Test_P]:HTTPAPI_ExecuteRequest::End4=%d\r\n", result);
#endif
        return result;
   }
    LogInfo("HTTPAPI_ExecuteRequest::Sending=%s\r\n",buf);
#if TEST_Program
	printf("[Test_P]:HTTPAPI_ExecuteRequest::Sending=%s\r\n", buf);
#endif
#if  GPRS_INSTALL
    //send AT+SEND  ????
    if (InternetInterfaceSocketSend(handle,AT_TCP_SEND,12)!=SOCKETOPERATIONRESULT_SUCCESS)
    {
        result = HTTPAPI_SEND_REQUEST_FAILED;
        LogError("(result = %s)\r\n", ENUM_TO_STRING(HTTPAPI_RESULT, result));
        LogInfo("HTTPAPI_ExecuteRequest::End=%d\r\n", result);
#if TEST_Program
	printf("[Test_P]:HTTPAPI_ExecuteRequest::End11=%d\r\n", result);
#endif
        return result;
    }
#else
#endif
#if GPRS_INSTALL
        ThreadAPI_Sleep(1000);//ThreadAPI_Sleep(1500);
#endif
    /*POST /devices/STM32F401/messages/events?api-version=2016-02-03 HTTP/1.1*/
    if (InternetInterfaceSocketSend(handle,buf, strlen(buf))!=SOCKETOPERATIONRESULT_SUCCESS)
    {
        result = HTTPAPI_SEND_REQUEST_FAILED;
        LogError("(result = %s)\r\n", ENUM_TO_STRING(HTTPAPI_RESULT, result));
        LogInfo("HTTPAPI_ExecuteRequest::End=%d\r\n", result);
#if TEST_Program
	printf("[Test_P]:HTTPAPI_ExecuteRequest::End5=%d\r\n", result);
#endif
        return result;
    }
#if GPRS_INSTALL
    DELAY_API_THREAD;
#endif
    //Send default headers
    for (size_t i = 0; i < headersCount; i++)
    {
        char* header;

        if (HTTPHeaders_GetHeader(httpHeadersHandle, i, &header) != HTTP_HEADERS_OK)
        {
            result = HTTPAPI_HTTP_HEADERS_FAILED;
            LogError("(result = %s)\r\n", ENUM_TO_STRING(HTTPAPI_RESULT, result));
            LogInfo("HTTPAPI_ExecuteRequest::End=%d\r\n", result);
#if TEST_Program
	printf("[Test_P]:HTTPAPI_ExecuteRequest::End6=%d\r\n", result);
#endif
            return result;
        }
        LogInfo("HTTPAPI_ExecuteRequest::Sending=%s\r\n", header);
#if TEST_Program
	printf("[Test_P]:HTTPheader:%s\r\n", header);
#endif
#if GPRS_INSTALL
	if(strstr(header,"Content-Length") == NULL)
		strcat(header,"\r\n");
	else
		strcat(header,"\r\n\r\n");
#endif
        if (InternetInterfaceSocketSend(handle,header, strlen(header))!=SOCKETOPERATIONRESULT_SUCCESS)
        {
            result = HTTPAPI_SEND_REQUEST_FAILED;
            LogError("(result = %s)\r\n", ENUM_TO_STRING(HTTPAPI_RESULT, result));
            free(header);
            LogInfo("HTTPAPI_ExecuteRequest::End=%d\r\n", result);
#if TEST_Program
	printf("[Test_P]:HTTPAPI_ExecuteRequest::End7=%d\r\n", result);
#endif
            return result;
        }
#if GPRS_INSTALL
    DELAY_API_THREAD;
#endif
#if !GPRS_INSTALL

        if (InternetInterfaceSocketSend(handle,HTTP_LINE_END, strlen(HTTP_LINE_END))!=SOCKETOPERATIONRESULT_SUCCESS)
        {
            result = HTTPAPI_SEND_REQUEST_FAILED;
            LogError("(result = %s)\r\n", ENUM_TO_STRING(HTTPAPI_RESULT, result));
            free(header);
            LogInfo("HTTPAPI_ExecuteRequest::End=%d\r\n", result);
#if TEST_Program
	printf("[Test_P]:HTTPAPI_ExecuteRequest::End8=%d\r\n", result);
#endif
            return result;
        }
        ThreadAPI_Sleep(200);//ThreadAPI_Sleep(1500);
#endif
        free(header);
    }
#if !GPRS_INSTALL
    //Close headers
    if (InternetInterfaceSocketSend(handle,HTTP_LINE_END, strlen(HTTP_LINE_END))!=SOCKETOPERATIONRESULT_SUCCESS)
    {
        result = HTTPAPI_SEND_REQUEST_FAILED;
        LogError("(result = %s)\r\n", ENUM_TO_STRING(HTTPAPI_RESULT, result));
        LogInfo("HTTPAPI_ExecuteRequest::End=%d\r\n", result);
#if TEST_Program
	printf("[Test_P]:HTTPAPI_ExecuteRequest::End9=%d\r\n", result);
#endif
        return result;
    }
#endif
    //Send data (if available)
    if (content && contentLength > 0)
    {
        LogInfo("HTTPAPI_ExecuteRequest::Sending data=%s\r\n", content);
        //printf(content);
        if (InternetInterfaceSocketSend(handle,(char*)content, contentLength) <= 0)
        {
            result = HTTPAPI_SEND_REQUEST_FAILED;
            LogError("(result = %s)\r\n", ENUM_TO_STRING(HTTPAPI_RESULT, result));
            LogInfo("HTTPAPI_ExecuteRequest::End=%d\r\n", result);
#if TEST_Program
	printf("[Test_P]:HTTPAPI_ExecuteRequest::End10=%d\r\n", result);
#endif
            return result;
      }
#if GPRS_INSTALL
    DELAY_API_THREAD;
#endif
    }
#if  !GPRS_TEST_TIME_SEND//GPRS_INSTALL
    //send 0x1a  ????
    if (InternetInterfaceSocketSend(handle,"0x1a",4)!=SOCKETOPERATIONRESULT_SUCCESS)
    {
        result = HTTPAPI_SEND_REQUEST_FAILED;
        LogError("(result = %s)\r\n", ENUM_TO_STRING(HTTPAPI_RESULT, result));
        LogInfo("HTTPAPI_ExecuteRequest::End=%d\r\n", result);
#if TEST_Program
	printf("[Test_P]:HTTPAPI_ExecuteRequest::End11=%d\r\n", result);
#endif
        return result;
    }
#else
#endif
//mark
#if GPRS_INSTALL
    // ThreadAPI_Sleep(200);//ThreadAPI_Sleep(1500);
#endif
    //Receive response
    if (readLine(handle,buf, sizeof(buf)) <= 0)
    {
        result = HTTPAPI_READ_DATA_FAILED;
        LogError("(result = %s)\r\n", ENUM_TO_STRING(HTTPAPI_RESULT, result));
        LogInfo("HTTPAPI_ExecuteRequest::End=%d\r\n", result);
        return result;
    }

    //Parse HTTP response
    //FIXME: AGGIUNTO
    char* presp = strstr(buf,HTTP_ANSWER_PROTOCOL);
    
    if(presp==0)
    {
        LogInfo("HTTPAPI_ExecuteRequest:: ANSWER FORMAT UNCORRECT\r\n");
        return result;   
    }
    
    presp += strlen(HTTP_ANSWER_PROTOCOL);
    //FIXME: AGGIUNTO

    //if (sscanf(buf, "HTTP/%*d.%*d %d %*[^\r\n]", &ret) != 1)

    if(sscanf(presp,"%d",&ret)<1)
    {
        //Cannot match string, error
        LogInfo("HTTPAPI_ExecuteRequest::Not a correct HTTP answer=%s\r\n", buf);
        result = HTTPAPI_READ_DATA_FAILED;
        LogError("(result = %s)\r\n", ENUM_TO_STRING(HTTPAPI_RESULT, result));
        LogInfo("HTTPAPI_ExecuteRequest::End=%d\r\n", result);
        return result;
    }
    if (statusCode)
        *statusCode = ret;
    LogInfo("HTTPAPI_ExecuteRequest::1Received response=%s\r\n", buf);

    //Read HTTP response headers
    if (readLine(handle,buf, sizeof(buf)) < 0)
    {
        result = HTTPAPI_READ_DATA_FAILED;
        LogError("(result = %s)\r\n", ENUM_TO_STRING(HTTPAPI_RESULT, result));
        LogInfo("HTTPAPI_ExecuteRequest::End=%d\r\n", result);
        return result;
    }

    while (buf[0])
    {
        const char ContentLength[] = "content-length:";
        const char TransferEncoding[] = "transfer-encoding:";

        LogInfo("Receiving header=%s\r\n", buf);

        if (strncasecmp(buf, ContentLength, CHAR_COUNT(ContentLength)) == 0)
        {
            if (sscanf(buf + CHAR_COUNT(ContentLength), " %d", &bodyLength) != 1)
            {
                result = HTTPAPI_READ_DATA_FAILED;
                LogError("(result = %s)\r\n", ENUM_TO_STRING(HTTPAPI_RESULT, result));
                LogInfo("HTTPAPI_ExecuteRequest::End=%d\r\n", result);
                return result;
            }
        }
        else if (strncasecmp(buf, TransferEncoding, CHAR_COUNT(TransferEncoding)) == 0)
        {
            const char* p = buf + CHAR_COUNT(TransferEncoding);
            while (isspace(*p)) p++;
            if (strcasecmp(p, "chunked") == 0)
                chunked = true;
        }

        char* whereIsColon = strchr((char*)buf, ':');
        if (whereIsColon && responseHeadersHandle != NULL)
        {
            *whereIsColon = '\0';
            HTTPHeaders_AddHeaderNameValuePair(responseHeadersHandle, buf, whereIsColon + 1);
        }

        if (readLine(handle,buf, sizeof(buf)) < 0)
        {
            result = HTTPAPI_READ_DATA_FAILED;
            LogError("(result = %s)\r\n", ENUM_TO_STRING(HTTPAPI_RESULT, result));
            LogInfo("HTTPAPI_ExecuteRequest::End=%d\r\n", result);
            return result;
        }
    }

    //Read HTTP response body
    LogInfo("HTTPAPI_ExecuteRequest::Receiving body=%d,%x\r\n", bodyLength, responseContent);
    if (!chunked)
    {
        if (bodyLength)
        {
            if (responseContent != NULL)
            {
                if (BUFFER_pre_build(responseContent, bodyLength) != 0)
                {
                    result = HTTPAPI_ALLOC_FAILED;
                    LogError("(result = %s)\r\n", ENUM_TO_STRING(HTTPAPI_RESULT, result));
                }
                else if (BUFFER_content(responseContent, &receivedContent) != 0)
                {
                    (void)BUFFER_unbuild(responseContent);

                    result = HTTPAPI_ALLOC_FAILED;
                    LogError("(result = %s)\r\n", ENUM_TO_STRING(HTTPAPI_RESULT, result));
                }

                if (readChunk(handle,(char*)receivedContent, bodyLength) < 0)
                {
                    result = HTTPAPI_READ_DATA_FAILED;
                    LogError("(result = %s)\r\n", ENUM_TO_STRING(HTTPAPI_RESULT, result));
                    LogInfo("HTTPAPI_ExecuteRequest::End=%d\r\n", result);
                    return result;
                }
                else
                {
                    LogInfo("HTTPAPI_ExecuteRequest::2Received response body=%s\r\n", receivedContent);
                    result = HTTPAPI_OK;
                }
            }
            else
            {
                (void)skipN(handle,bodyLength, buf, sizeof(buf));
                result = HTTPAPI_OK;
            }
        }
        else
        {
            result = HTTPAPI_OK;
        }
    }
    else
    {
        size_t size = 0;
        result = HTTPAPI_OK;
        for (;;)
        {
            int chunkSize;
            if (readLine(handle,buf, sizeof(buf)) < 0)    // read [length in hex]/r/n
            {
                result = HTTPAPI_READ_DATA_FAILED;
                LogError("(result = %s)\r\n", ENUM_TO_STRING(HTTPAPI_RESULT, result));
                LogInfo("HTTPAPI_ExecuteRequest::End=%d\r\n", result);
                return result;
            }
            if (sscanf(buf, "%x", &chunkSize) != 1)     // chunkSize is length of next line (/r/n is not counted)
            {
                //Cannot match string, error
                result = HTTPAPI_RECEIVE_RESPONSE_FAILED;
                LogError("(result = %s)\r\n", ENUM_TO_STRING(HTTPAPI_RESULT, result));
                LogInfo("HTTPAPI_ExecuteRequest::End=%d\r\n", result);
                return result;
            }

            if (chunkSize == 0)
            {
                // 0 length means next line is just '\r\n' and end of chunks
                if (readChunk(handle,(char*)buf, 2) < 0
                    || buf[0] != '\r' || buf[1] != '\n') // skip /r/n
                {
                    (void)BUFFER_unbuild(responseContent);

                    result = HTTPAPI_READ_DATA_FAILED;
                    LogError("(result = %s)\r\n", ENUM_TO_STRING(HTTPAPI_RESULT, result));
                    LogInfo("HTTPAPI_ExecuteRequest::End=%d\r\n", result);
                    return result;
                }
                break;
            }
            else
            {
                if (responseContent != NULL)
                {
                    if (BUFFER_enlarge(responseContent, chunkSize) != 0)
                    {
                        (void)BUFFER_unbuild(responseContent);

                        result = HTTPAPI_ALLOC_FAILED;
                        LogError("(result = %s)\r\n", ENUM_TO_STRING(HTTPAPI_RESULT, result));
                    }
                    else if (BUFFER_content(responseContent, &receivedContent) != 0)
                    {
                        (void)BUFFER_unbuild(responseContent);

                        result = HTTPAPI_ALLOC_FAILED;
                        LogError("(result = %s)\r\n", ENUM_TO_STRING(HTTPAPI_RESULT, result));
                    }

                    if (readChunk(handle,(char*)receivedContent + size, chunkSize) < 0)
                    {
                        result = HTTPAPI_READ_DATA_FAILED;
                        LogError("(result = %s)\r\n", ENUM_TO_STRING(HTTPAPI_RESULT, result));
                        LogInfo("HTTPAPI_ExecuteRequest::End=%d\r\n", result);
                        return result;
                    }
                }
                else
                {
                    if (skipN(handle,chunkSize, buf, sizeof(buf)) < 0)
                    {
                        result = HTTPAPI_READ_DATA_FAILED;
                        LogError("(result = %s)\r\n", ENUM_TO_STRING(HTTPAPI_RESULT, result));
                        LogInfo("HTTPAPI_ExecuteRequest::End=%d\r\n", result);
                        return result;
                    }
                }

                if (readChunk(handle,(char*)buf, 2) < 0
                    || buf[0] != '\r' || buf[1] != '\n') // skip /r/n
                {
                    result = HTTPAPI_READ_DATA_FAILED;
                    LogError("(result = %s)\r\n", ENUM_TO_STRING(HTTPAPI_RESULT, result));
                    LogInfo("HTTPAPI_ExecuteRequest::End=%d\r\n", result);
                    return result;
                }
                size += chunkSize;
            }
        }

        if (size > 0)
        {
            LogInfo("HTTPAPI_ExecuteRequest::Received chunk body=%s\r\n",(const char*)responseContent);
        }
    }
#if GPRS_INSTALL
        ThreadAPI_Sleep(500);//ThreadAPI_Sleep(1500);
#endif
    return result;
}

HTTPAPI_RESULT HTTPAPI_SetOption(HTTP_HANDLE handle, const char* optionName, const void* value)
{
    HTTPAPI_RESULT result;
    if (
        (handle == NULL) ||
        (optionName == NULL) ||
        (value == NULL)
        )
    {
        result = HTTPAPI_INVALID_ARG;
        LogError("invalid parameter (NULL) passed to HTTPAPI_SetOption\r\n");
    }
    else if (strcmp("TrustedCerts", optionName) == 0)
    {
        HTTP_HANDLE_DATA* h = (HTTP_HANDLE_DATA*)handle;
        if (h->certificate)
        {
            free(h->certificate);
        }

        int len = strlen((char*)value);
        h->certificate = malloc((len + 1)*sizeof(char));
        if (h->certificate == NULL)
        {
            result = HTTPAPI_ERROR;
            LogError("unable to allocate certificate memory in HTTPAPI_SetOption\r\n");
        }
        else
        {
            (void)strcpy(h->certificate, (const char*)value);
            result = HTTPAPI_OK;
        }
    }
    else
    {
        result = HTTPAPI_INVALID_ARG;
        LogError("unknown option %s\r\n", optionName);
    }
    return result;
}

HTTPAPI_RESULT HTTPAPI_CloneOption(const char* optionName, const void* value, const void** savedValue)
{
    HTTPAPI_RESULT result;
    if (
        (optionName == NULL) ||
        (value == NULL) ||
        (savedValue == NULL)
        )
    {
        result = HTTPAPI_INVALID_ARG;
        LogError("invalid argument(NULL) passed to HTTPAPI_CloneOption\r\n");
    }
    else if (strcmp("TrustedCerts", optionName) == 0)
    {
        size_t certLen = strlen((const char*)value);
        char* tempCert = (char*)malloc(certLen+1);
        if (tempCert == NULL)
        {
            result = HTTPAPI_INVALID_ARG;
            LogError("unable to allocate certificate memory in HTTPAPI_CloneOption\r\n");
        }
        else
        {
            (void)strcpy(tempCert, (const char*)value);
            *savedValue = tempCert;
            result = HTTPAPI_OK;
        }
    }
    else
    {
        result = HTTPAPI_INVALID_ARG;
        LogError("unknown option %s\r\n", optionName);
    }
    return result;
}
#if GPRS_INSTALL
void get_header(HTTP_HEADERS_HANDLE httpHeadersHandle,char index, char* header)
{
    //Send default headers

        if (HTTPHeaders_GetHeader(httpHeadersHandle, index, &header) != HTTP_HEADERS_OK)
        {
           // result = HTTPAPI_HTTP_HEADERS_FAILED;
            //LogError("(result = %s)\r\n", ENUM_TO_STRING(HTTPAPI_RESULT, result));
           // LogInfo("HTTPAPI_ExecuteRequest::End=%d\r\n", result);
#if TEST_Program
	printf("[Test_P]:HTTPAPI_ExecuteRequest::End6=%d\r\n", index);
#endif
           // return result;
        }
       //LogInfo("HTTPAPI_ExecuteRequest::Sending=%s\r\n", header);
#if TEST_Program
	printf("[Test_P]:HTTPheader:%s\r\n", header);
#endif
}
#endif

/**
  * @}
  */ 


/**
  * @}
  */ 


/**
  * @}
  */ 
