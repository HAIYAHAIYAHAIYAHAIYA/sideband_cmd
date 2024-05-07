OUTPUT_DIR = build
SOURCE_DIR = source
INCLUDE_DIR = include
# CFLAGS = -c -g -lpthread
CFLAGS = -c -g -O2
MKDIR = mkdir -p
NUL = dev/null

include $(INCLUDE_DIR)/subdir.mk
include $(SOURCE_DIR)/subdir.mk

export PLDM_FWUP_IMG_GEN = ${SOURCE_DIR}/net/pldm/pldm_scripts/pldm_fwup_scripts/pldm_fwup_pkg_creator.py
export PLDM_FWUP_IMG_INFO_GEN = ${SOURCE_DIR}/net/pldm/pldm_scripts/pldm_fwup_scripts/pldm_fwup_img_info_creator.py
export PLDM_REDFISH_DICT_GEN = ${SOURCE_DIR}/net/pldm/pldm_scripts/pldm_redfish_scripts/pldm_redfish_dict_gen.py

OBJS = $(C_SRCS:%.c=$(OUTPUT_DIR)/%.o)
OBJS_D := $(patsubst %.c,$(OUTPUT_DIR)/%.d,$(C_SRCS))		# add .d 自动生成依赖

TARGET = $(OUTPUT_DIR)/main.exe

CC = gcc
ALL = cleanall excute clean

#-lwsock32 	$(shell @$(MKDIR) $(dir $@) 2> dev/null || @echo off) -lmingw32 -lgdi32	@echo $(dir $@) @$(MKDIR) $(dir $@) 2> $(NUL)
$(TARGET) : $(OBJS)
	@echo -e '\033[36mGen file: all file -> $@ \033[0m'
	$(CC) $(OBJS) $' -o $@
	@python3 ${PLDM_REDFISH_DICT_GEN} 2>/dev/null || python ${PLDM_REDFISH_DICT_GEN}
	@python3 ${PLDM_FWUP_IMG_GEN} 2>/dev/null || python ${PLDM_FWUP_IMG_GEN}
	@python3 ${PLDM_FWUP_IMG_INFO_GEN} 2>/dev/null || python ${PLDM_FWUP_IMG_INFO_GEN}

-include $(OBJS_D)
$(OBJS):$(OUTPUT_DIR)/%.o:%.c
	$(shell $(MKDIR) $(dir $@) 2> /dev/null)
	@echo -e '\033[32mCompile C file: $< -> $@ \033[0m'
	$(CC) $(CFLAGS) $(INCLUDES) $< -MMD -o $@

.PHONY : ALL

cleanall:clean
	rm -rf $(TARGET)

clean:
	rm -rf $(OUTPUT_DIR)/

excute:
	$(OUTPUT_DIR)/main
