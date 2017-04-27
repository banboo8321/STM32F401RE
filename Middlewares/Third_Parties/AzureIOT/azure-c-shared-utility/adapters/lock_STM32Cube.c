// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

//#include <cstdlib>
//#ifdef _CRTDBG_MAP_ALLOC
//#include <crtdbg.h>
//#endif

#include "azure_c_shared_utility/lock.h"
#include "azure_c_shared_utility/xlogging.h"

// ST
#include "FreeRTOS.h"
#include "portmacro.h"

/** @addtogroup MIDDLEWARES
* @{
*/ 

/** @addtogroup AZURE_SDK
* @{
*/ 

/** @defgroup  AZURE_ADAPTER_LOCK
  * @brief Wi-Fi User API modules
  * @{
  */

DEFINE_ENUM_STRINGS(LOCK_RESULT, LOCK_RESULT_VALUES);

static int lockID = 1;

/*SRS_LOCK_99_002:[ This API on success will return a valid lock handle which should be a non NULL value]*/
LOCK_HANDLE Lock_Init(void)
{
  return &lockID;
}


LOCK_RESULT Lock(LOCK_HANDLE handle)
{
  portENTER_CRITICAL();
  
  return LOCK_OK;
}

LOCK_RESULT Unlock(LOCK_HANDLE handle)
{
  portEXIT_CRITICAL();
  return LOCK_OK;
}

LOCK_RESULT Lock_Deinit(LOCK_HANDLE handle)
{
    return LOCK_OK;
}


/**
  * @}
  */ 


/**
  * @}
  */ 


/**
  * @}
  */ 

