#!/usr/bin/make -f
TARGET := lpc2103-hello.elf
TARGET_HEX := $(TARGET:.elf=.ihx)
TARGET_BIN := $(TARGET:.elf=.bin)
TARGETS := $(TARGET) $(TARGET_HEX) $(TARGET_BIN)

BUILD_DIR := .build

ARCH := arm-none-eabi-
CC := $(ARCH)gcc
CXX := $(ARCH)g++
AS := $(ARCH)as
CCAS := $(CC) -x assembler-with-cpp
CCLD := $(CC)
CXXLD := $(CXX)
OBJCOPY := $(ARCH)objcopy
SIZE := $(ARCH)size

# Device descriptions are not shipped by every vendor: the MSPM0 SDK has no SVD
# at all, and its <ti/devices/...> headers are a 68-file tree of per-peripheral
# register definitions, so the SVD for it has to be sourced separately.
SVDCONV_PATH := C:\Keil_v5\UV4
SVDCONV := $(SVDCONV_PATH)/SVDConv.exe

SVD_FILES := $(wildcard contrib/platform/share/*.svd)
DEV_HEADERS := $(patsubst %.svd,%.h,$(subst /share/,/include/,$(SVD_FILES)))
DEV_HANDLERS := $(DEV_HEADERS:%.h=%_handlers.h)
GENHDRS := $(DEV_HEADERS) $(DEV_HANDLERS)

SRC_PATHS := src contrib/libusb_stm32/src contrib/platform/src
HEADER_PATHS := include contrib/platform/include contrib/cmsis/CMSIS/Core/Include
LIB_PATHS := lib contrib/platform/lib

C_FILES := $(wildcard $(addsuffix /*.c, $(SRC_PATHS)))
CXX_FILES := $(wildcard $(addsuffix /*.cc, $(SRC_PATHS)) $(addsuffix /*.cpp, $(SRC_PATHS)))
S_FILES := $(wildcard $(addsuffix /*.S, $(SRC_PATHS)) $(addsuffix /*.s, $(SRC_PATHS)))
LD_FILES := $(wildcard $(addsuffix /*.ld, $(LIB_PATHS)))
SRC_FILES := $(C_FILES) $(CXX_FILES) $(S_FILES)
OBJS := $(addprefix $(BUILD_DIR)/, $(addsuffix .o, $(SRC_FILES)))
DEPS := $(OBJS:.o=.d)

CPUFLAGS := -mcpu=cortex-m0plus -mthumb
OPTFLAGS := -Og -g3
WARNFLAGS := -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function
GENFLAGS := -ffunction-sections -fdata-sections
DEPFLAGS := -MMD -MP

CPPFLAGS := $(addprefix -I, $(HEADER_PATHS))
CFLAGS := $(CPUFLAGS) -std=gnu17 $(DEPFLAGS) $(GENFLAGS) $(OPTFLAGS) $(WARNFLAGS)
CXXFLAGS := $(CPUFLAGS) -std=gnu++17 $(DEPFLAGS) $(GENFLAGS) $(OPTFLAGS) $(WARNFLAGS)
ASFLAGS := $(CPUFLAGS) $(DEPFLAGS) $(GENFLAGS) $(OPTFLAGS) $(WARNFLAGS)
LDFLAGS := $(CPUFLAGS) $(addprefix -L, $(LIB_PATHS)) -Wl,-T,lpc2103-hello.ld -Wl,--gc-sections # Platform code include a linker script fragment.

all: headers $(TARGET)
hex: $(TARGET_HEX)
bin: $(TARGET_BIN)
headers: $(GENHDRS)

# Berkeley totals (text/data/bss), then the per-section breakdown. Sections
# with address 0 (.comment, .ARM.attributes, .debug_*) are not loaded and do
# not consume flash or SRAM.
size: $(TARGET)
	$(SIZE) $<
	@echo
	$(SIZE) -A $<

contrib/platform/include/%.h: contrib/platform/share/%.svd
	-$(SVDCONV) $< -o $(dir $@) --generate=header --fields=macro --fields=enum

contrib/platform/include/%_handlers.h: contrib/platform/include/%.h
	python3 contrib/irqn/irqn.py -i $< -o $@

$(BUILD_DIR)/%.c.o: %.c $(GENHDRS)
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.cc.o: %.cc $(GENHDRS)
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%.cpp.o: %.cpp $(GENHDRS)
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%.S.o: %.S $(GENHDRS)
	@mkdir -p $(dir $@)
	$(CCAS) $(CPPFLAGS) $(ASFLAGS) -c $< -o $@

$(BUILD_DIR)/%.s.o: %.s $(GENHDRS)
	@mkdir -p $(dir $@)
	$(AS) $(CPPFLAGS) $(ASFLAGS) -c $< -o $@

$(TARGET): $(OBJS) $(LD_FILES)
	$(if $(CXX_FILES),$(CXXLD),$(CCLD)) $(LDFLAGS) $(OBJS) $(LIBS) -o $(TARGET)

%.ihx: %.elf
	$(OBJCOPY) -O ihex $< $@

%.bin: %.elf
	$(OBJCOPY) -O binary $< $@

clean:
	-rm -rf $(TARGETS) $(BUILD_DIR)

distclean: clean
	-rm -rf $(GENHDRS)

-include $(DEPS)
