#ifndef __THREAD_API_TYPES_H
#define __THREAD_API_TYPES_H

#include "azure_c_shared_utility/threadapi.h"

#define SIZE_NAMETASK 8
//COMMENT TEST

typedef enum __ThreadStatus_t
{
  THREAD_STATUS_NONE,
  THREAD_STATUS_OK,
  THREAD_STATUS_ERROR
}ThreadStatus;

typedef struct _RTOSThreadHandle_t
{
  char nameTask[SIZE_NAMETASK];

  osThreadId                            threadId;
  ThreadStatus                          isThreadActivated;
  volatile THREAD_START_FUNC            taskFunctionPointer;
  volatile void*                        taskArgsPointer;
  int                                   taskIndicator;
  osThreadDef_t                         threadDef;
  int                                   threadLastIndicator;
}RTOSThreadHandle;

typedef enum _TOperationTaskRequired_t
{
  REQUEST_NONE,
  REQUEST_RESUME
}TOperationTaskRequired;

#endif
