
PLDM_FRU_GEN_DIR = $(SOURCE_DIR)/pkt_gen/pldm_gen/pldm_fru_data_gen
INCLUDES += -I $(PLDM_FRU_GEN_DIR)

PLDM_FRU_GEN_C_SRCS = $(wildcard $(PLDM_FRU_GEN_DIR)/*.c)

C_SRCS += $(PLDM_FRU_GEN_C_SRCS)
