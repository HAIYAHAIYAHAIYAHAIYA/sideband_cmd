#ifndef __MAIN_H__
#define __MAIN_H__

#include "type.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include "time.h"

#define VDM_SPLIT_MAX_LEN               (48)
#define MAX_LAN_NUM                     (4)
#define UPGRADE_MODE                    CBIT(1)
#define CM_LOG_FILTER                   (0)

#define cm_memcpy                       memcpy
#define cm_memset                       memset
#define cm_memcmp                       memcmp
#define cm_strncpy                      strncpy
#define cm_strlen                       strlen
#define cm_strcmp                       strcmp
#define cm_sprintf                      sprintf
#define cm_snprintf                     snprintf

#define CM_MODULE_GET_TEMPERATURE_DATE(port)        (0)
#define CM_MODULE_GET_VOLTAGE_DATA(port)            (0)
#define CM_MODULE_GET_POWER_DATA(port)              (0)
#define CM_MODULE_GET_IDENTIFIER(port)              (0)
/* sign  | temperature hign . temperature low */
/* bit 1 | bit 7 | bit 8 */
#define CM_MODULE_GET_WARN_DATA(port)               (0)
#define CM_MODULE_GET_CRITICAL_DATA(port)           (0)
#define CM_MODULE_GET_FATAL_DATA(port)              (0)
/* example : return is 25, 10, 40 (G) */
#define CM_MODULE_GET_SIGNAL_RATE_DATA(port)        (0)

/* TBD */
#define CM_NIC_GET_TEMPERATURE_DATE                 (0)
#define CM_NIC_GET_WARN_DATA                        (100)
#define CM_NIC_GET_CRITICAL_DATA                    (110)
#define CM_NIC_GET_FATAL_DATA                       (120)

#define LINK_DOWN                               0
#define LINK_UP                                 1

#define CM_PLDM_FWUP_START_UPDATE() \
do {\
} while(0)

#define CM_PLDM_FWUP_END_UPDATE(log_filter_temp) \
do {\
} while(0)

#define CM_GET_CUR_TIMER_MS() \
({\
    u64 _ms = clock();\
    _ms;\
})

typedef int sts_t;

extern FILE *g_fp;

#if 1
    #define LOG(...)               \
    do { \
        printf(__VA_ARGS__);\
        printf("\r\n");\
        if (!g_fp) {\
            g_fp = fopen("log.txt", "w+"); \
        }\
        fprintf(g_fp, "[%20s][%06d]", __FUNCTION__, __LINE__);\
        fprintf(g_fp, __VA_ARGS__);\
        fprintf(g_fp, "\r\n"); \
    }while(0)
#else 
    #define LOG(...)
#endif

#endif /* __MAIN_H__ */