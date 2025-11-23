# SPDX-License-Identifier: Apache-2.0
# STM32G0 TOTP Benchmark Makefile

# Target
TARGET = totp_benchmark

# Toolchain
PREFIX = arm-none-eabi-
CC = $(PREFIX)gcc
OBJCOPY = $(PREFIX)objcopy
OBJDUMP = $(PREFIX)objdump
SIZE = $(PREFIX)size
GDB = $(PREFIX)gdb

# STM32 specific
MCU = cortex-m0plus
ARCH = -mcpu=$(MCU) -mthumb

# Directories
TOTP_ROOT = ../..
SRC_DIR = $(TOTP_ROOT)/src
INC_DIR = $(TOTP_ROOT)/include
BUILD_DIR = build

# Source files
SRCS = main.c \
       usart.c \
       startup_stm32g0.c \
       syscalls.c \
       $(SRC_DIR)/sha1.c \
       $(SRC_DIR)/hmac_sha1.c \
       $(SRC_DIR)/base32.c \
       $(SRC_DIR)/totp.c \
       $(SRC_DIR)/totp_time.c

OBJS = $(addprefix $(BUILD_DIR)/, $(notdir $(SRCS:.c=.o)))

# Includes
INC = -I$(INC_DIR) -I.

# Compiler flags
CFLAGS = $(ARCH) \
         -Os \
         -Wall \
         -Wextra \
         -std=c99 \
         -ffunction-sections \
         -fdata-sections \
         -fno-common \
         -g3 \
         $(INC)

# Linker flags
LDFLAGS = $(ARCH) \
          -T stm32g0.ld \
          -Wl,--gc-sections \
          -Wl,-Map=$(BUILD_DIR)/$(TARGET).map \
          --specs=nosys.specs

# Default target
all: $(BUILD_DIR) $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).bin $(BUILD_DIR)/$(TARGET).hex
	@echo "Build complete!"
	@$(SIZE) $(BUILD_DIR)/$(TARGET).elf

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build ELF
$(BUILD_DIR)/$(TARGET).elf: $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

# Build objects
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Generate binary
$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@

# Generate hex
$(BUILD_DIR)/$(TARGET).hex: $(BUILD_DIR)/$(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

# Flash to device
flash: $(BUILD_DIR)/$(TARGET).bin
	st-flash write $(BUILD_DIR)/$(TARGET).bin 0x8000000

# Erase flash
erase:
	st-flash erase

# Debug with st-util and gdb
debug: $(BUILD_DIR)/$(TARGET).elf
	st-util &
	$(GDB) -ex "target extended-remote :4242" -ex "load" $(BUILD_DIR)/$(TARGET).elf

# Clean
clean:
	rm -rf $(BUILD_DIR)

# Disassembly
disasm: $(BUILD_DIR)/$(TARGET).elf
	$(OBJDUMP) -d $(BUILD_DIR)/$(TARGET).elf > $(BUILD_DIR)/$(TARGET).dis

.PHONY: all clean flash erase debug disasm
