
NCSI_GEN_DIR = $(SOURCE_DIR)/pkt_gen/ncsi_gen
INCLUDES += -I $(NCSI_GEN_DIR)

NCSI_GEN_C_SRCS = $(wildcard $(NCSI_GEN_DIR)/*.c)

C_SRCS += $(NCSI_GEN_C_SRCS)
