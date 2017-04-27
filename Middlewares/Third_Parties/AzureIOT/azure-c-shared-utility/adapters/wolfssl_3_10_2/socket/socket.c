/**
 *
 * \file
 *
 * \brief BSD alike socket interface.
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

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
INCLUDES
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

//#include "bsp/include/nm_bsp.h"
#include<stdlib.h>
#include "socket.h"
//#include "driver/source/m2m_hif.h"
#include "socket_internal.h"
#include "m2m_socket_host_if.h"

#define NBIT31				(0x80000000)
#define NBIT30				(0x40000000)
#define NBIT29				(0x20000000)
#define NBIT28				(0x10000000)
#define NBIT27				(0x08000000)
#define NBIT26				(0x04000000)
#define NBIT25				(0x02000000)
#define NBIT24				(0x01000000)
#define NBIT23				(0x00800000)
#define NBIT22				(0x00400000)
#define NBIT21				(0x00200000)
#define NBIT20				(0x00100000)
#define NBIT19				(0x00080000)
#define NBIT18				(0x00040000)
#define NBIT17				(0x00020000)
#define NBIT16				(0x00010000)
#define NBIT15				(0x00008000)
#define NBIT14				(0x00004000)
#define NBIT13				(0x00002000)
#define NBIT12				(0x00001000)
#define NBIT11				(0x00000800)
#define NBIT10				(0x00000400)
#define NBIT9				(0x00000200)
#define NBIT8				(0x00000100)
#define NBIT7				(0x00000080)
#define NBIT6				(0x00000040)
#define NBIT5				(0x00000020)
#define NBIT4				(0x00000010)
#define NBIT3				(0x00000008)
#define NBIT2				(0x00000004)
#define NBIT1				(0x00000002)
#define NBIT0				(0x00000001)
/*states*/
#define M2M_SUCCESS         ((signed char)0)
#define M2M_ERR_SEND		((signed char)-1)
#define M2M_ERR_RCV			((signed char)-2)
#define M2M_ERR_MEM_ALLOC	((signed char)-3)
#define M2M_ERR_TIME_OUT	((signed char)-4)
#define M2M_ERR_INIT        ((signed char)-5)
#define M2M_ERR_BUS_FAIL    ((signed char)-6)
#define M2M_NOT_YET			((signed char)-7)
#define M2M_ERR_FIRMWARE	((signed char)-8)
#define M2M_SPI_FAIL		((signed char)-9)
#define M2M_ERR_FIRMWARE_bURN	 ((signed char)-10)
#define M2M_ACK				((signed char)-11)
#define M2M_ERR_FAIL		((signed char)-12)
#define M2M_ERR_FW_VER_MISMATCH         ((signed char)-13)
#define M2M_ERR_SCAN_IN_PROGRESS         ((signed char)-14)
/*
Invalid argument
*/
#define M2M_ERR_INVALID_ARG				 ((signed char)-15)
typedef enum{
	M2M_REQ_GRP_MAIN = 0, M2M_REQ_GRP_WIFI, M2M_REQ_GRP_IP, M2M_REQ_GRP_HIF, M2M_REQ_GRP_OTA
}tenuM2mReqGrp;
typedef enum{
	M2M_REQ_CONFIG_PKT,
	M2M_REQ_DATA_PKT = NBIT7
}tenuM2mReqPkt;
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
MACROS
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
/**
*	@struct		tstrHifHdr
*	@brief		Structure to hold HIF header
*/
typedef struct
{
	unsigned char   u8Gid;		/*!< Group ID */
	unsigned char   u8Opcode;	/*!< OP code */
	unsigned short  u16Length;	/*!< Payload length */
}tstrHifHdr;
#define M2M_HIF_HDR_OFFSET (sizeof(tstrHifHdr) + 4)


#define TLS_RECORD_HEADER_LENGTH			(5)
#define ETHERNET_HEADER_OFFSET				(34)
#define ETHERNET_HEADER_LENGTH				(14)
#define TCP_IP_HEADER_LENGTH				(40)
#define UDP_IP_HEADER_LENGTH				(28)

#define IP_PACKET_OFFSET					(ETHERNET_HEADER_LENGTH + ETHERNET_HEADER_OFFSET - M2M_HIF_HDR_OFFSET)

#define TCP_TX_PACKET_OFFSET				(IP_PACKET_OFFSET + TCP_IP_HEADER_LENGTH)
#define UDP_TX_PACKET_OFFSET				(IP_PACKET_OFFSET + UDP_IP_HEADER_LENGTH)
#define SSL_TX_PACKET_OFFSET				(TCP_TX_PACKET_OFFSET + TLS_RECORD_HEADER_LENGTH)

#define SOCKET_REQUEST(reqID, reqArgs, reqSize, reqPayload, reqPayloadSize, reqPayloadOffset)		\
		gprs_send(M2M_REQ_GRP_IP, reqID, reqArgs, reqSize, reqPayload, reqPayloadSize, reqPayloadOffset)


#define SSL_FLAGS_ACTIVE					0x01
#define SSL_FLAGS_BYPASS_X509				0x02

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
PRIVATE DATA TYPES
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/


/*!
*  @brief
*/
typedef struct{
	SOCKET		sock;
	unsigned char		u8Dummy;
	unsigned short		u16SessionID;
}tstrCloseCmd;


