PLDM_DIR = $(SOURCE_DIR)/net/pldm
INCLUDES += -I $(PLDM_DIR)

PLDM_C_SRCS = $(wildcard $(PLDM_DIR)/*.c)

C_SRCS += $(PLDM_C_SRCS)
