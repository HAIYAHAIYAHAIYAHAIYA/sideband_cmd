#include "pldm_fru_gen.h"
#include "pkt_gen.h"

extern pldm_gen_state_t gs_pldm_fru_gen_state;

static void pldm_fru_gen_recv_cmd_01(u8 *buf)
{
    pldm_fru_get_fru_record_table_metadata_rsp_dat_t *rsp_dat = (pldm_fru_get_fru_record_table_metadata_rsp_dat_t *)(buf + sizeof(pldm_response_t));
    LOG("fru_data_major_ver : %d", rsp_dat->fru_data_major_ver);
    LOG("fru_data_minor_ver : %d", rsp_dat->fru_data_minor_ver);
    LOG("fru_table_len : %d", rsp_dat->fru_table_len);
    LOG("fru_table_maxi_size : %d", rsp_dat->fru_table_maxi_size);
    LOG("num_of_records : %d", rsp_dat->num_of_records);
    LOG("num_of_records_set_identifiers : %d", rsp_dat->num_of_records_set_identifiers);
    u32 crc32 = 0;
    cm_memcpy(&crc32, &((u8 *)rsp_dat)[sizeof(pldm_fru_get_fru_record_table_metadata_rsp_dat_t)], sizeof(u32));
    LOG("crc32 : %#x", crc32);
    gs_pldm_fru_gen_state.event_id = PLDM_FRU_GEN_UNKNOW;
}

pldm_fru_get_fru_record_table_rsp_dat_t get_fru_record_table_rsp_dat;
u32 cal_crc32 = 0;
extern u32 crc32_pldm(u32 init_crc, u8 *data, u32 len);
extern u8 *pldm_fru_find_next_fru_data(u8 *start_addr);
static void pldm_fru_gen_recv_cmd_02(u8 *buf)
{
    pldm_fru_get_fru_record_table_rsp_dat_t *rsp_dat = (pldm_fru_get_fru_record_table_rsp_dat_t *)(buf + sizeof(pldm_response_t));
    pldm_response_t *rsp_hdr = (pldm_response_t *)(buf);

    if (rsp_hdr->cpl_code == MCTP_COMMAND_SUCCESS) {
        get_fru_record_table_rsp_dat = *rsp_dat;
        u8 *next = pldm_fru_find_next_fru_data(rsp_dat->portion_of_data);
        u16 data_len = next - rsp_dat->portion_of_data;
        cal_crc32 = crc32_pldm(cal_crc32 ^ 0xFFFFFFFFUL, rsp_dat->portion_of_data, data_len);
        if (rsp_dat->transfer_flg == PLDM_FRU_TRANSFRT_FLG_START_AND_END || rsp_dat->transfer_flg == PLDM_FRU_TRANSFRT_FLG_END) {
            get_fru_record_table_rsp_dat.next_data_transfer_hdl = 0;
            LOG("calc crc : 0x%08x", cal_crc32);
            cal_crc32 = 0;
        } else {
            gs_pldm_fru_gen_state.event_id = PLDM_FRU_GEN_ENTER_CMD_02;
        }
        LOG("%s, %d", __FUNCTION__, rsp_dat->next_data_transfer_hdl);
    } else {
        LOG("%s, cpl_code %#x", __FUNCTION__, rsp_hdr->cpl_code);
    }
}

pldm_fru_get_fru_record_table_rsp_dat_t pldm_fru_get_fru_record_table_rsp_dat(void)
{
    return get_fru_record_table_rsp_dat;
}

void pldm_fru_gen_recv(int cmd, u8 *buf)
{
    gen_cmd cmds[] = {
        gen_cmd_unsupport,
        pldm_fru_gen_recv_cmd_01,
        pldm_fru_gen_recv_cmd_02,
    };

    if (cmd < PLDM_FRU_DATA_CMD) {
        if (cmds[cmd])
            cmds[cmd](buf);
        LOG("RECV CMD : %#x\n", cmd);
    } else {
        gs_pldm_fru_gen_state.event_id = PLDM_FRU_GEN_UNKNOW;
        LOG("ERR CMD : %#x\n", cmd);
    }
}