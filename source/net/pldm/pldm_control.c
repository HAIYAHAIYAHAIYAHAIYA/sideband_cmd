#include "pldm_control.h"
#include "pldm.h"
#include "mctp.h"
#include "pldm_monitor.h"

extern pldm_monitor_base_info_t g_pldm_monitor_info;

void pldm_unsupport_cmd(protocol_msg_t *pkt, int *pkt_len)
{
    LOG("the cmd is not supported");
    pldm_response_t *rsp = (pldm_response_t *)(pkt->rsp_buf - sizeof(pldm_response_t));
    rsp->cpl_code = MCTP_COMMAND_UNSUPPORT;
}

void pldm_ctrl_set_tid(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_set_tid_req_dat_t *req_dat = (pldm_set_tid_req_dat_t *)(pkt->req_buf);
    pldm_response_t *rsp_hdr = (pldm_response_t *)(pkt->rsp_buf - sizeof(pldm_response_t));

    if (req_dat->tid == 0x0 || req_dat->tid == 0xff) {
        rsp_hdr->cpl_code = MCTP_COMMAND_INVALIDDATA;
        return;
    }

    g_pldm_monitor_info.tid = req_dat->tid;
    terminus_locator_pdr_chg();
    LOG("%s tid : %#x", __FUNCTION__, req_dat->tid);
}

void pldm_ctrl_get_tid(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_get_tid_rsp_dat_t *rsp_dat = (pldm_get_tid_rsp_dat_t *)(pkt->rsp_buf);
    rsp_dat->tid = g_pldm_monitor_info.tid;
    LOG("%s tid : %#x", __FUNCTION__, rsp_dat->tid);

    *pkt_len += sizeof(pldm_get_tid_rsp_dat_t);
}

static void pldm_ctrl_get_ver(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_get_ver_req_dat_t *req_dat = (pldm_get_ver_req_dat_t *)(pkt->req_buf);
    pldm_get_ver_rsp_dat_t *rsp_dat = (pldm_get_ver_rsp_dat_t *)(pkt->rsp_buf);
    pldm_response_t *rsp_hdr = (pldm_response_t *)(pkt->rsp_buf - sizeof(pldm_response_t));
    if (req_dat->trans_op_flag != GET_FIRST_PART) {
        rsp_hdr->cpl_code = MCTP_PLDM_INVALID_TRANSFER_OPERATION_FLAG;
    }

    u32 pldm_type = req_dat->pldm_type;
    switch (pldm_type) {
        case PLDM_MESSAGING_CONTROL_AND_DISCOVERY :
            rsp_dat->pldm_ver = PLDM_TYPE_0_VERSION;
            break;

        case PLDM_FOR_PLATFORM_MONITORING_AND_CONTROL :
            rsp_dat->pldm_ver = PLDM_TYPE_2_VERSION;
            break;

        case PLDM_FOR_FRU_DATA :
            rsp_dat->pldm_ver = PLDM_TYPE_4_VERSION;
            break;

        case PLDM_FOR_FIRMWARE_UPDATE :
            rsp_dat->pldm_ver = PLDM_TYPE_5_VERSION;
            break;

        case PLDM_FOR_REDFISH_DEVICE_ENABLEMENT :
            rsp_dat->pldm_ver = PLDM_TYPE_6_VERSION;
            break;

        default :
            rsp_hdr->cpl_code = MCTP_PLDM_INVALID_PLDM_TYPE_IN_REQUEST_DATA;
            rsp_dat->pldm_ver = PLDM_TYPE_0_VERSION;
            break;
    }
    rsp_dat->next_dat_trans_handle = 0x00;
    rsp_dat->trans_flag = STARTANDEND;
    rsp_dat->checksum = crc32_pldm(0xFFFFFFFFUL, (u8 *)(&rsp_dat->pldm_ver), 4);

    *pkt_len += sizeof(pldm_get_ver_rsp_dat_t);
}

static void pldm_ctrl_get_type(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_get_type_rsp_dat_t *rsp_dat = (pldm_get_type_rsp_dat_t *)(pkt->rsp_buf);
    u8 pldmtype = 0;
    pldmtype = CBIT(0) | CBIT(2) | CBIT(4) | CBIT(5) | CBIT(6);    // MCTP_PLDM_CONTROL, MCTP_PLDM_MONITOR, MCTP_PLDM_FRU_DATA, MCTP_PLDM_UPDATE, MCTP_PLDM_REDFISH

    rsp_dat->pldmtype = pldmtype;
    LOG("%s pldmtype : %#x", __FUNCTION__, pldmtype);
    *pkt_len += sizeof(pldm_get_type_rsp_dat_t);
}

