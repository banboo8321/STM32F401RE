/**
 *
 * \file
 *
 * \brief BSD alike socket interface internal types.
 *
 * Copyright (c) 2015 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */
#ifndef __M2M_SOCKET_HOST_IF_H__
#define __M2M_SOCKET_HOST_IF_H__


#ifdef  __cplusplus
extern "C" {
#endif

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
INCLUDES
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

#ifndef	_BOOT_
#ifndef _FIRMWARE_
#include "socket.h"
#else
#include "m2m_types.h"
#endif
#endif

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
MACROS
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

#ifdef _FIRMWARE_
#define HOSTNAME_MAX_SIZE			(64)
#endif

#define SSL_MAX_OPT_LEN				HOSTNAME_MAX_SIZE



#define SOCKET_CMD_INVALID					0x00
/*!< 
	Invlaid Socket command value.
*/


#define SOCKET_CMD_BIND					0x41
/*!< 
	Socket Binding command value.
*/


#define SOCKET_CMD_LISTEN					0x42
/*!< 
	Socket Listening command value.
*/


#define SOCKET_CMD_ACCEPT					0x43
/*!< 
	Socket Accepting command value.
*/


#define SOCKET_CMD_CONNECT				0x44
/*!< 
	Socket Connecting command value.
*/


#define SOCKET_CMD_SEND					0x45
/*!< 
	Socket send command value.
*/


#define SOCKET_CMD_RECV					0x46
/*!< 
	Socket Recieve command value.
*/


#define SOCKET_CMD_SENDTO					0x47
/*!< 
	Socket sendTo command value.
*/


#define SOCKET_CMD_RECVFROM				0x48
/*!< 
	Socket RecieveFrom command value.
*/


#define SOCKET_CMD_CLOSE					0x49
/*!< 
	Socket Close command value.
*/


#define SOCKET_CMD_DNS_RESOLVE			0x4A
/*!< 
	Socket DNS Resolve command value.
*/


#define SOCKET_CMD_SSL_CONNECT			0x4B
/*!< 
	SSL-Socket Connect command value.
*/


#define SOCKET_CMD_SSL_SEND				0x4C	
/*!< 
	SSL-Socket Send command value.
*/	


#define SOCKET_CMD_SSL_RECV				0x4D
/*!< 
	SSL-Socket Recieve command value.
*/


#define SOCKET_CMD_SSL_CLOSE				0x4E
/*!< 
	SSL-Socket Close command value.
*/


#define SOCKET_CMD_SET_SOCKET_OPTION		0x4F
/*!< 
	Set Socket Option command value.
*/


#define SOCKET_CMD_SSL_CREATE				0x50
/*!<
*/


#define SOCKET_CMD_SSL_SET_SOCK_OPT		0x51


#define SOCKET_CMD_PING						0x52




#define PING_ERR_SUCCESS					0
#define PING_ERR_DEST_UNREACH				1
#define PING_ERR_TIMEOUT					2

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
DATA TYPES
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/


/*!
*  @brief	
*/
typedef struct{	
	unsigned short		u16Family;
	unsigned short		u16Port;
	unsigned long		u32IPAddr;
}tstrSockAddr;


typedef signed char			SOCKET;
typedef tstrSockAddr	tstrUIPSockAddr;



/*!
@struct	\
	tstrDnsReply
	
@brief
	DNS Reply, contains hostName and HostIP.
*/
typedef struct{
	char		acHostName[HOSTNAME_MAX_SIZE];
	unsigned long		u32HostIP;
}tstrDnsReply;


/*!
@brief
*/
typedef struct{
	tstrSockAddr	strAddr;
	SOCKET		sock;
	unsigned char		u8Void;
	unsigned short		u16SessionID;
}tstrBindCmd;


/*!
@brief
*/
typedef struct{
	SOCKET		sock;
	signed char		s8Status;
	unsigned short		u16SessionID;
}tstrBindReply;


/*!
*  @brief
*/
typedef struct{
	SOCKET	sock;
	unsigned char	u8BackLog;
	unsigned short	u16SessionID;
}tstrListenCmd;


/*!
@struct	\
	tstrSocketRecvMsg
	
@brief	Socket recv status. 

	It is passed to the APPSocketEventHandler with SOCKET_MSG_RECV or SOCKET_MSG_RECVFROM message type 
	in a response to a user call to the recv or recvfrom.
	If the received data from the remote peer is larger than the USER Buffer size (given at recv call), the data is 
	delivered to the user in a number of consecutive chunks according to the USER Buffer size.
*/
typedef struct{
	SOCKET		sock;
	signed char			s8Status;
	unsigned short		u16SessionID;
}tstrListenReply;


/*!
*  @brief
*/
typedef struct{
	tstrSockAddr	strAddr;
	SOCKET			sListenSock;
	SOCKET			sConnectedSock;
	unsigned short			u16Void;
}tstrAcceptReply;


/*!
*  @brief
*/
typedef struct{
	tstrSockAddr	strAddr;
	SOCKET			sock;
	unsigned char			u8SslFlags;
	unsigned short			u16SessionID;
}tstrConnectCmd;


/*!
@struct	\
	tstrConnectReply
	
@brief
	Connect Reply, contains sock number and error value
*/
typedef struct{
	SOCKET		sock;
	signed char		s8Error;
	unsigned short		u16SessionID;
}tstrConnectReply;


/*!
@brief
*/
typedef struct{
	SOCKET			sock;
	unsigned char			u8Void;
	unsigned short			u16DataSize;
	tstrSockAddr	strAddr;
	unsigned short		u16SessionID;
	unsigned short			u16Void;
}tstrSendCmd;


/*!
@struct	\
	tstrSendReply
	
@brief
	Send Reply, contains socket number and number of sent bytes.
*/
typedef struct{
	SOCKET		sock;
	unsigned char		u8Void;
	signed short		s16SentBytes;
	unsigned short		u16SessionID;
	unsigned short		u16Void;
}tstrSendReply;


/*!
*  @brief
*/
typedef struct{
	unsigned long		u32Timeoutmsec;
	SOCKET		sock;
	unsigned char		u8Void;
	unsigned short		u16SessionID;
}tstrRecvCmd;


/*!
@struct
@brief
*/
typedef struct{
	tstrSockAddr		strRemoteAddr;
	signed short			s16RecvStatus;
	unsigned short			u16DataOffset;
	SOCKET			sock;
	unsigned char			u8Void;
	unsigned short			u16SessionID;
}tstrRecvReply;


/*!
*  @brief
*/
typedef struct{
	unsigned long		u32OptionValue;
	SOCKET		sock;
	unsigned char 		u8Option;
	unsigned short		u16SessionID;
}tstrSetSocketOptCmd;


typedef struct{
	SOCKET		sslSock;
	unsigned char		__PAD24__[3];
}tstrSSLSocketCreateCmd;


/*!
*  @brief
*/
typedef struct{
	SOCKET		sock;
	unsigned char 		u8Option;
	unsigned short		u16SessionID;
	unsigned long		u32OptLen;
	unsigned char		au8OptVal[SSL_MAX_OPT_LEN];
}tstrSSLSetSockOptCmd;


/*!
*/
typedef struct{
	unsigned long	u32DestIPAddr;
	unsigned long	u32CmdPrivate;
	unsigned short	u16PingCount;
	unsigned char	u8TTL;
	unsigned char	__PAD8__;
}tstrPingCmd;


typedef struct{
	unsigned long	u32IPAddr;
	unsigned long	u32CmdPrivate;
	unsigned long	u32RTT;
	unsigned short	u16Success;
	unsigned short	u16Fail;
	unsigned char	u8ErrorCode;
	unsigned char	__PAD24__[3];
}tstrPingReply;

#ifdef  __cplusplus
}
#endif /* __cplusplus */

#endif /* __M2M_SOCKET_HOST_IF_H__ */
