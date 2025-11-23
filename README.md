# smalltotp - TOTP Library for Embedded Systems

A minimal, embedded-friendly C library for generating and validating Time-based One-Time Password (TOTP) codes, compliant with RFC 6238.

## Features

- **Minimal dependencies**: Designed for embedded systems with small code footprint
- **Cross-platform**: Supports native compilation and ARM Cortex-M targets (M0+, M4)
- **Size-optimized**: Built with `-Os` and dead code elimination
- **Flexible time window**: Separate before/after validation windows for fine-grained control
- **Time abstraction**: Easy integration with custom RTC implementations
- **RFC 6238 compliant**: Includes test vectors for validation
- **Base32 decoding**: Built-in support for standard TOTP secret format

## Project Structure

```
.
├── include/          # Public header files
│   ├── totp.h       # Main TOTP API
│   ├── sha1.h       # SHA1 implementation
│   ├── hmac_sha1.h  # HMAC-SHA1 wrapper
│   ├── base32.h     # Base32 decoder
│   └── totp_time.h  # Time abstraction layer
├── src/             # Implementation files
├── tests/           # Unit tests
└── Makefile         # Build system
```

## Building

### Native Build (for testing)

```bash
make
```

### Embedded Targets

```bash
# For ARM Cortex-M0+
make ARCH=cortex-m0plus

# For ARM Cortex-M4
make ARCH=cortex-m4
```

### Running Tests

```bash
make test
```

### Clean Build

```bash
make clean
```

## Usage

### Basic TOTP Generation

```c
#include "totp.h"
#include "base32.h"

// Decode your Base32-encoded secret
const char *secret_b32 = "JBSWY3DPEHPK3PXP";
uint8_t secret[64];
int secret_len = base32_decode(secret_b32, secret, sizeof(secret));

// Configure TOTP
totp_config_t config = {
    .secret = secret,
    .secret_len = secret_len,
    .time_step = 30,        // 30-second time step
    .digits = 6             // 6-digit codes
};

// Generate code for current time
uint32_t code = totp_generate_current(&config);
printf("TOTP code: %06u\n", code);
```

### TOTP Validation with Time Window

```c
// Validate with no time window (exact match only)
int valid = totp_validate_current(&config, user_code, 0, 0);

// Validate accepting codes from the past (up to 30 seconds old)
int valid = totp_validate_current(&config, user_code, 1, 0);

// Validate accepting codes from the future (up to 30 seconds ahead)
int valid = totp_validate_current(&config, user_code, 0, 1);

// Validate with symmetric window (±30 seconds)
int valid = totp_validate_current(&config, user_code, 1, 1);

// Validate with larger symmetric window (±60 seconds)
int valid = totp_validate_current(&config, user_code, 2, 2);

// Validate with asymmetric window (past: 60s, future: 30s)
int valid = totp_validate_current(&config, user_code, 2, 1);
```

The time window parameters allow fine-grained control over accepting codes from previous or future time periods:
- `window_before`: Number of past time steps to accept (1 step = 30 seconds)
- `window_after`: Number of future time steps to accept (1 step = 30 seconds)

Examples:
- `(0, 0)`: Only accept codes for the current 30-second period
- `(1, 0)`: Accept codes from current period and 30 seconds in the past
- `(0, 1)`: Accept codes from current period and 30 seconds in the future
- `(1, 1)`: Accept codes from ±30 seconds (3 total periods)
- `(2, 2)`: Accept codes from ±60 seconds (5 total periods)
- `(2, 1)`: Accept codes from -60s to +30s (4 total periods)

### Custom Time Source (RTC Integration)

```c
#include "totp_time.h"

// Implement your RTC function
uint64_t my_rtc_get_time(void) {
    // Read from your RTC hardware
    return rtc_read_unix_timestamp();
}

// Set the custom time function
totp_set_time_func(my_rtc_get_time);

// Now TOTP will use your RTC
uint32_t code = totp_generate_current(&config);
```

### STM32 HAL RTC Integration

For STM32 microcontrollers using the HAL library, a conversion utility is provided:

```c
#include "totp.h"
#include "totp_time.h"
#include "stm32_rtc_time.h"

static RTC_HandleTypeDef hrtc;

// Wrapper function to get Unix time from STM32 RTC
uint64_t stm32_rtc_get_unix_time(void) {
    RTC_TimeTypeDef sTime;
    RTC_DateTypeDef sDate;

    // IMPORTANT: Must call GetTime before GetDate to unlock shadow registers
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    return stm32_rtc_to_unix_time(&sTime, &sDate);
}

// Configure TOTP to use STM32 RTC
totp_set_time_func(stm32_rtc_get_unix_time);
```

**Important notes:**
- The RTC `Year` field is 0-99 representing years since 2000
- Always call `HAL_RTC_GetDate()` after `HAL_RTC_GetTime()` to unlock the RTC shadow registers
- Use `RTC_FORMAT_BIN` for binary format (or handle BCD conversion if needed)
- See `stm32_example.c` for a complete working example

## Compilation Flags

The library is compiled with the following optimization flags:

- `-Os`: Optimize for size
- `-ffunction-sections`: Place each function in its own section
- `-fdata-sections`: Place each data item in its own section
- `-Wl,--gc-sections`: Remove unused sections at link time

This ensures the smallest possible code footprint for embedded systems.

## Memory Requirements

Approximate memory usage:

- **Stack**: ~512 bytes (SHA1 context + buffers)
- **Code size**: ~3-5 KB (depending on architecture and optimization)
- **Secret storage**: User-provided (typically 10-20 bytes)

## API Reference

### TOTP Generation

```c
uint32_t totp_generate(const totp_config_t *config, uint64_t timestamp);
uint32_t totp_generate_current(const totp_config_t *config);
```

### TOTP Validation

```c
int totp_validate(const totp_config_t *config, uint32_t code,
                  uint64_t timestamp, uint32_t window_before, uint32_t window_after);
int totp_validate_current(const totp_config_t *config, uint32_t code,
                          uint32_t window_before, uint32_t window_after);
```

### Time Functions

```c
void totp_set_time_func(totp_time_func_t func);
uint64_t totp_get_time(void);
```

### Base32 Decoding

```c
int base32_decode(const char *encoded, uint8_t *result, size_t result_len);
```

## Testing

The test suite includes:

- Base32 decoding validation
- RFC 6238 test vectors (8-digit codes)
- 6-digit code generation
- Time window validation
- Custom time function testing

All tests pass on native builds and validate against official RFC 6238 test vectors.

## License

Copyright 2025 Yann Ramin

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
