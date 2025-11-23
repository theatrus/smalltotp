# STM32G0 TOTP Benchmark Example

This example demonstrates the smalltotp library running on an STM32G07x/G08x microcontroller.

## Hardware

- STM32G07x or STM32G08x microcontroller (Cortex-M0+)
- ST-Link debugger/programmer
- Optional: LED on PA5 (available on most STM32G0 development boards)

## Features

- RFC 6238 TOTP test vector validation
- Performance benchmarking with cycle counting
- Time window validation testing
- LED status indication
- Semihosting printf output for results

## Building

Ensure you have the ARM GCC toolchain and stlink tools installed:

```bash
# Build the project
make

# Flash to device
make flash

# View the output with st-util and GDB
make debug
```

## Running

### Option 1: Using st-util and GDB (recommended for viewing output)

In one terminal:
```bash
st-util
```

In another terminal:
```bash
cd examples/stm32g0_benchmark
arm-none-eabi-gdb build/totp_benchmark.elf
(gdb) target extended-remote :4242
(gdb) load
(gdb) monitor arm semihosting enable
(gdb) continue
```

The output will appear in the terminal running `st-util`.

### Option 2: Flash only (LED indication)

```bash
make flash
```

The LED on PA5 will:
- Turn on briefly at startup
- Toggle during each benchmark iteration
- Blink continuously at 1Hz when complete

## Expected Output

```
====================================
  smalltotp STM32G0 Benchmark
====================================

=== TOTP Benchmark on STM32G0 ===
System Clock: 16000000 Hz
Processor: Cortex-M0+

Secret decoded: 20 bytes

--- RFC 6238 Test Vectors ---
Time: 59
  Expected: 94287082
  Generated: 94287082
  Result: PASS
  Cycles: ~XXXXX (~XX ms)

[... more test vectors ...]

--- Performance Benchmark ---
Running 10 iterations...

Results:
  Average cycles: ~XXXXX
  Average time: ~XX ms
  Codes per second: ~XX

--- Memory Usage ---
  Code size: ~1.2 KB (library)
  Stack usage: ~512 bytes (estimated)
  Secret storage: 20 bytes

=== Benchmark Complete ===

[... validation tests ...]
```

## Performance Notes

On STM32G0 running at 16 MHz (default HSI), typical performance:
- TOTP generation: ~10-15ms per code
- Code size: ~1.2KB for TOTP library
- RAM usage: <1KB total

## Customization

### Change System Clock

Edit `main.c` and update `SYSTEM_CLOCK_HZ` to match your clock configuration.

### Use UART Instead of Semihosting

Semihosting adds overhead. For production use, implement UART printf:

1. Remove `--specs=rdimon.specs` from Makefile LDFLAGS
2. Implement `_write()` in syscalls.c to output via UART
3. Initialize UART in `main.c`

### Different LED Pin

Change the GPIO configuration in `gpio_init()` and update LED functions.

## Troubleshooting

**No output when debugging:**
- Make sure to run `monitor arm semihosting enable` in GDB
- Check that st-util is running

**Flash fails:**
- Verify ST-Link connection: `st-info --probe`
- Try erasing first: `make erase`

**Wrong cycles count:**
- Verify system clock frequency matches `SYSTEM_CLOCK_HZ`
