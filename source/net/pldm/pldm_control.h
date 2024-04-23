#ifndef __PLDM_CONTROL_H__
#define __PLDM_CONTROL_H__

#include "protocol_device.h"

#define PLDM_CTRL_CMD       0x6
#define PLDM_TID_RSVD       0xFF
#define PLDM_TID_UNASSIGN   0x0

typedef enum {
    GET_NEXT_PART = 0x0,
    GET_FIRST_PART,
} pldm_getver_trans_op_flag_t;  /* pldm get version, request, transfer operation flag */

typedef enum {
    START = 0x1,
    MIDDLE,
    END = 0x4,
    STARTANDEND,
} pldm_getver_trans_flag_t;     /* pldm get version, response, transfer flag */

#pragma pack(1)
typedef struct {
    u8 tid;
} pldm_set_tid_req_dat_t, pldm_get_tid_rsp_dat_t;

typedef struct {
    u32 dat_trans_handle;  /*  This field is a handle that is used to identify PLDM version data transfer.
                                This handle is ignored by the responder when the TransferOperationFlag is set to GetFirstPart. */
    u8 trans_op_flag;       /* transfer operation flag, This field is an operation flag that indicates whether this is the start of the transfer. see trans_op_flag_t */
    u8 pldm_type;           /* This field identifies the PLDM Type whose version information is being requested. */
} pldm_get_ver_req_dat_t;

typedef struct {
    u32 next_dat_trans_handle;
    u8 trans_flag;
    u32 pldm_ver;
    u32 checksum;
} pldm_get_ver_rsp_dat_t;

typedef struct {
    u8 pldmtype;
} pldm_get_type_rsp_dat_t;

typedef struct {
    u8 pldm_type;
    u32 ver;
} pldm_get_cmd_req_dat_t;

typedef struct {
    u8 pldm_cmd[32];
} pldm_get_cmd_rsp_dat_t;
#pragma pack()

typedef enum {
    PLDM_MESSAGING_CONTROL_AND_DISCOVERY = 0x00,
    PLDM_FOR_SMBIOS,
    PLDM_FOR_PLATFORM_MONITORING_AND_CONTROL,
    PLDM_FOR_BIOS_CONTROL_AND_CONFIGURATION,
    PLDM_FOR_FRU_DATA,
    PLDM_FOR_FIRMWARE_UPDATE,
    PLDM_FOR_REDFISH_DEVICE_ENABLEMENT,
} pldm_type_t;      /* vdm暂时只考虑0，2，5, 6的情况 */

#define PLDM_TYPE_0_VERSION     0xF1F0F000
#define PLDM_TYPE_2_VERSION     0xF1F2F000
#define PLDM_TYPE_4_VERSION     0xF1F0F000
#define PLDM_TYPE_5_VERSION     0xF1F0F000
#define PLDM_TYPE_6_VERSION     0xF1F1F000

void pldm_control_process(protocol_msg_t *pkt, int *pkt_len, u32 cmd_code);
u32 crc32_pldm(u32 init_crc, u8 *data, u32 len);

#endif