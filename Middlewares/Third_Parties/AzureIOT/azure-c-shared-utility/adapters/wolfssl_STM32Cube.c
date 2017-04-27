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
#include "ssl.h"
#include "error-ssl.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "azure_c_shared_utility/tlsio.h"
#include "azure_c_shared_utility/tlsio_wolfssl.h"
#include "azure_c_shared_utility/socketio.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/xlogging.h"
#include "socket.h"


/** Server address Settings */
#define TLS_SERVER_IP		"192.168.115.195"//	/**< Destination IP */
#define TLS_SERVER_PORT		8433			/**< Destination PORT */
#define _htons(A)				(A)
/** \brief Single socket for tls client */
static SOCKET tls_client_socket = -1;
typedef struct SockCbInfo {
    int sd;         /* Socket */

    /* Reader buffer markers */
    int bufRemain;
    int bufPos;     /* Position */
} SockCbInfo;
/** \brief TLS objects for client */
static WOLFSSL*        ssl_client    = NULL;
static WOLFSSL_CTX*    ctx_client    = NULL;
static WOLFSSL_METHOD* method_client = NULL;
static SockCbInfo      ioCtx;
/* Cipher suite for client */
#define CLIENT_CIPHER_LIST	"ECDH-ECDSA-AES128-GCM-SHA256"
static unsigned char FAKE_KEY_BUFFER[] = {
	0x41, 0x53, 0x4E, 0x31, 0x20, 0x4F, 0x49, 0x44, 0x3A, 0x20, 0x70, 0x72, 0x69, 0x6D, 0x65, 0x32,
	0x35, 0x36, 0x76, 0x31, 0x0A, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x42, 0x45, 0x47, 0x49, 0x4E, 0x20,
	0x45, 0x43, 0x20, 0x50, 0x41, 0x52, 0x41, 0x4D, 0x45, 0x54, 0x45, 0x52, 0x53, 0x2D, 0x2D, 0x2D,
	0x2D, 0x2D, 0x0A, 0x42, 0x67, 0x67, 0x71, 0x68, 0x6B, 0x6A, 0x4F, 0x50, 0x51, 0x4D, 0x42, 0x42,
	0x77, 0x3D, 0x3D, 0x0A, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x45, 0x4E, 0x44, 0x20, 0x45, 0x43, 0x20,
	0x50, 0x41, 0x52, 0x41, 0x4D, 0x45, 0x54, 0x45, 0x52, 0x53, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x0A,
	0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x42, 0x45, 0x47, 0x49, 0x4E, 0x20, 0x45, 0x43, 0x20, 0x50, 0x52,
	0x49, 0x56, 0x41, 0x54, 0x45, 0x20, 0x4B, 0x45, 0x59, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x0A, 0x4D,
	0x48, 0x63, 0x43, 0x41, 0x51, 0x45, 0x45, 0x49, 0x45, 0x57, 0x32, 0x61, 0x51, 0x4A, 0x7A, 0x6E,
	0x47, 0x79, 0x46, 0x6F, 0x54, 0x68, 0x62, 0x63, 0x75, 0x6A, 0x6F, 0x78, 0x36, 0x7A, 0x45, 0x41,
	0x34, 0x31, 0x54, 0x4E, 0x51, 0x54, 0x36, 0x62, 0x43, 0x6A, 0x63, 0x4E, 0x49, 0x33, 0x68, 0x71,
	0x41, 0x6D, 0x4D, 0x6F, 0x41, 0x6F, 0x47, 0x43, 0x43, 0x71, 0x47, 0x53, 0x4D, 0x34, 0x39, 0x0A,
	0x41, 0x77, 0x45, 0x48, 0x6F, 0x55, 0x51, 0x44, 0x51, 0x67, 0x41, 0x45, 0x75, 0x7A, 0x4F, 0x73,
	0x54, 0x43, 0x64, 0x51, 0x53, 0x73, 0x5A, 0x4B, 0x70, 0x51, 0x54, 0x44, 0x50, 0x4E, 0x36, 0x66,
	0x4E, 0x74, 0x74, 0x79, 0x4C, 0x63, 0x36, 0x55, 0x36, 0x69, 0x76, 0x36, 0x79, 0x79, 0x41, 0x4A,
	0x4F, 0x53, 0x77, 0x57, 0x36, 0x47, 0x45, 0x43, 0x36, 0x61, 0x39, 0x4E, 0x30, 0x77, 0x4B, 0x54,
	0x0A, 0x6D, 0x6A, 0x46, 0x62, 0x6C, 0x35, 0x49, 0x68, 0x66, 0x2F, 0x44, 0x50, 0x47, 0x4E, 0x71,
	0x52, 0x45, 0x51, 0x49, 0x30, 0x68, 0x75, 0x67, 0x67, 0x57, 0x44, 0x4D, 0x4C, 0x67, 0x44, 0x53,
	0x4A, 0x32, 0x41, 0x3D, 0x3D, 0x0A, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x45, 0x4E, 0x44, 0x20, 0x45,
	0x43, 0x20, 0x50, 0x52, 0x49, 0x56, 0x41, 0x54, 0x45, 0x20, 0x4B, 0x45, 0x59, 0x2D, 0x2D, 0x2D,
	0x2D, 0x2D, 0x0A
};


