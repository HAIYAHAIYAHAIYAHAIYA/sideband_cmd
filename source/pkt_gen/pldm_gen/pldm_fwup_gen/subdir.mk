
PLDM_FWUP_GEN_DIR = $(SOURCE_DIR)/pkt_gen/pldm_gen/pldm_fwup_gen
INCLUDES += -I $(PLDM_FWUP_GEN_DIR)

PLDM_FWUP_GEN_C_SRCS = $(wildcard $(PLDM_FWUP_GEN_DIR)/*.c)

C_SRCS += $(PLDM_FWUP_GEN_C_SRCS)
