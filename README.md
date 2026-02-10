# ESP32-S3-AMOLED Platform

![PlatformIO](https://img.shields.io/badge/Platform-PlatformIO-orange)
![ESP32](https://img.shields.io/badge/Chip-ESP32--S3-32cd32)
![Display](https://img.shields.io/badge/Display-AMOLED_1.91%22-blue)
![LVGL](https://img.shields.io/badge/GUI-LVGL_9-red)

A highly optimized master reference implementation for the **Waveshare ESP32-S3-AMOLED-1.91** development board. This project standardizes the drivers, power management, and UI architecture to serve as a robust foundation for future derived systems.

---

## ✨ Features

*   **Native Low Power Mode**: Polled `esp_light_sleep` implementation for <1mA idle consumption with instant wake-on-input.
*   **High-Performance Graphics**: Dual-buffered **LVGL 9** rendering on the RM67162 AMOLED controller using OPI PSRAM.
*   **Unified Input Architecture**: Single I2C bus driver for both Trackball and IMU, with automatic coordinate rotation.
*   **Crash-Proof Config**: Tested partition tables and build scripts to prevent common Xtensa/ARM assembly conflicts.

---

## 🚀 Getting Started

### Prerequisites
*   **VSCode** with **PlatformIO** extension.
*   **Python 3** (for build scripts).

### Installation
1.  **Clone the repository**
2.  **Verify `platformio.ini`**: Ensure `board_build.arduino.memory_type = qio_opi` is set.
3.  **Build & Flash**: Connect via USB-C (ensure you hold BOOT if it's the first flash).

---

## 🛠️ Hardware Specification

| Component | Detail |
| :--- | :--- |
| **MCU** | ESP32-S3R8 (Dual Core 240MHz) |
| **PSRAM** | 8MB OPI (Octal SPI) |
| **Flash** | 16MB (External) |
| **Display** | 1.91" AMOLED (240x536) @ 60Hz |
| **IMU** | QMI8658 (6-Axis) |
| **Input** | I2C Trackball with RGB LED |

---

## 🔌 Pinout Reference

| Function | Pin (GPIO) | Notes |
| :--- | :--- | :--- |
| **QSPI SCK** | 47 | Shared with SD Card CLK |
| **QSPI CS** | 6 | Display Chip Select |
| **I2C SDA** | 40 | Trackball & IMU Shared |
| **I2C SCL** | 39 | Trackball & IMU Shared |
| **Display RST** | - | Internal/Shared |
| **Battery ADC** | 1 | Voltage Monitor |

---

## 🧠 System Architecture

### 1. Display (RM67162)
Driven via **QSPI** at 40MHz. Requires specific initialization for the AMOLED panel:
*   **Color Inversion**: `INVON (0x21)` is **mandatory** for correct black levels.
*   **Orientation**: `MADCTL (0x36)` set to `0x20 | 0x80` for landscape.
*   **Buffering**: Uses two `MALLOC_CAP_SPIRAM` buffers in PSRAM for smooth 60ms refreshes.

### 2. Power Management
Uses a **Polled Light Sleep** loop:
1.  Enter `esp_light_sleep_start()` for 100ms.
2.  Wake & Poll Trackball I2C.
3.  If no activity -> Sleep.
4.  If activity -> Wake Display -> `lv_obj_invalidate()` -> Fade In.

### 3. Memory Layout
*   **App Partition**: 3MB (via `partitions.csv`)
*   **Filesystem**: ~9.9MB FATFS/LittleFS
*   **PSRAM**: 8MB OPI (Critical: `qio_opi` mode)

---

## 🐛 Troubleshooting & Tips

*   **Display is Negative?** -> You missed the `INVON (0x21)` command in init.
*   **Build Error (Neon/Helium)?** -> Run the `fix_lvgl_9.py` script to remove ARM assembly.
*   **No Serial Output?** -> Set `ARDUINO_USB_CDC_ON_BOOT=1` in `platformio.ini`.
*   **Input Lag?** -> Check `NAVIGATION_THRESHOLD` in `input.cpp`.

---

## 📚 Appendices

### A. Partition Table (`partitions.csv`)
```csv
# Name,   Type, SubType, Offset,  Size, Flags
nvs,      data, nvs,     0x9000,  0x5000,
otadata,  data, ota,     0xe000,  0x2000,
app0,     app,  ota_0,   0x10000, 0x300000,
app1,     app,  ota_1,   0x310000,0x300000,
spiffs,   data, spiffs,  0x610000, 0x9E0000,
```

### B. Display Init Sequence (`qspi_display.cpp`)
```cpp
// 1. SlpOut & Wait
writeCommand(0x11); delay(120);
// 2. Color Mode 16-bit
writeC8D8(0x3A, 0x55); 
// 3. Orientation
writeC8D8(0x36, 0x20 | 0x80 | 0x00); 
// 4. Brightness
writeC8D8(0x51, 0x00); 
// 5. Display ON
writeCommand(0x29); 
// 6. Color Inversion (CRITICAL)
writeCommand(0x21); delay(20);
```

### C. Build Patch (`fix_lvgl_9.py`)
```python
# Removes ARM assembly from LVGL which breaks ESP32 builds
import os
import shutil
Import("env")

try:
    # Get the library dependencies directory
    libdeps_dir = env.subst("$PROJECT_LIBDEPS_DIR")
    env_name = env.subst("$PIOENV")
    lvgl_dir = os.path.join(libdeps_dir, env_name, "lvgl")

    if os.path.exists(lvgl_dir):
        problematic_dirs = [
            os.path.join(lvgl_dir, "src", "draw", "sw", "blend", "helium"),
            os.path.join(lvgl_dir, "src", "draw", "sw", "blend", "neon"),
            os.path.join(lvgl_dir, "src", "draw", "convert", "helium"),
            os.path.join(lvgl_dir, "src", "draw", "convert", "neon"),
        ]
        
        for d in problematic_dirs:
            if os.path.exists(d):
                print(f"Removing problematic LVGL 9 directory: {d}")
                shutil.rmtree(d)
except Exception as e:
    print(f"Error in fix_lvgl_9.py: {e}")
```