/** Socket Status */
#define	SOCKET_STATUS_BIND					(1 << 0)		/* 00000001 */
#define	SOCKET_STATUS_LISTEN				(1 << 1)		/* 00000010 */
#define	SOCKET_STATUS_ACCEPT				(1 << 2)		/* 00000100 */
#define	SOCKET_STATUS_CONNECT				(1 << 3)		/* 00001000 */
#define	SOCKET_STATUS_RECEIVE				(1 << 4)		/* 00010000 */
#define	SOCKET_STATUS_SEND			    	(1 << 5)		/* 00100000 */
#define	SOCKET_STATUS_RECEIVE_FROM	    	(1 << 6)		/* 01000000 */
#define	SOCKET_STATUS_SEND_TO				(1 << 7)		/* 10000000 */
typedef int socklen_t;


TLSIO_CONFIG* io_create_parameters_s;
CONCRETE_IO_HANDLE tls_io_s;
ON_IO_OPEN_COMPLETE on_io_open_complete_s;
char* on_io_open_complete_context_s;
ON_BYTES_RECEIVED on_bytes_received_s;
char* on_bytes_received_context_s;
ON_IO_ERROR on_io_error_s;
char* on_io_error_context_s;
ON_SEND_COMPLETE on_send_complete_s;
void* callback_context_s;


/**
 * \brief Callback to get the Data from socket.
 */
void tls_client_socket_cb(SOCKET sock, uint8_t u8Msg, void *pvMsg)
{
	printf("Socket Event : %s \r\n", tls_get_socket_string(u8Msg));
	switch (u8Msg) {

		case SOCKET_MSG_BIND:
		{
			tstrSocketBindMsg *pstrBind = (tstrSocketBindMsg *)pvMsg;
			if (pstrBind && pstrBind->status == 0) {
				printf("socket callback : bind success!\r\n");
				ENABLE_SOCKET_STATUS(SOCKET_STATUS_BIND);
			} else {
				printf("socket callback : bind error!\r\n");
				DISABLE_SOCKET_STATUS(SOCKET_STATUS_BIND);
				close(tls_client_socket);
			}
		}
		break;

		case SOCKET_MSG_LISTEN:
		{
			tstrSocketListenMsg *pstrListen = (tstrSocketListenMsg *)pvMsg;
			if (pstrListen && pstrListen->status == 0) {
				printf("socket callback : listen success!\r\n");
				ENABLE_SOCKET_STATUS(SOCKET_STATUS_LISTEN);
			} else {
				printf("socket callback : listen error!\r\n");
				DISABLE_SOCKET_STATUS(SOCKET_MSG_LISTEN);
				close(tls_client_socket);
			}
		}
		break;

		case SOCKET_MSG_ACCEPT:
		{
			tstrSocketAcceptMsg *pstrAccept = (tstrSocketAcceptMsg *)pvMsg;
			if (pstrAccept) {
				printf("socket callback : accept success!\r\n");
				ENABLE_SOCKET_STATUS(SOCKET_STATUS_ACCEPT);
			} else {
				printf("socket callback : accept error!\r\n");
				DISABLE_SOCKET_STATUS(SOCKET_STATUS_ACCEPT);
				close(tls_client_socket);
			}
		}
		break;

		case SOCKET_MSG_CONNECT:
		{
			tstrSocketConnectMsg *pstrConnect = (tstrSocketConnectMsg *)pvMsg;
			if (pstrConnect && pstrConnect->s8Error >= 0) {
				printf("socket callback : connect success!\r\n");
				ENABLE_SOCKET_STATUS(SOCKET_STATUS_CONNECT);
			} else {
				printf("socket callback : connect error!\r\n");
				DISABLE_SOCKET_STATUS(SOCKET_STATUS_CONNECT);
				close(tls_client_socket);
			}
		}
		break;


		case SOCKET_MSG_RECV:
		{
			tstrSocketRecvMsg *pstrRecv = (tstrSocketRecvMsg *)pvMsg;
			if (pstrRecv && pstrRecv->s16BufferSize > 0) {
				printf("socket callback : recv success!\r\n");
				ENABLE_SOCKET_STATUS(SOCKET_STATUS_RECEIVE);
			} else {
				printf("socket callback : recv error!\r\n");
				DISABLE_SOCKET_STATUS(SOCKET_STATUS_RECEIVE);
				close(tls_client_socket);
			}
		}
		break;


		case SOCKET_MSG_SEND:
		{
			tstrSocketConnectMsg *pstrConnect = (tstrSocketConnectMsg *)pvMsg;
			if (pstrConnect && pstrConnect->s8Error >= 0) {
				printf("socket callback : send success!\r\n");
				ENABLE_SOCKET_STATUS(SOCKET_STATUS_SEND);
			} else {
				printf("socket callback : send error!\r\n");
				DISABLE_SOCKET_STATUS(SOCKET_STATUS_SEND);
				close(tls_client_socket);
			}
		}
		break;

	default:
		break;
	}
}
/**
 * \brief Try to send simple message to server.
 */