static void pldm_ctrl_get_cmd(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_get_cmd_req_dat_t *req_dat = (pldm_get_cmd_req_dat_t *)(pkt->req_buf);
    pldm_get_cmd_rsp_dat_t *rsp_dat = (pldm_get_cmd_rsp_dat_t *)(pkt->rsp_buf);
    pldm_response_t *rsp_hdr = (pldm_response_t *)(pkt->rsp_buf - sizeof(pldm_response_t));
    u8 pldm_type = req_dat->pldm_type;
    u32 version = req_dat->ver;

    u32 *pldm_command = (u32 *)rsp_dat->pldm_cmd;
    switch (pldm_type) {
        case MCTP_PLDM_CONTROL:
            if (version == PLDM_TYPE_0_VERSION) {
                pldm_command[0] = 0x3E;
            } else {
                rsp_hdr->cpl_code = MCTP_PLDM_INVALID_PLDM_VERSION_IN_REQUEST_DATA;
            }
            break;

        case MCTP_PLDM_MONITOR:
            if (version == PLDM_TYPE_2_VERSION) {
                pldm_command[0] = 0x002F3C38;
                pldm_command[1] = 0x00000003;
                pldm_command[2] = 0x000b0000;
            } else {
                rsp_hdr->cpl_code = MCTP_PLDM_INVALID_PLDM_VERSION_IN_REQUEST_DATA;
            }
            break;

        case MCTP_PLDM_FRU_DATA:
            if (version == PLDM_TYPE_4_VERSION) {
                pldm_command[0] = 0x6;
            } else {
                rsp_hdr->cpl_code = MCTP_PLDM_INVALID_PLDM_VERSION_IN_REQUEST_DATA;
            }
            break;

        case MCTP_PLDM_UPDATE:
            if (version == PLDM_TYPE_5_VERSION) {
                pldm_command[0] = 0x3CFB0006;
            } else {
                rsp_hdr->cpl_code = MCTP_PLDM_INVALID_PLDM_VERSION_IN_REQUEST_DATA;
            }
            break;

        case MCTP_PLDM_REDFISH:
            if (version == PLDM_TYPE_6_VERSION) {
                pldm_command[0] = 0x007F1FFE;
                pldm_command[1] = 0x00030000;
            } else {
                rsp_hdr->cpl_code = MCTP_PLDM_INVALID_PLDM_VERSION_IN_REQUEST_DATA;
            }
            break;

        default:
            rsp_hdr->cpl_code = MCTP_PLDM_INVALID_PLDM_TYPE_IN_REQUEST_DATA;
            break;
    }

    *pkt_len += sizeof(pldm_get_cmd_rsp_dat_t);
}

static pldm_cmd_func pldm_cmd_table[PLDM_CTRL_CMD] =
{
    pldm_unsupport_cmd,
    pldm_ctrl_set_tid,
    pldm_ctrl_get_tid,
    pldm_ctrl_get_ver,
    pldm_ctrl_get_type,
    pldm_ctrl_get_cmd,
};

void pldm_control_process(protocol_msg_t *pkt, int *pkt_len, u32 cmd_code)
{
    pldm_cmd_func cmd_proc = NULL;

    if (cmd_code < PLDM_CTRL_CMD) {
        cmd_proc = pldm_cmd_table[cmd_code];
    } else {
        cmd_proc = pldm_unsupport_cmd;
    }

    return cmd_proc(pkt, pkt_len);
}

// u32 crc32_pldm(u8 *data, u32 len)
// {
//     u32 crc = 0xFFFFFFFF;

//     while (len--) {
//         crc ^= (unsigned int)(*data++) << 24;
//         for (int i = 0; i < 8; ++i)
//         {
//             if (crc & 0x80000000)
//                 crc = (crc << 1) ^ 0x04C11DB7;
//             else
//                 crc <<= 1;
//         }
//     }
//     return crc;
// }

#define PLDM_POLY 0xEDB88320UL
// u32 crc = 0xFFFFFFFFUL;
u32 crc32_pldm(u32 init_crc, u8 *data, u32 len)
{
    for (size_t i = 0; i < len; i++) {
        init_crc ^= data[i];        // 把当前字节与 crc 的低 8 位进行异或操作

        // 处理当前字节的 8 位，每次处理一位
        for (int j = 0; j < 8; j++) {
            if (init_crc & 1) {      // 如果 crc 的最低位为 1，则右移并与多项式除数进行异或操作
                init_crc = (init_crc >> 1) ^ PLDM_POLY;
            } else {            // 否则，只右移一个比特位
                init_crc >>= 1;
            }
        }
    }

    return ~init_crc;                // 取反操作得到最终结果
}