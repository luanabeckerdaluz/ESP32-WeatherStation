# Automatically generated build file. Do not edit.
COMPONENT_INCLUDES += $(IDF_PATH)/components/bootloader_support/test/include
COMPONENT_LDFLAGS += -L$(BUILD_DIR_BASE)/bootloader_support_test -Wl,--whole-archive -lbootloader_support_test -Wl,--no-whole-archive
COMPONENT_LINKER_DEPS += 
COMPONENT_SUBMODULES += 
COMPONENT_LIBRARIES += bootloader_support_test
component-bootloader_support_test-build: 