int tls_send_message(void)
{
    char msg[32] = "Hello Bob!";   /* GET may make bigger */
    int  msg_len = (int)strlen(msg);

    if (wolfSSL_write(ssl_client, msg, msg_len) != msg_len) {
        printf("Failed to send client message!\r\n");
		return SSL_ERROR_SSL;
    }

	printf("\r\n============================================\r\n");
	printf("Sent a encrypted cipher text : %s\r\n", msg);
	printf("============================================\r\n");
	return SSL_SUCCESS;
}

/**
 * \brief Try to receive response from server.
 */
int tls_receive_message(void)
{
	int  msg_len;
	uint8_t msg[80];

    msg_len = wolfSSL_read(ssl_client, msg, sizeof(msg)-1);

	if (msg_len > 0) {
		msg[msg_len] = 0;
		printf("\r\n===================================\r\n");
		printf("Received a plain text : %s\r\n", msg);
		printf("===================================\r\n");
		return SSL_SUCCESS;
	} else {
		printf("Failed to receive msessage\r\n");
		return SSL_ERROR_SSL;
	}
}

/**
 * \brief Get started handshake with server.
 */
int tls_start_handshake(void)
{

	if (wolfSSL_connect(ssl_client) != SSL_SUCCESS) {
        int  err = wolfSSL_get_error(ssl_client, 0);
        char buffer[80];
        printf("Failed to handshake, Error num = %d, %s\r\n", err, wolfSSL_ERR_error_string(err, buffer));
        return SSL_FAILURE;
    }

	return SSL_SUCCESS;
}

/**
 * \brief Load TLS objects, set certificate and cipher suite.
 */
int tls_load_wolfssl_objects(void)
{

	if (wolfSSL_Init() != SSL_SUCCESS) {
    	printf("Failed to initialze wolfSSL!\r\n");
		return SSL_FAILURE;
	}

  	method_client = wolfTLSv1_2_client_method();
	if (method_client == NULL) {
    	printf("Failed to alloc dynamic buffer.\r\n");
		return SSL_FAILURE;
	}

	ctx_client = wolfSSL_CTX_new(method_client);
	if (ctx_client == NULL) {
    	printf("Failed to create wolfssl context.\r\n");
		return SSL_FAILURE;
	}

	if (wolfSSL_CTX_use_PrivateKey_buffer(ctx_client, FAKE_KEY_BUFFER,
						sizeof(FAKE_KEY_BUFFER), SSL_FILETYPE_PEM) != SSL_SUCCESS) {
		printf("Can't load ecc key file.\r\n");
		return SSL_FAILURE;
	}
#if 0
	if (wolfSSL_CTX_load_verify_buffer(ctx_client, atcert.signer_ca,
		atcert.signer_ca_size, SSL_FILETYPE_ASN1) != SSL_SUCCESS) {
		printf("Faile to load verification certificate!\r\n");
		return SSL_FAILURE;
	}

	if (wolfSSL_CTX_use_certificate_buffer(ctx_client, atcert.end_user,
			atcert.end_user_size, SSL_FILETYPE_ASN1) != SSL_SUCCESS) {
		printf("Faile to set own certificate!\r\n");
		return SSL_FAILURE;
	}
#endif
    if (wolfSSL_CTX_set_cipher_list(ctx_client, CLIENT_CIPHER_LIST) != SSL_SUCCESS) {
		printf("unable to set cipher list for client : %s\r\n", CLIENT_CIPHER_LIST);
		return SSL_FAILURE;
    }

	//wolfSSL_CTX_set_verify(ctx_client, SSL_VERIFY_PEER, tls_verify_peer_cert_cb);
	//wolfSSL_CTX_SetEccSignCb(ctx_client, tls_sign_certificate_cb);
	//wolfSSL_CTX_SetEccSharedSecretCb(ctx_client, tls_create_pms_cb);
	//wolfSSL_SetIORecv(ctx_client, tls_receive_packet_cb);
	//wolfSSL_SetIOSend(ctx_client, tls_send_packet_cb);

	ssl_client = wolfSSL_new(ctx_client);
	if (ssl_client == NULL) {
		printf("unable to get wolfssl context.\r\n");
		return SSL_FAILURE;
	}

    ioCtx.sd = tls_client_socket;
    wolfSSL_SetIOReadCtx(ssl_client, &ioCtx);
    wolfSSL_SetIOWriteCtx(ssl_client, &ioCtx);

	wolfSSL_Debugging_ON();

	return SSL_SUCCESS;
}

