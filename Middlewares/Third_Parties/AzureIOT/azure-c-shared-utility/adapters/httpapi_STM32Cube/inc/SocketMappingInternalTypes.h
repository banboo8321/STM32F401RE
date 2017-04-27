#ifndef __SOCKET_INTERNAL_TYPES
#define __SOCKET_INTERNAL_TYPES

#include "InternetInterfaceTypes.h"

#define SOCKETHANDLE                    uint8_t
#define INVALID_SOCKET_HANDLE           0xFF
#define SOCKETHANDLE_UNIVERSAL          0x0c //FIXME: This is due to uncorrect list id socket

typedef struct __TSocketMapping_t
{
  SOCKETSYSTEMHANDLE    realHandle;
  SOCKETHANDLE          mappedSocket;
  TSocketFlag           isSocketSet;
  TSocketStatus         socketStatus;
  TSocketConnectionType socketConnectionType;
}TSocketMapping;

#endif
