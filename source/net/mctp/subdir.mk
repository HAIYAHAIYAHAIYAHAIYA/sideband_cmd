MCTP_DIR = $(SOURCE_DIR)/net/mctp
INCLUDES += -I $(MCTP_DIR)

MCTP_C_SRCS = $(wildcard $(MCTP_DIR)/*.c)

C_SRCS += $(MCTP_C_SRCS)
