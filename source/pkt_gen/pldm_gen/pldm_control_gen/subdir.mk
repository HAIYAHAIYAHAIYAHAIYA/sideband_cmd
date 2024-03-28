
PLDM_CTRL_GEN_DIR = $(SOURCE_DIR)/pkt_gen/pldm_gen/pldm_control_gen
INCLUDES += -I $(PLDM_CTRL_GEN_DIR)

PLDM_CTRL_GEN_C_SRCS = $(wildcard $(PLDM_CTRL_GEN_DIR)/*.c)

C_SRCS += $(PLDM_CTRL_GEN_C_SRCS)
