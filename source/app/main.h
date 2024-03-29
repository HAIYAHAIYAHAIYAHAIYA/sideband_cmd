#ifndef __MAIN_H__
#define __MAIN_H__

#include "type.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

#define VDM_SPLIT_MAX_LEN               (48)
#define MAX_LAN_NUM                     (2)
#define UPGRADE_MODE                    CBIT(1)
#define CM_LOG_FILTER                   (0)

#define cm_memcpy                       memcpy
#define cm_memset                       memset
#define cm_memcmp                       memcmp
#define cm_strncpy                      strncpy
#define cm_strlen                       strlen
#define cm_strcmp                       strcmp
#define cm_sprintf                      sprintf

#define CM_PLDM_FWUP_START_UPDATE() \
do {\
} while(0)

#define CM_PLDM_FWUP_END_UPDATE(log_filter_temp) \
do {\
} while(0)

#define CM_GET_CUR_TIMER_MS() \
({\
    u64 _ms = 0;\
    _ms;\
})

extern FILE *g_fp;

#if 1
    #define LOG(...)               \
    do { \
        printf(__VA_ARGS__);\
        printf("\r\n");\
        if (!g_fp) {\
            g_fp = fopen("log.txt", "w+"); \
        }\
        fprintf(g_fp, "[%s][%06d]", __FUNCTION__, __LINE__);\
        fprintf(g_fp, __VA_ARGS__);\
        fprintf(g_fp, "\r\n"); \
    }while(0)
#else 
    #define LOG(...)
#endif

#pragma pack(1)

typedef struct
{
    /* BCD encode */
    u32 version;
    u32 mode;
} sys_ctrl_t;

#pragma pach()

#endif /* __MAIN_H__ */