// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <stdarg.h>
#include <stdio.h>
#include "azure_c_shared_utility/xlogging.h"

#if defined _MSC_VER
#define FUNC_NAME __FUNCDNAME__
#else
#define FUNC_NAME __func__
#endif

void consolelogger_log(LOG_CATEGORY log_category,unsigned int options, const char* format, ...)
{
	va_list args;
	va_start(args, format);

    time_t t = time(NULL); 
    
    switch (log_category)
    {
    case LOG_INFO:
        (void)printf("Info: ");
        break;
    case LOG_ERROR:
        (void)printf("Error: Time:%.24s File:%s Func:%s Line:%d ", ctime(&t), __FILE__, FUNC_NAME, __LINE__);
        break;
    default:
        break;
    }

	(void)vprintf(format, args);
	va_end(args);

    (void)log_category;
	if (options & LOG_LINE)
	{
		(void)printf("\r\n");
	}
}
