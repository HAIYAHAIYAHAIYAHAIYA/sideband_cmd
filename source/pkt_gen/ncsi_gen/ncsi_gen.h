#ifndef __NCSI_GEN_H__
#define __NCSI_GEN_H__

#include "main.h"

#define NCSI_CMD        (0x1A + 1)

void ncsi_cmd_gen(int cmd, u8 *buf);

#endif /* __NCSI_GEN_H__ */