/**
 * \brief Shutdown WolfSSL objects.
 */
int tls_release_objects(void)
{
	wolfSSL_shutdown(ssl_client);

	wolfSSL_free(ssl_client);

	wolfSSL_CTX_free(ctx_client);

	close(tls_client_socket);

	printf("Released client objects!\r\n");

	return SSL_SUCCESS;
}
/**
 * \brief Try to connect to server with given ip and port.
 */
SOCKET tls_access_server(const char* ip, uint16_t port)
{
	struct sockaddr_in addr = {AF_INET, 0,};
	int rc = 1;
	int val;
	socklen_t len = sizeof(val);

	tls_client_socket = socket(AF_INET, SOCK_STREAM, 0);

	if (tls_client_socket < 0) {
		printf("Failed to assign socket!\r\n");
		return -1;
	}

	/* Initialize socket address structure. */
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = nmi_inet_addr(TLS_SERVER_IP);
	addr.sin_port = _htons(TLS_SERVER_PORT);

	if (connect(tls_client_socket, (const struct sockaddr*)&addr, sizeof(addr)) != 0) {
		printf("Failed to connect to server\r\n");
		close(tls_client_socket);
		return -1;
	}

	while (!GET_SOCKET_STATUS(SOCKET_STATUS_CONNECT)) {
		//????m2m_wifi_handle_events(NULL);
	}

	return tls_client_socket;
}
void wolfssl_test(void)
{
	int ret = 0;
	const char* buffer={"send message"};
	size_t size = 512;

	io_create_parameters_s->hostname ="192.168.115.195";
	io_create_parameters_s->port = 8433;
	/* Initialize socket module */
	socketInit();

    /* Register socket callback to communicate with TLS server */
	registerSocketCallback(tls_client_socket_cb, NULL);

	ret = tls_access_server(TLS_SERVER_IP, TLS_SERVER_PORT);
	if (ret < 0) {
		printf("Failed to create TLS client socket!\r\n");
		//break;
	}
#if 0
	ret = tls_build_signer_ca_cert();
	if (ret != ATCACERT_E_SUCCESS) {
		printf("Failed to build server's signer certificate!\r\n");
		//break;
	}

	ret = tls_build_end_user_cert();
	if (ret != ATCACERT_E_SUCCESS) {
		printf("Failed to build client certificate!\r\n");
		//break;
	}
#endif
	ret = tls_load_wolfssl_objects();
	if (ret != SSL_SUCCESS) {
		printf("Failed to load wolfssl!\r\n");
		//break;
	}

	ret = tls_start_handshake();
	if (ret != SSL_SUCCESS) {
		printf("Failed to handshake with server!\r\n");
		//break;
	}

	ret = tls_send_message();
	if (ret != SSL_SUCCESS) {
		printf("Failed to send msessage to server!\r\n");
		//break;
	}

	ret = tls_receive_message();
	if (ret != SSL_SUCCESS) {
		printf("Failed to receive msessage from server!\r\n");
		//break;
	}

	/* Release initialized objects */
	tls_release_objects();
	//atcatls_finish();

#if 0
	tlsio_wolfssl_init();
	tls_io_s = tlsio_wolfssl_create(io_create_parameters_s);//create_wolfssl_instance

	tlsio_wolfssl_open(tls_io_s,on_io_open_complete_s,on_io_open_complete_context_s,on_bytes_received_s,on_bytes_received_context_s,on_io_error_s,on_io_error_context_s);

	tlsio_wolfssl_send(tls_io_s, buffer,  size,  on_send_complete_s,  callback_context_s);
#endif
}




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


#if 0
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
#if 1
#define TCP_SERVERPOR_TEST                   8433
       // httpHandle->host = "192.168.115.195";//:8433.
        if (InternetInterfaceSocketOpen(handle,"192.168.115.195", TCP_SERVERPOR_TEST,SOCKETCONNECTIONTYPE_TLS) !=SOCKETSTATUS_CONNECTED)
#else
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

    //Send default headers
    for (size_t i = 0; i < headersCount; i++)
    {
        char* header;
#if 0//GPRS_INSTALL
        ThreadAPI_Sleep(200);//ThreadAPI_Sleep(1500);
#endif
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
    }
#if GPRS_INSTALL
    //send 0x1a  ????
    if (InternetInterfaceSocketSend(handle,"0x1a",1)!=SOCKETOPERATIONRESULT_SUCCESS)
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