/*!
*  @brief
*/
typedef struct{
	unsigned char				*pu8UserBuffer;
	unsigned short				u16UserBufferSize;
	unsigned char				bIsUsed;
	unsigned char				u8SSLFlags;
	unsigned char				bIsRecvPending;
	unsigned short				u16SessionID;
}tstrSocket;

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
GLOBALS
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

volatile signed char					gsockerrno;
volatile tstrSocket				gastrSockets[MAX_SOCKET];
volatile unsigned char					gu8OpCode;
volatile unsigned short					gu16BufferSize;
volatile unsigned short					gu16SessionID = 0;	

volatile tpfAppSocketCb		    gpfAppSocketCb;
volatile tpfAppResolveCb		gpfAppResolveCb;
volatile unsigned char					gbSocketInit = 0;
volatile tpfPingCb				gfpPingCb;
/*********************************************************************
Function
		Socket_ReadSocketData

Description
		Callback function used by the NMC1500 driver to deliver messages
		for socket layer.

Return
		None.

Author
		Ahmed Ezzat

Version
		1.0

Date
		17 July 2012
*********************************************************************/
 void Socket_ReadSocketData(SOCKET sock, tstrSocketRecvMsg *pstrRecv,unsigned char u8SocketMsg,
								  unsigned long u32StartAddress,unsigned short u16ReadCount)
{
	if((u16ReadCount > 0) && (gastrSockets[sock].pu8UserBuffer != NULL) && (gastrSockets[sock].u16UserBufferSize > 0) && (gastrSockets[sock].bIsUsed == 1))
	{
		unsigned long	u32Address = u32StartAddress;
		unsigned short	u16Read;
		signed short	s16Diff;
		unsigned char	u8SetRxDone;

		pstrRecv->u16RemainingSize = u16ReadCount;
		do
		{
			u8SetRxDone = 1;
			u16Read = u16ReadCount;
			s16Diff	= u16Read - gastrSockets[sock].u16UserBufferSize;
			if(s16Diff > 0)
			{
				u8SetRxDone = 0;
				u16Read		= gastrSockets[sock].u16UserBufferSize;
			}
			if(hif_receive(u32Address, gastrSockets[sock].pu8UserBuffer, u16Read, u8SetRxDone) == M2M_SUCCESS)
			{
				pstrRecv->pu8Buffer			= gastrSockets[sock].pu8UserBuffer;
				pstrRecv->s16BufferSize		= u16Read;
				pstrRecv->u16RemainingSize	-= u16Read;

				if (gpfAppSocketCb)
					gpfAppSocketCb(sock,u8SocketMsg, pstrRecv);

				u16ReadCount -= u16Read;
				u32Address += u16Read;
			}
			else
			{
				M2M_INFO("(ERRR)Current <%d>\n", u16ReadCount);
				break;
			}
		}while(u16ReadCount != 0);
	}
}
/*********************************************************************
Function
		m2m_ip_cb

Description
		Callback function used by the NMC1000 driver to deliver messages
		for socket layer.

Return
		None.

Author
		Ahmed Ezzat

Version
		1.0

Date
		17 July 2012
*********************************************************************/
static void m2m_ip_cb(unsigned char u8OpCode, unsigned short u16BufferSize,unsigned long u32Address)
{
	if(u8OpCode == SOCKET_CMD_BIND)
	{
		tstrBindReply		strBindReply;
		tstrSocketBindMsg	strBind;

		if(hif_receive(u32Address, (unsigned char*)&strBindReply, sizeof(tstrBindReply), 0) == M2M_SUCCESS)
		{
			strBind.status = strBindReply.s8Status;
			if(gpfAppSocketCb)
				gpfAppSocketCb(strBindReply.sock,SOCKET_MSG_BIND,&strBind);
		}
	}
	else if(u8OpCode == SOCKET_CMD_LISTEN)
	{
		tstrListenReply			strListenReply;
		tstrSocketListenMsg		strListen;
		if(hif_receive(u32Address, (unsigned char*)&strListenReply, sizeof(tstrListenReply), 0) == M2M_SUCCESS)
		{
			strListen.status = strListenReply.s8Status;
			if(gpfAppSocketCb)
				gpfAppSocketCb(strListenReply.sock,SOCKET_MSG_LISTEN, &strListen);
		}
	}
	else if(u8OpCode == SOCKET_CMD_ACCEPT)
	{
		tstrAcceptReply			strAcceptReply;
		tstrSocketAcceptMsg		strAccept;
		if(hif_receive(u32Address, (unsigned char*)&strAcceptReply, sizeof(tstrAcceptReply), 0) == M2M_SUCCESS)
		{
			if(strAcceptReply.sConnectedSock >= 0)
			{
				gastrSockets[strAcceptReply.sConnectedSock].u8SSLFlags 	= 0;
				gastrSockets[strAcceptReply.sConnectedSock].bIsUsed 	= 0;//1

				/* The session ID is used to distinguish different socket connections
					by comparing the assigned session ID to the one reported by the firmware*/
				++gu16SessionID;
				if(gu16SessionID == 0)
					++gu16SessionID;

				gastrSockets[strAcceptReply.sConnectedSock].u16SessionID = gu16SessionID;
				M2M_DBG("Socket %d session ID = %d\r\n",strAcceptReply.sConnectedSock , gu16SessionID );		
			}
			strAccept.sock = strAcceptReply.sConnectedSock;
			strAccept.strAddr.sin_family		= AF_INET;
			strAccept.strAddr.sin_port = strAcceptReply.strAddr.u16Port;
			strAccept.strAddr.sin_addr.s_addr = strAcceptReply.strAddr.u32IPAddr;
			if(gpfAppSocketCb)
				gpfAppSocketCb(strAcceptReply.sListenSock, SOCKET_MSG_ACCEPT, &strAccept);
		}
	}
	else if((u8OpCode == SOCKET_CMD_CONNECT) || (u8OpCode == SOCKET_CMD_SSL_CONNECT))
	{
		tstrConnectReply		strConnectReply;
		tstrSocketConnectMsg	strConnMsg;
		if(hif_receive(u32Address, (unsigned char*)&strConnectReply, sizeof(tstrConnectReply), 0) == M2M_SUCCESS)
		{
			strConnMsg.sock		= strConnectReply.sock;
			strConnMsg.s8Error	= strConnectReply.s8Error;
			if(gpfAppSocketCb)
				gpfAppSocketCb(strConnectReply.sock,SOCKET_MSG_CONNECT, &strConnMsg);
		}
	}
	else if(u8OpCode == SOCKET_CMD_DNS_RESOLVE)
	{
		tstrDnsReply	strDnsReply;
		if(hif_receive(u32Address, (unsigned char*)&strDnsReply, sizeof(tstrDnsReply), 0) == M2M_SUCCESS)
		{
			//strDnsReply.u32HostIP = strDnsReply.u32HostIP;
			if(gpfAppResolveCb)
				gpfAppResolveCb((unsigned char*)strDnsReply.acHostName, strDnsReply.u32HostIP);
		}
	}
	else if((u8OpCode == SOCKET_CMD_RECV) || (u8OpCode == SOCKET_CMD_RECVFROM) || (u8OpCode == SOCKET_CMD_SSL_RECV))
	{
		SOCKET				sock;
		signed short				s16RecvStatus;
		tstrRecvReply		strRecvReply;
		unsigned short				u16ReadSize;
		tstrSocketRecvMsg	strRecvMsg;
		unsigned char				u8CallbackMsgID = SOCKET_MSG_RECV;
		unsigned short				u16DataOffset;

		if(u8OpCode == SOCKET_CMD_RECVFROM)
			u8CallbackMsgID = SOCKET_MSG_RECVFROM;

		/* Read RECV REPLY data structure.
		*/
		u16ReadSize = sizeof(tstrRecvReply);
		if(hif_receive(u32Address, (unsigned char*)&strRecvReply, u16ReadSize, 0) == M2M_SUCCESS)
		{
			unsigned short u16SessionID = 0;

			sock			= strRecvReply.sock;
			u16SessionID = strRecvReply.u16SessionID;
			M2M_DBG("recv callback session ID = %d\r\n",u16SessionID);
			
			/* Reset the Socket RX Pending Flag.
			*/
			gastrSockets[sock].bIsRecvPending = 0;

			s16RecvStatus	= NM_BSP_B_L_16(strRecvReply.s16RecvStatus);
			u16DataOffset	= NM_BSP_B_L_16(strRecvReply.u16DataOffset);
			strRecvMsg.strRemoteAddr.sin_port 			= strRecvReply.strRemoteAddr.u16Port;
			strRecvMsg.strRemoteAddr.sin_addr.s_addr 	= strRecvReply.strRemoteAddr.u32IPAddr;

			if(u16SessionID == gastrSockets[sock].u16SessionID)
			{
				if((s16RecvStatus > 0) && (s16RecvStatus < u16BufferSize))
				{
					/* Skip incoming bytes until reaching the Start of Application Data. 
					*/
					u32Address += u16DataOffset;

					/* Read the Application data and deliver it to the application callback in
					the given application buffer. If the buffer is smaller than the received data,
					the data is passed to the application in chunks according to its buffer size.
					*/
					u16ReadSize = (unsigned short)s16RecvStatus;
					Socket_ReadSocketData(sock, &strRecvMsg, u8CallbackMsgID, u32Address, u16ReadSize);
				}
				else
				{
					strRecvMsg.s16BufferSize	= s16RecvStatus;
					strRecvMsg.pu8Buffer		= NULL;
					if(gpfAppSocketCb)
						gpfAppSocketCb(sock,u8CallbackMsgID, &strRecvMsg);
				}
			}
			else
			{
				M2M_DBG("Discard recv callback %d %d \r\n",u16SessionID , gastrSockets[sock].u16SessionID);
				if(u16ReadSize < u16BufferSize)
					hif_receive(0, NULL, 0, 1);
			}
		}
	}
	else if((u8OpCode == SOCKET_CMD_SEND) || (u8OpCode == SOCKET_CMD_SENDTO) || (u8OpCode == SOCKET_CMD_SSL_SEND))
	{
		SOCKET			sock;
		signed short			s16Rcvd;
		tstrSendReply	strReply;
		unsigned char			u8CallbackMsgID = SOCKET_MSG_SEND;

		if(u8OpCode == SOCKET_CMD_SENDTO)
			u8CallbackMsgID = SOCKET_MSG_SENDTO;

		if(hif_receive(u32Address, (unsigned char*)&strReply, sizeof(tstrSendReply), 0) == M2M_SUCCESS)
		{
			unsigned short u16SessionID = 0;
			
			sock = strReply.sock;
			u16SessionID = strReply.u16SessionID;
			M2M_DBG("send callback session ID = %d\r\n",u16SessionID);
			
			s16Rcvd = NM_BSP_B_L_16(strReply.s16SentBytes);

			if(u16SessionID == gastrSockets[sock].u16SessionID)
			{
				if(gpfAppSocketCb)
					gpfAppSocketCb(sock,u8CallbackMsgID, &s16Rcvd);
			}
			else
			{
				M2M_DBG("Discard send callback %d %d \r\n",u16SessionID , gastrSockets[sock].u16SessionID);
			}
		}
	}
	else if(u8OpCode == SOCKET_CMD_PING)
	{
		tstrPingReply	strPingReply;
		if(hif_receive(u32Address, (unsigned char*)&strPingReply, sizeof(tstrPingReply), 1) == M2M_SUCCESS)
		{
			gfpPingCb = (void (*)(unsigned long , unsigned long , unsigned char))strPingReply.u32CmdPrivate;
			if(gfpPingCb != NULL)
			{
				gfpPingCb(strPingReply.u32IPAddr, strPingReply.u32RTT, strPingReply.u8ErrorCode);
			}
		}
	}
}
/*********************************************************************
Function
		socketInit

Description

Return
		None.

Author
		Ahmed Ezzat

Version
		1.0

Date
		4 June 2012
*********************************************************************/
void socketInit(void)
{
	if(gbSocketInit==0)
	{
		m2m_memset((unsigned char*)gastrSockets, 0, MAX_SOCKET * sizeof(tstrSocket));
		//????hif_register_cb(M2M_REQ_GRP_IP,m2m_ip_cb);
		gbSocketInit=1;
		gu16SessionID = 0;
	}
}

