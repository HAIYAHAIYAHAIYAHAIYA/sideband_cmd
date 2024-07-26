#include "pkt_gen.h"
#include "mctp_ctrl_gen.h"
#include "pldm_ctrl_gen.h"
#include "pldm_fwup_gen.h"
#include "pldm_monitor_gen.h"
#include "pldm_redfish_gen.h"
#include "pldm_fru_gen.h"

#include "ncsi_gen.h"
#include "ncsi.h"

void gen_cmd_unsupport(u8 *buf)
{

}

void pldm_gen_req_hdr_update(u8 *buf, int cmd)
{
    pldm_request_t *req_hdr = (pldm_request_t *)(buf - sizeof(pldm_request_t));
    req_hdr->cmd_code = cmd;
}

void mctp_gen(int cmd, u8 *buf)
{
    mctp_ctrl_gen(cmd, buf);
}

void ncsi_gen(int cmd, u8 *buf)
{
    ncsi_cmd_gen(cmd, buf);
}

void pldm_gen_init(void)
{
    pldm_ctrl_gen_init();
    pldm_fwup_gen_init();
    pldm_monitor_gen_init();
    pldm_redfish_gen_init();
    pldm_fru_gen_init();

    ncsi_init(1);
}

u8 pldm_gen(int type, int cmd, u8 *buf)
{
    u8 ret = 0;
    pldm_request_t *req_hdr = (pldm_request_t *)buf;
    req_hdr->pldm_type = type;
    // req_hdr->cmd_code = cmd;
    buf += sizeof(pldm_request_t);

    switch (type) {
        case MCTP_PLDM_CONTROL:
            // pldm_ctrl_gen(cmd, buf);
            ret = pldm_ctrl_state_transform_switch(cmd, buf);
            break;

        case MCTP_PLDM_MONITOR:
            // pldm_monitor_gen(cmd, buf);
            ret = pldm_monitor_state_transform_switch(cmd, buf);
            break;

        case MCTP_PLDM_FRU_DATA:
            // pldm_fru_gen(cmd, buf);
            ret = pldm_fru_state_transform_switch(cmd, buf);
            break;

        case MCTP_PLDM_UPDATE:
            // pldm_fwup_gen(cmd, buf);
            ret = pldm_fwup_state_transform_switch(cmd, buf);
            break;

        case MCTP_PLDM_REDFISH:
            ret = pldm_redfish_state_transform_switch(cmd, buf);
            break;

        default:
            LOG("ERR PLDM PROTOCOL! : %d", type);
            break;
    }
    buf -= sizeof(pldm_request_t);
    return ret;
}

u8 pldm_gen_manual(int type, int cmd, u8 *buf)
{
    u8 ret = 0;
    pldm_request_t *req_hdr = (pldm_request_t *)buf;
    req_hdr->pldm_type = type;
    req_hdr->cmd_code = cmd;
    buf += sizeof(pldm_request_t);

    switch (type) {
        case MCTP_PLDM_CONTROL:
            pldm_ctrl_gen(cmd, buf);
            break;

        case MCTP_PLDM_MONITOR:
            pldm_monitor_gen(cmd, buf);
            break;

        case MCTP_PLDM_FRU_DATA:
            pldm_fru_gen(cmd, buf);
            break;

        case MCTP_PLDM_UPDATE:
            pldm_fwup_gen(cmd, buf);
            break;

        case MCTP_PLDM_REDFISH:
            pldm_redfish_gen(cmd, buf);
            break;

        default:
            LOG("ERR PLDM PROTOCOL! : %d", type);
            break;
    }
    buf -= sizeof(pldm_request_t);
    return ret;
}

void pldm_gen_recv(u8 *msg, u8 type, u8 cmd_code)
{
    switch (type) {
        case MCTP_PLDM_CONTROL:
            pldm_ctrl_gen_recv(cmd_code, msg);
            break;

        case MCTP_PLDM_MONITOR:
            pldm_monitor_gen_recv(cmd_code, msg);
            break;

        case MCTP_PLDM_FRU_DATA:
            pldm_fru_gen_recv(cmd_code, msg);
            break;

        case MCTP_PLDM_UPDATE:
            pldm_fwup_gen_recv(cmd_code, msg);
            break;

        case MCTP_PLDM_REDFISH:
            pldm_redfish_gen_recv(cmd_code, msg);
            break;

        default:
            LOG("ERR PLDM PROTOCOL!");
            break;
    }
}