#include "main.h"
#include "mctp_ctl.h"
#include "pldm_redfish.h"
#include "pldm_monitor.h"
#include "pldm_fw_update.h"
#include "pldm_control.h"
#include "protocol_device.h"
#include "pldm.h"
#include "mctp.h"
#include "pkt_gen.h"

sys_ctrl_t g_sys;
FILE *g_fp = NULL;

int cmd_get(void)
{
    int choice, confirm;
    LOG("Choice you protocol: ");
    LOG("\t1:mctp cmd!");
    LOG("\t2:pldm cmd!");
    LOG("\tff:quit!");
    LOG("input you wanted operation : ");
	scanf("%x", &choice);
    while ((confirm = getchar()) != '\n')//用户再次输入空格键才表示其真正确认
		continue;
    while (choice != 0xFF && (choice < 1 || choice > 2))//判断执行1-8，并且检测合法输入
	{
		LOG("you input: '%d' err! need re input : ", choice);
		scanf("%x", &choice);
        scanf("%*[^\n]");
        scanf("%*c");
	}
    return choice;
}

int pldm_type_get(void)
{
    int choice, confirm;
    LOG("Choice you PLDM protocol: ");
    LOG("\t0:MCTP_PLDM_CONTROL!");
    LOG("\t2:MCTP_PLDM_MONITOR!");
    LOG("\t5:MCTP_PLDM_UPDATE!");
    LOG("\t6:MCTP_PLDM_REDFISH!");
    LOG("\tff:quit!");
    LOG("pldm type : ");
	scanf("%x", &choice);
    while ((confirm = getchar()) != '\n')//用户再次输入空格键才表示其真正确认
		continue;
    while (choice != 0xFF && choice > 6)//判断执行1-8，并且检测合法输入
	{
		LOG("you input: '%d' err! need re input : ", choice);
		scanf("%x", &choice);
        scanf("%*[^\n]");           /* 清空缓冲区 */
        scanf("%*c");
	}
    return choice;
}

int cmd_num_get(void)
{
    int choice, confirm;
    LOG("input you cmd num : ");
	scanf("%x", &choice);
    while ((confirm = getchar()) != '\n')//用户再次输入回车键才表示其真正确认
		continue;
    while (choice != 0xFF && (choice < 0 || choice > 0x53))//判断执行1-8，并且检测合法输入
	{
		LOG("you input: '%d' err! need re input : ", choice);
		scanf("%x", &choice);
        scanf("%*[^\n]");
        scanf("%*c");
	}
    return choice;
}

void cmd_init(void)
{
    mctp_ctrl_init();
    pldm_monitor_init();
    pldm_redfish_init();
    pldm_fwup_init();

    pldm_gen_init();
}

void mctp_process(protocol_msg_t *pkt)
{
    int choice;
    while (1) {
        LOG("MCTP CMD");
        choice = cmd_num_get();
        if (choice == 0xFF) break;
        mctp_gen(choice, pkt->req_buf);
        mctp_ctl_process(pkt);
    }
}

// void pldm_process(protocol_msg_t *pkt)
// {
//     int choice;
//     int pldm_type;
//     while (1) {
//         pldm_type = pldm_type_get();
//         if (pldm_type == 0xFF) break;
//         while (1) {
//             LOG("PLDM CMD");
//             // choice = cmd_num_get();
//             // if (choice == 0xFF) break;
//             u8 i = 0;
//             for (i = 0; i < 0x1d; i++) {
//                 pldm_gen(pldm_type, i, pkt->req_buf);
//                 pldm_pkt_process(pkt);
//             }
//             if (i == 0x1d) break;
//         }
//     }
// }

void pldm_process(protocol_msg_t *pkt)
{
    int choice;
    int pldm_type;
    while (1) {
        pldm_type = pldm_type_get();
        if (pldm_type == 0xFF) break;
        while (1) {
            LOG("PLDM CMD");
            choice = cmd_num_get();
            if (choice == 0xFF) break;
            if (choice)
                pldm_gen_manual(pldm_type, choice, pkt->req_buf);
            while (1) {
                u8 ret = pldm_gen(pldm_type, choice, pkt->req_buf);
                if (ret == 0xFF) break;
                if (ret == 1) pldm_pkt_process(pkt);
            }
        }
    }
}

void my_exit(void)
{
    fclose(g_fp);
    printf("before exit () !\n");
}

u8 req_buf[1024 + 4 + 5];
u8 rsp_buf[1024 + 4 + 5];

int main(int argc, char * argv [])
{
    cmd_init();
    atexit (my_exit);
    LOG("hello world!");
    int choice;
    protocol_msg_t pkt;
    pkt.req_buf = req_buf;
    pkt.rsp_buf = rsp_buf;
    pkt.mctp_hw_id = 1;
	while ((choice = cmd_get()) && choice != 0xFF)
	{
        switch (choice) {
            case 1:
            mctp_process(&pkt);
            break;
            case 2:
            pldm_process(&pkt);
            default:
            LOG("choice : %d", choice);
            break;
        }
        // LOG("%d", choice);
	}
    // fclose(g_fp);
	LOG("thanks for use!");
    return 0;
}