/*********************************************************************
Function
		socketDeinit

Description 

Return
		None.

Author
		Samer Sarhan

Version
		1.0

Date
		27 Feb 2015
*********************************************************************/
void socketDeinit(void)
{	
	m2m_memset((unsigned char*)gastrSockets, 0, MAX_SOCKET * sizeof(tstrSocket));
	//????hif_register_cb(M2M_REQ_GRP_IP, NULL);
	gpfAppSocketCb = NULL;
	gpfAppResolveCb = NULL;
	gbSocketInit = 0;
}


/*********************************************************************
Function
		registerSocketCallback

Description

Return
		None.

Author
		Ahmed Ezzat

Versio
		1.0

Date
		4 June 2012
*********************************************************************/
void registerSocketCallback(tpfAppSocketCb pfAppSocketCb, tpfAppResolveCb pfAppResolveCb)
{
	gpfAppSocketCb = pfAppSocketCb;
	gpfAppResolveCb = pfAppResolveCb;
}

/*********************************************************************
Function
		socket

Description
		Creates a socket.

Return
		- Negative value for error.
		- ZERO or positive value as a socket ID if successful.

Author
		Ahmed Ezzat

Version
		1.0

Date
		4 June 2012
*********************************************************************/
SOCKET socket(unsigned short u16Domain, unsigned char u8Type, unsigned char u8Flags)
{
	SOCKET	sock = -1;
	unsigned char	u8Count,u8SocketCount = MAX_SOCKET;

	/* The only supported family is the AF_INET for UDP and TCP transport layer protocols. */
	if(u16Domain == AF_INET)
	{
		if(u8Type == SOCK_STREAM)
		{
			u8SocketCount = TCP_SOCK_MAX;
			u8Count = 0;
		}
		else if(u8Type == SOCK_DGRAM)
		{
			/*--- UDP SOCKET ---*/
			u8SocketCount = MAX_SOCKET;
			u8Count = TCP_SOCK_MAX;
		}
		else
			return sock;

		for(;u8Count < u8SocketCount; u8Count ++)
		{
			if(gastrSockets[u8Count].bIsUsed == 0)
			{
				gastrSockets[u8Count].bIsUsed = 1;
				
				/* The session ID is used to distinguish different socket connections
					by comparing the assigned session ID to the one reported by the firmware*/
				++gu16SessionID;
				if(gu16SessionID == 0)
					++gu16SessionID;
				
				gastrSockets[u8Count].u16SessionID = gu16SessionID;
				M2M_DBG("1 Socket %d session ID = %d\r\n",u8Count, gu16SessionID );
				sock = (SOCKET)u8Count;

				if(u8Flags & SOCKET_FLAGS_SSL)
				{
					tstrSSLSocketCreateCmd	strSSLCreate;
					strSSLCreate.sslSock = sock;
					gastrSockets[u8Count].u8SSLFlags = SSL_FLAGS_ACTIVE;
					SOCKET_REQUEST(SOCKET_CMD_SSL_CREATE, (unsigned char*)&strSSLCreate, sizeof(tstrSSLSocketCreateCmd), 0, 0, 0);
				}
				break;
			}
		}
	}
	return sock;
}
/*********************************************************************
Function
		bind

Description
		Request to bind a socket on a local address.

Return


Author
		Ahmed Ezzat

Version
		1.0

Date
		5 June 2012
*********************************************************************/
signed char bind(SOCKET sock, struct sockaddr *pstrAddr, unsigned char u8AddrLen)
{
	signed char	s8Ret = SOCK_ERR_INVALID_ARG;
	if((pstrAddr != NULL) && (sock >= 0) && (gastrSockets[sock].bIsUsed == 1) && (u8AddrLen != 0))
	{
		tstrBindCmd			strBind;

		/* Build the bind request. */
		strBind.sock = sock;
		m2m_memcpy((unsigned char *)&strBind.strAddr, (unsigned char *)pstrAddr, sizeof(tstrSockAddr));
		//strBind.strAddr = *((tstrSockAddr*)pstrAddr);

		//strBind.strAddr.u16Family	= strBind.strAddr.u16Family;
		//strBind.strAddr.u16Port		= strBind.strAddr.u16Port;
		//strBind.strAddr.u32IPAddr	= strBind.strAddr.u32IPAddr;
		strBind.u16SessionID		= gastrSockets[sock].u16SessionID;
		
		/* Send the request. */
		s8Ret = SOCKET_REQUEST(SOCKET_CMD_BIND, (unsigned char*)&strBind,sizeof(tstrBindCmd) , NULL , 0, 0);
		if(s8Ret != SOCK_ERR_NO_ERROR)
		{
			s8Ret = SOCK_ERR_INVALID;
		}
	}
	return s8Ret;
}
/*********************************************************************
Function
		listen

Description


Return


Author
		Ahmed Ezzat

Version
		1.0

Date
		5 June 2012
*********************************************************************/
signed char listen(SOCKET sock, unsigned char backlog)
{
	signed char	s8Ret = SOCK_ERR_INVALID_ARG;
	
	if(sock >= 0 && (gastrSockets[sock].bIsUsed == 1))
	{
		tstrListenCmd		strListen;

		strListen.sock = sock;
		strListen.u8BackLog = backlog;
		strListen.u16SessionID		= gastrSockets[sock].u16SessionID;

		s8Ret = SOCKET_REQUEST(SOCKET_CMD_LISTEN, (unsigned char*)&strListen, sizeof(tstrListenCmd), NULL, 0, 0);
		if(s8Ret != SOCK_ERR_NO_ERROR)
		{
			s8Ret = SOCK_ERR_INVALID;
		}
	}
	return s8Ret;
}
/*********************************************************************
Function
		accept

Description

Return


Author
		Ahmed Ezzat

Version
		1.0

Date
		5 June 2012
*********************************************************************/
signed char accept(SOCKET sock, struct sockaddr *addr, unsigned char *addrlen)
{
	signed char	s8Ret = SOCK_ERR_INVALID_ARG;
	
	if(sock >= 0 && (gastrSockets[sock].bIsUsed == 1) )
	{
		s8Ret = SOCK_ERR_NO_ERROR;
	}
	return s8Ret;
}
/*********************************************************************
Function
		connect

Description
		Connect to a remote TCP Server.

Return


Author
		Ahmed Ezzat

Version
		1.0

Date
		5 June 2012
*********************************************************************/
signed char connect(SOCKET sock, struct sockaddr *pstrAddr, unsigned char u8AddrLen)
{
	signed char	s8Ret = SOCK_ERR_INVALID_ARG;
	if((sock >= 0) && (pstrAddr != NULL) && (gastrSockets[sock].bIsUsed == 1) && (u8AddrLen != 0))
	{
		tstrConnectCmd	strConnect;
		unsigned char			u8Cmd = SOCKET_CMD_CONNECT;
		if((gastrSockets[sock].u8SSLFlags) & SSL_FLAGS_ACTIVE)
		{
			u8Cmd = SOCKET_CMD_SSL_CONNECT;
			strConnect.u8SslFlags = gastrSockets[sock].u8SSLFlags;
		}
		strConnect.sock = sock;
		m2m_memcpy((unsigned char *)&strConnect.strAddr, (unsigned char *)pstrAddr, sizeof(tstrSockAddr));

		//strConnect.strAddr.u16Family	= strConnect.strAddr.u16Family;
		//strConnect.strAddr.u16Port		= strConnect.strAddr.u16Port;
		//strConnect.strAddr.u32IPAddr	= strConnect.strAddr.u32IPAddr;
		strConnect.u16SessionID		= gastrSockets[sock].u16SessionID;
		s8Ret = SOCKET_REQUEST(u8Cmd, (unsigned char*)&strConnect,sizeof(tstrConnectCmd), NULL, 0, 0);
		if(s8Ret != SOCK_ERR_NO_ERROR)
		{
			s8Ret = SOCK_ERR_INVALID;
		}
	}
	return s8Ret;
}
/*********************************************************************
Function
		send

Description

Return

Author
		Ahmed Ezzat

Version
		1.0

Date
		5 June 2012
*********************************************************************/
signed short send(SOCKET sock, void *pvSendBuffer, unsigned short u16SendLength, unsigned short flags)
{
	signed short	s16Ret = SOCK_ERR_INVALID_ARG;
	
	if((sock >= 0) && (pvSendBuffer != NULL) && (u16SendLength <= SOCKET_BUFFER_MAX_LENGTH) && (gastrSockets[sock].bIsUsed == 1))
	{
		unsigned short			u16DataOffset;
		tstrSendCmd		strSend;
		unsigned char			u8Cmd;

		u8Cmd			= SOCKET_CMD_SEND;
		u16DataOffset	= TCP_TX_PACKET_OFFSET;

		strSend.sock		= sock;
		strSend.u16DataSize	= NM_BSP_B_L_16(u16SendLength);
		strSend.u16SessionID		= gastrSockets[sock].u16SessionID;

		if(sock >= TCP_SOCK_MAX)
		{
			u16DataOffset = UDP_TX_PACKET_OFFSET;
		}
		if(gastrSockets[sock].u8SSLFlags & SSL_FLAGS_ACTIVE)
		{
			u8Cmd			= SOCKET_CMD_SSL_SEND;
			u16DataOffset	= SSL_TX_PACKET_OFFSET;
		}

		s16Ret =  SOCKET_REQUEST(u8Cmd|M2M_REQ_DATA_PKT, (unsigned char*)&strSend, sizeof(tstrSendCmd), pvSendBuffer, u16SendLength, u16DataOffset);
		if(s16Ret != SOCK_ERR_NO_ERROR)
		{
			s16Ret = SOCK_ERR_BUFFER_FULL;
		}
	}
	return s16Ret;
}
/*********************************************************************
Function
		sendto

Description

Return

Author
		Ahmed Ezzat

Version
		1.0

Date
		4 June 2012
*********************************************************************/
signed short sendto(SOCKET sock, void *pvSendBuffer, unsigned short u16SendLength, unsigned short flags, struct sockaddr *pstrDestAddr, unsigned char u8AddrLen)
{
	signed short	s16Ret = SOCK_ERR_INVALID_ARG;
	
	if((sock >= 0) && (pvSendBuffer != NULL) && (u16SendLength <= SOCKET_BUFFER_MAX_LENGTH) && (gastrSockets[sock].bIsUsed == 1))
	{
		if(gastrSockets[sock].bIsUsed)
		{
			tstrSendCmd	strSendTo;

			m2m_memset((unsigned char*)&strSendTo, 0, sizeof(tstrSendCmd));

			strSendTo.sock			= sock;
			strSendTo.u16DataSize	= NM_BSP_B_L_16(u16SendLength);
			strSendTo.u16SessionID		= gastrSockets[sock].u16SessionID;
			
			if(pstrDestAddr != NULL)
			{
				struct sockaddr_in	*pstrAddr;
				pstrAddr = (void*)pstrDestAddr;

				strSendTo.strAddr.u16Family	= pstrAddr->sin_family;
				strSendTo.strAddr.u16Port	= pstrAddr->sin_port;
				strSendTo.strAddr.u32IPAddr	= pstrAddr->sin_addr.s_addr;
			}
			//????s16Ret = SOCKET_REQUEST(SOCKET_CMD_SENDTO|M2M_REQ_DATA_PKT, (unsigned char*)&strSendTo,  sizeof(tstrSendCmd),
				//pvSendBuffer, u16SendLength, UDP_TX_PACKET_OFFSET);

			if(s16Ret != SOCK_ERR_NO_ERROR)
			{
				s16Ret = SOCK_ERR_BUFFER_FULL;
			}
		}
	}
	return s16Ret;
}
/*********************************************************************
Function
		recv

Description

Return


Author
		Ahmed Ezzat

Version
		1.0
		2.0  9 April 2013 --> Add timeout for recv operation.

Date
		5 June 2012
*********************************************************************/
signed short recv(SOCKET sock, void *pvRecvBuf, unsigned short u16BufLen, unsigned long u32Timeoutmsec)
{
	signed short	s16Ret = SOCK_ERR_INVALID_ARG;
	
	if((sock >= 0) && (pvRecvBuf != NULL) && (u16BufLen != 0) && (gastrSockets[sock].bIsUsed == 1))
	{
		s16Ret = SOCK_ERR_NO_ERROR;
		gastrSockets[sock].pu8UserBuffer 		= (unsigned char*)pvRecvBuf;
		gastrSockets[sock].u16UserBufferSize 	= u16BufLen;

		if(!gastrSockets[sock].bIsRecvPending)
		{
			tstrRecvCmd	strRecv;
			unsigned char		u8Cmd = SOCKET_CMD_RECV;

			gastrSockets[sock].bIsRecvPending = 1;
			if(gastrSockets[sock].u8SSLFlags & SSL_FLAGS_ACTIVE)
			{
				u8Cmd = SOCKET_CMD_SSL_RECV;
			}

			/* Check the timeout value. */
			if(u32Timeoutmsec == 0)
				strRecv.u32Timeoutmsec = 0xFFFFFFFF;
			else
				strRecv.u32Timeoutmsec = NM_BSP_B_L_32(u32Timeoutmsec);
			strRecv.sock = sock;
			strRecv.u16SessionID		= gastrSockets[sock].u16SessionID;
		
			s16Ret = SOCKET_REQUEST(u8Cmd, (unsigned char*)&strRecv, sizeof(tstrRecvCmd), NULL , 0, 0);
			if(s16Ret != SOCK_ERR_NO_ERROR)
			{
				s16Ret = SOCK_ERR_BUFFER_FULL;
			}
		}
	}
	return s16Ret;
}
/*********************************************************************
Function
		close

Description

Return
		None.

Author
		Ahmed Ezzat

Version
		1.0

Date
		4 June 2012
*********************************************************************/
signed char close(SOCKET sock)
{
	signed char	s8Ret = SOCK_ERR_INVALID_ARG;
	if(sock >= 0 && (gastrSockets[sock].bIsUsed == 1))
	{
		unsigned char	u8Cmd = SOCKET_CMD_CLOSE;
		tstrCloseCmd strclose;
		strclose.sock = sock; 
		strclose.u16SessionID		= gastrSockets[sock].u16SessionID;
		
		gastrSockets[sock].bIsUsed = 0;
		gastrSockets[sock].u16SessionID =0;
		
		if(gastrSockets[sock].u8SSLFlags & SSL_FLAGS_ACTIVE)
		{
			u8Cmd = SOCKET_CMD_SSL_CLOSE;
		}
		s8Ret = SOCKET_REQUEST(u8Cmd, (unsigned char*)&strclose, sizeof(tstrCloseCmd), NULL,0, 0);
		if(s8Ret != SOCK_ERR_NO_ERROR)
		{
			s8Ret = SOCK_ERR_INVALID;
		}
		m2m_memset((unsigned char*)&gastrSockets[sock], 0, sizeof(tstrSocket));
	}
	return s8Ret;
}
/*********************************************************************
Function
		recvfrom

Description

Return


Author
		Ahmed Ezzat

Version
		1.0
		2.0  9 April 2013 --> Add timeout for recv operation.

Date
		5 June 2012
*********************************************************************/
signed short recvfrom(SOCKET sock, void *pvRecvBuf, unsigned short u16BufLen, unsigned long u32Timeoutmsec)
{
	signed short	s16Ret = SOCK_ERR_NO_ERROR;
	if((sock >= 0) && (pvRecvBuf != NULL) && (u16BufLen != 0) && (gastrSockets[sock].bIsUsed == 1))
	{
		if(gastrSockets[sock].bIsUsed)
		{
			s16Ret = SOCK_ERR_NO_ERROR;
			gastrSockets[sock].pu8UserBuffer = (unsigned char*)pvRecvBuf;
			gastrSockets[sock].u16UserBufferSize = u16BufLen;

			if(!gastrSockets[sock].bIsRecvPending)
			{
				tstrRecvCmd	strRecv;

				gastrSockets[sock].bIsRecvPending = 1;

				/* Check the timeout value. */
				if(u32Timeoutmsec == 0)
					strRecv.u32Timeoutmsec = 0xFFFFFFFF;
				else
					strRecv.u32Timeoutmsec = NM_BSP_B_L_32(u32Timeoutmsec);
				strRecv.sock = sock;
				strRecv.u16SessionID		= gastrSockets[sock].u16SessionID;
				
				s16Ret = SOCKET_REQUEST(SOCKET_CMD_RECVFROM, (unsigned char*)&strRecv, sizeof(tstrRecvCmd), NULL , 0, 0);
				if(s16Ret != SOCK_ERR_NO_ERROR)
				{
					s16Ret = SOCK_ERR_BUFFER_FULL;
				}
			}
		}
	}
	else
	{
		s16Ret = SOCK_ERR_INVALID_ARG;
	}
	return s16Ret;
}
/*********************************************************************
Function
		nmi_inet_addr

Description

Return
		Unsigned 32-bit integer representing the IP address in Network
		byte order.

Author
		Ahmed Ezzat

Version
		1.0

Date
		4 June 2012
*********************************************************************/
unsigned long nmi_inet_addr(char *pcIpAddr)
{
	unsigned char	tmp;
	unsigned long	u32IP = 0;
	unsigned char	au8IP[4];
	unsigned char 	c;
	unsigned char	i, j;

	tmp = 0;

	for(i = 0; i < 4; ++i)
	{
		j = 0;
		do
		{
			c = *pcIpAddr;
			++j;
			if(j > 4)
			{
				return 0;
			}
			if(c == '.' || c == 0)
			{
				au8IP[i] = tmp;
				tmp = 0;
			}
			else if(c >= '0' && c <= '9')
			{
				tmp = (tmp * 10) + (c - '0');
			}
			else
			{
				return 0;
			}
			++pcIpAddr;
		} while(c != '.' && c != 0);
	}
	m2m_memcpy((unsigned char*)&u32IP, au8IP, 4);
	return u32IP;
}
/*********************************************************************
Function
		gethostbyname

Description

Return
		None.

Author
		Ahmed Ezzat

Version
		1.0

Date
		4 June 2012
*********************************************************************/
signed char gethostbyname(unsigned char * pcHostName)
{
	signed char	s8Err = SOCK_ERR_INVALID_ARG;
	unsigned char	u8HostNameSize = (unsigned char)m2m_strlen(pcHostName);
	if(u8HostNameSize <= HOSTNAME_MAX_SIZE)
	{
		s8Err = SOCKET_REQUEST(SOCKET_CMD_DNS_RESOLVE|M2M_REQ_DATA_PKT, (unsigned char*)pcHostName, u8HostNameSize + 1, NULL,0, 0);
		if(s8Err != SOCK_ERR_NO_ERROR)
		{
			s8Err = SOCK_ERR_INVALID;
		}
	}
	return s8Err;
}
/*********************************************************************
Function
		setsockopt

Description

Return
		None.

Author
		Abdelrahman Diab

Version
		1.0

Date
		9 September 2014
*********************************************************************/
signed char sslSetSockOpt(SOCKET sock, unsigned char  u8Opt, const void *pvOptVal, unsigned short u16OptLen)
{
	signed char	s8Ret = SOCK_ERR_INVALID_ARG;
	if(sock < TCP_SOCK_MAX)
	{
		if(gastrSockets[sock].u8SSLFlags & SSL_FLAGS_ACTIVE)
		{
			if(u8Opt == SO_SSL_BYPASS_X509_VERIF)
			{
				int	optVal = *((int*)pvOptVal);
				if(optVal)
				{
					gastrSockets[sock].u8SSLFlags |= SSL_FLAGS_BYPASS_X509;
				}
				else
				{
					gastrSockets[sock].u8SSLFlags &= ~SSL_FLAGS_BYPASS_X509;
				}
				s8Ret = SOCK_ERR_NO_ERROR;
			}
			else if(u8Opt == SO_SSL_SNI)
			{
				if(u16OptLen < HOSTNAME_MAX_SIZE)
				{
					unsigned char					*pu8SNI = (unsigned char*)pvOptVal;
					tstrSSLSetSockOptCmd	strCmd;

					strCmd.sock			= sock;
					strCmd.u16SessionID	= gastrSockets[sock].u16SessionID;
					strCmd.u8Option		= u8Opt;
					strCmd.u32OptLen	= u16OptLen;
					m2m_memcpy(strCmd.au8OptVal, pu8SNI, HOSTNAME_MAX_SIZE);
					
					//????if(SOCKET_REQUEST(SOCKET_CMD_SSL_SET_SOCK_OPT, (unsigned char*)&strCmd, sizeof(tstrSSLSetSockOptCmd),
						//0, 0, 0) == M2M_ERR_MEM_ALLOC)
					//{
						//????s8Ret = SOCKET_REQUEST(SOCKET_CMD_SSL_SET_SOCK_OPT | M2M_REQ_DATA_PKT,
							//(unsigned char*)&strCmd, sizeof(tstrSSLSetSockOptCmd), 0, 0, 0);
					//}
					s8Ret = SOCK_ERR_NO_ERROR;
				}
				else
				{
					M2M_ERR("SNI Exceeds Max Length\n");
				}
			}
			else
			{
				M2M_ERR("Unknown SSL Socket Option %d\n",u8Opt);
			}
		}
		else
		{
			M2M_ERR("Not SSL Socket\n");
		}
	}
	return s8Ret;
}
/*********************************************************************
Function
		setsockopt

Description

Return
		None.

Author
		Abdelrahman Diab

Version
		1.0

Date
		9 September 2014
*********************************************************************/
signed char setsockopt(SOCKET sock, unsigned char  u8Level, unsigned char  option_name,
       const void *option_value, unsigned short u16OptionLen)
{
	signed char	s8Ret = SOCK_ERR_INVALID_ARG;
	if((sock >= 0)  && (option_value != NULL)  && (gastrSockets[sock].bIsUsed == 1))
	{
		if(u8Level == SOL_SSL_SOCKET)
		{
			s8Ret = sslSetSockOpt(sock, option_name, option_value, u16OptionLen);
		}
		else
		{
			unsigned char	u8Cmd = SOCKET_CMD_SET_SOCKET_OPTION;
			tstrSetSocketOptCmd strSetSockOpt;
			strSetSockOpt.u8Option=option_name;
			strSetSockOpt.sock = sock; 
			strSetSockOpt.u32OptionValue = *(unsigned long*)option_value;
			strSetSockOpt.u16SessionID		= gastrSockets[sock].u16SessionID;

			s8Ret = SOCKET_REQUEST(u8Cmd, (unsigned char*)&strSetSockOpt, sizeof(tstrSetSocketOptCmd), NULL,0, 0);
			if(s8Ret != SOCK_ERR_NO_ERROR)
			{
				s8Ret = SOCK_ERR_INVALID;
			}
		}
	}
	return s8Ret;	
}
/*********************************************************************
Function
		getsockopt

Description

Return
		None.

Author
		Ahmed Ezzat

Version
		1.0

Date
		24 August 2014
*********************************************************************/
signed char getsockopt(SOCKET sock, unsigned char u8Level, unsigned char u8OptName, const void *pvOptValue, unsigned char* pu8OptLen)
{
	/* TBD */
	return M2M_SUCCESS;
}
/*********************************************************************
Function
	m2m_ping_req

Description
	Send Ping request.

Return
	
Author
	Ahmed Ezzat

Version
	1.0

Date
	4 June 2015
*********************************************************************/
signed char m2m_ping_req(unsigned long u32DstIP, unsigned char u8TTL, tpfPingCb fpPingCb)
{
	signed char	s8Ret = M2M_ERR_INVALID_ARG;

	if((u32DstIP != 0) && (fpPingCb != NULL))
	{
		tstrPingCmd	strPingCmd;

		strPingCmd.u16PingCount		= 1;
		strPingCmd.u32DestIPAddr	= u32DstIP;
		strPingCmd.u32CmdPrivate	= (unsigned long)fpPingCb;
		strPingCmd.u8TTL			= u8TTL;

		s8Ret = SOCKET_REQUEST(SOCKET_CMD_PING, (unsigned char*)&strPingCmd, sizeof(tstrPingCmd), NULL, 0, 0);
	}
	return s8Ret;
}
