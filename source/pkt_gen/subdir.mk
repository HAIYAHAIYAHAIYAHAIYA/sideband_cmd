include $(SOURCE_DIR)/pkt_gen/pldm_gen/subdir.mk
include $(SOURCE_DIR)/pkt_gen/mctp_gen/subdir.mk

PKT_GEN_DIR = $(SOURCE_DIR)/pkt_gen
INCLUDES += -I $(PKT_GEN_DIR)

PKT_GEN_DIR_C_SRCS = $(wildcard $(PKT_GEN_DIR)/*.c)

C_SRCS += $(PKT_GEN_DIR_C_SRCS)
