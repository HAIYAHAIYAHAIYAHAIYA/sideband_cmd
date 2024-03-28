APP_DIR = $(SOURCE_DIR)/app
INCLUDES += -I $(APP_DIR)

APP_C_SRCS = $(wildcard $(APP_DIR)/*.c)

C_SRCS += $(APP_C_SRCS)
