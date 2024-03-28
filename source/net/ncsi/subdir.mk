NCSI_DIR = $(SOURCE_DIR)/net/ncsi
INCLUDES += -I $(NCSI_DIR)

NCSI_C_SRCS = $(wildcard $(NCSI_DIR)/*.c)

C_SRCS += $(NCSI_C_SRCS)
