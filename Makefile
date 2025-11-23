# Configurable variables
ARCH ?= native
CC_NATIVE = gcc
CC_ARM = arm-none-eabi-gcc

# Determine compiler based on architecture
ifeq ($(ARCH),native)
    CC = $(CC_NATIVE)
    CFLAGS_ARCH =
else
    CC = $(CC_ARM)
    CFLAGS_ARCH = -mcpu=$(ARCH) -mthumb
endif

# Common flags
CFLAGS_COMMON = -Os -Wall -Wextra -std=c99 \
                -ffunction-sections -fdata-sections \
                -Iinclude

LDFLAGS_COMMON = -Wl,--gc-sections

# Source files
SRCS = src/sha1.c \
       src/hmac_sha1.c \
       src/base32.c \
       src/totp_time.c \
       src/totp.c

OBJS = $(SRCS:.c=.o)

# Test files
TEST_SRCS = tests/test_totp.c
TEST_OBJS = $(TEST_SRCS:.c=.o)
TEST_TARGET = tests/test_totp

# Library target
LIB_TARGET = libtotp.a

# Phony targets
.PHONY: all clean test embedded

# Default target
all: $(LIB_TARGET)

# Embedded target
embedded:
	@echo "Building for $(ARCH)..."
	$(MAKE) ARCH=$(ARCH) $(LIB_TARGET)

# Library
$(LIB_TARGET): $(OBJS)
	$(AR) rcs $@ $^

# Object files
%.o: %.c
	$(CC) $(CFLAGS_COMMON) $(CFLAGS_ARCH) -c $< -o $@

# Test target (always builds for native)
test: CC = $(CC_NATIVE)
test: CFLAGS_ARCH =
test: $(LIB_TARGET) $(TEST_OBJS)
	$(CC) $(CFLAGS_COMMON) $(LDFLAGS_COMMON) -o $(TEST_TARGET) $(TEST_OBJS) $(LIB_TARGET)
	./$(TEST_TARGET)

# Clean
clean:
	rm -f $(OBJS) $(TEST_OBJS) $(LIB_TARGET) $(TEST_TARGET)

# Help
help:
	@echo "TOTP Library Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  all        - Build library for native (default)"
	@echo "  embedded   - Build library for embedded target"
	@echo "  test       - Build and run tests (native only)"
	@echo "  clean      - Remove all build artifacts"
	@echo ""
	@echo "Variables:"
	@echo "  ARCH       - Target architecture (default: native)"
	@echo "               Options: native, cortex-m0plus, cortex-m4"
	@echo ""
	@echo "Examples:"
	@echo "  make                          # Build for native"
	@echo "  make ARCH=cortex-m0plus       # Build for Cortex-M0+"
	@echo "  make ARCH=cortex-m4           # Build for Cortex-M4"
	@echo "  make test                     # Run tests"
