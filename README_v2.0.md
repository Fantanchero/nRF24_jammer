# 🎯 BT Deauth v2.0 - Bluetooth Speaker Disconnection PoC

<div align="center">
  <h2>Bluetooth Speaker Ghost Disconnection Attack</h2>
  <p><strong>Proof of Concept</strong> - Exploiting LMP Layer Vulnerabilities</p>
</div>

---

## 📌 Overview

**BT Deauth v2.0** is a specialized proof-of-concept that demonstrates vulnerabilities in Bluetooth Classic audio devices. Instead of broad-spectrum jamming, it performs **selective LMP (Link Manager Protocol) detachment attacks** on Bluetooth speakers.

### Core Concept: "Ghost Disconnection"
- **Identifies** active Bluetooth audio devices via BLE scanning
- **Spoofs** the MAC address of the connected source device
- **Injects** legitimate LMP detach packets into the audio stream
- **Result**: Target speaker disconnects cleanly, believing the user terminated the connection

---

## 🛠️ Hardware Requirements

| Component | Quantity | Notes |
|-----------|----------|-------|
| **LOLIN32 Lite** | 1 | ESP32 with optimized power profile |
| **nRF24L01+PA+LNA** | 2 | PA+LNA variant for extended range |
| **SSD1306 OLED 128x64** | 1 | Display for UI |
| **Tactile Buttons** | 2 | Basic navigation |
| **100µF 16V Capacitors** | 2 | Power filtering for nRF24 modules |

---

## 📐 PINOUT - LOLIN32 LITE v2.0

```
╔════════════════════════════════════════════╗
║         LOLIN32 LITE PINOUT v2.0           ║
╠════════════════════════════════════════════╣
║                                            ║
║  OLED DISPLAY (SSD1306 - I2C)              ║
║  ├─ SDA ────────────────── GPIO 4          ║
║  ├─ SCL ────────────────── GPIO 5          ║
║  ├─ VCC ────────────────── 3.3V            ║
║  └─ GND ────────────────── GND             ║
║                                            ║
║  nRF24L01+PA+LNA #1 (HSPI Primary)         ║
║  ├─ MOSI ───────────────── GPIO 13         ║
║  ├─ MISO ───────────────── GPIO 12         ║
║  ├─ SCK ────────────────── GPIO 14         ║
║  ├─ CE ─────────────────── GPIO 25         ║
║  ├─ CSN ────────────────── GPIO 26         ║
║  ├─ VCC ────────────────── 3.3V + 100µF    ║
║  └─ GND ────────────────── GND             ║
║                                            ║
║  nRF24L01+PA+LNA #2 (HSPI Secondary)       ║
║  ├─ MOSI ───────────────── GPIO 13 (SPI)  ║
║  ├─ MISO ───────────────── GPIO 12 (SPI)  ║
║  ├─ SCK ────────────────── GPIO 14 (SPI)  ║
║  ├─ CE ─────────────────── GPIO 27         ║
║  ├─ CSN ────────────────── GPIO 16         ║
║  ├─ VCC ────────────────── 3.3V + 100µF    ║
║  └─ GND ────────────────── GND             ║
║                                            ║
║  BUTTONS (Pull-up Internal, Active LOW)    ║
║  ├─ Button 1 (NEXT/OK) ─── GPIO 32         ║
║  ├─ Button 2 (BACK) ────── GPIO 33         ║
║  └─ GND (both) ─────────── GND             ║
║                                            ║
║  OPTIONAL STATUS LED                       ║
║  └─ LED ────────────────── GPIO 2          ║
║                                            ║
╚════════════════════════════════════════════╝
```

### ⚠️ Important Notes:
- **SPI Sharing**: Pins 13, 12, 14 are shared between both nRF24 modules
- **Chip Select**: Each nRF24 has **unique CE and CSN pins** for independent control
- **Capacitors**: **MANDATORY** - Place 100µF capacitor across VCC-GND for each nRF24
- **I2C**: OLED uses standard I2C protocol on GPIO 4 (SDA) and GPIO 5 (SCL)

---

## 🚀 Build & Flash

### Prerequisites
1. Install **PlatformIO IDE** extension in VS Code
2. Install **ESP32 board package** via PlatformIO
3. Install dependencies (handled automatically)

### Build from Source
```bash
# Open project in PlatformIO
pio project init

# Build for v2.0
pio run -e v2_0_bluetooth_deauth

# Binary output: .pio/build/v2_0_bluetooth_deauth/firmware.bin
```

### Flash to Device
```bash
# Via USB (using esptool)
pio run -e v2_0_bluetooth_deauth -t upload

# Or use ESP Web Tool: https://esp.huhn.me/
```

---

## 🎮 User Interface & Controls

### 2-Button Control Scheme

#### **Menu Navigation**
- **Button 1 (NEXT)**: Move to next option
- **Button 1 Long Press** (>1s): Select option / Start attack
- **Button 2 (BACK)**: Go back / Stop attack

#### **Scanning State**
- Device list automatically updates as speakers are found

#### **Attack State**
- **Button 1 Long Press**: Pause/Resume attack
- **Button 2 Short Press**: Stop attack and return to device selection

### Display States

```
┌─────────────────────────────┐
│  == SCANNING ==             │
│  Devices: 3                 │  → Auto-refreshing
│  Searching for Bluetooth... │
└─────────────────────────────┘

┌─────────────────────────────┐
│  == DEVICES FOUND ==        │
│ ▶ JBL Flip 5                │  → Scrollable list
│   00:11:22:33:44:55         │
│ [1/3]                       │
└─────────────────────────────┘

┌─────────────────────────────┐
│  == TARGET SELECTED ==      │
│  Name: JBL Flip 5           │  → Ready to attack
│  MAC: 00:11:22:33:44:55     │
│  Press OK to start          │
└─────────────────────────────┘

┌─────────────────────────────┐
│  == ATTACKING ==            │
│  Target: JBL Flip 5         │  → Live feedback
│  Packets: 1250              │
│  [===========] 50%          │
│  Press BACK to stop         │
└─────────────────────────────┘
```

---

## 🔬 Technical Details

### Attack Mechanism

#### **Phase 1: Device Detection**
1. LOLIN32 initiates BLE scan using **NimBLE stack**
2. Scans for **BLE Appearance codes** typical of audio devices
3. Verifies device name contains audio keywords (e.g., "speaker", "JBL", "Bose")
4. Stores discovered devices with MAC, RSSI, and name

#### **Phase 2: Target Selection**
1. User navigates through discovered devices
2. Selects target audio device
3. System stores target **MAC address** and verifies connectivity

#### **Phase 3: LMP Injection**
1. **nRF24L01+PA+LNA** operates in **raw RF injection mode** at 2.4 GHz
2. Cycles through Bluetooth **hopping channels** (0-78)
3. Injects crafted **LMP_DETACH** packets with:
   - Target MAC address (spoofed source)
   - Reason code: `0x13` (Remote user terminated connection)
   - Transaction ID for link identification
4. Multiple channels targeted for success redundancy

#### **Phase 4: Disconnection**
- Audio device receives legitimate-looking **LMP_DETACH** command
- Device validates source MAC matches active connection
- Speaker cleanly disconnects, showing to user as normal disconnection
- Original source device still shows "connected" (depending on timeout)

### Why This Works

✅ **No Broad Jamming** → Avoids detecting Wi-Fi disruption  
✅ **Selective Attack** → Only targets audio devices via MAC  
✅ **LMP Exploitation** → Layer 2 protocol lacks rigorous authentication in many devices  
✅ **Clean Disconnect** → Appears as user action, not equipment failure  

### Why It's Limited

⚠️ **Not all speakers vulnerable** → Modern devices implement tighter LMP validation  
⚠️ **Range depends on nRF24 power** → PA+LNA gives ~100m outdoor  
⚠️ **Timing sensitivity** → Hopping sequence must align with target  
⚠️ **Single attack** → May not work with paired/authenticated links  

---

## 📦 Project Structure

```
src/v2_0_bluetooth_deauth/
├── main.cpp                 # Entry point + FreeRTOS setup
├── config.h                 # Hardware & feature configuration
├── utils.h                  # MAC formatting, RSSI conversion, etc.
├── display_manager.h/cpp    # OLED UI + state rendering
├── button_handler.h/cpp     # Button input with debouncing
├── bt_sniffer.h/cpp         # NimBLE scanning + audio device detection
├── nrf24_injector.h/cpp     # RF24 packet injection engine
└── attack_manager.h/cpp     # Main state machine + orchestration

include/v2_0_bluetooth_deauth/
└── [Header copies for organization]

lib/
├── Adafruit_SSD1306/        # OLED display driver
├── Adafruit_GFX_Library/    # Graphics primitives
├── RF24/                    # nRF24 radio control
└── NimBLE-Arduino/          # Bluetooth stack
```

---

## ⚡ Build Flags & Configuration

### Key `config.h` Parameters

```cpp
// Device
#define DEVICE_LOLIN32_LITE
#define VERSION "2.0.0-beta"

// Scanning
#define BLE_SCAN_DURATION 10          // seconds
#define MAX_TARGETS 10                // max devices to track

// Attack
#define ATTACK_PACKET_COUNT 100       // packets per cycle
#define ATTACK_INTERVAL 100           // ms between cycles
#define LMP_DETACH_OPCODE 0x05        // Link Manager opcode

// Debug
#define DEBUG_ENABLED true            // Serial output
#define SERIAL_BAUD 115200
```

---

## 🔧 Customization & Future Versions

### Planned Enhancements (v2.1+)
- [ ] Multi-target simultaneous attacks
- [ ] Packet injection rate optimization
- [ ] EEPROM storage of successful targets
- [ ] Frequency hopping algorithm refinement
- [ ] Web interface for remote control
- [ ] Bluetooth Classic sniffing (raw link layer)
- [ ] Statistics and attack reports

### Extensibility
- **Add new attack vectors**: Modify `nrf24_injector.cpp`
- **Change UI layout**: Update `display_manager.cpp`
- **Adjust detection**: Tweak `bt_sniffer.cpp` audio keywords
- **Enable logging**: Set `DEBUG_VERBOSE true` in `config.h`

---

## 📚 References & Documentation

### Bluetooth Specifications
- [Bluetooth Core Spec v5.3](https://www.bluetooth.com/specifications/specs/) - Link Manager Protocol (LMP)
- [BLE Appearance Codes](https://www.bluetooth.com/specifications/gatt/) - Device type identification

### Related Research
- **Link Manager Vulnerabilities**: CVE patterns in Bluetooth stacks
- **MAC Spoofing**: RF layer manipulation techniques
- **nRF24 Packet Injection**: Extended capabilities beyond standard protocol

### Tools Used
- **PlatformIO** - Build system
- **NimBLE** - Lightweight BLE stack
- **RF24 Library** - nRF24 control
- **Adafruit GFX** - Display rendering

---

## ⚠️ Legal & Ethical Notice

This project is designed for **educational and authorized security research only**.

**Disclaimer:**
- This tool exploits known vulnerabilities in Bluetooth implementations
- Unauthorized access to or disruption of wireless devices may be illegal
- Users are responsible for compliance with local laws and regulations
- This project comes with NO WARRANTY of any kind
- Use only on devices you own or have explicit permission to test

---

## 🤝 Contributing

Contributions are welcome! Areas for improvement:
- Compatibility testing with various Bluetooth speakers
- Optimization of LMP packet injection
- UI/UX enhancements
- Documentation improvements

Submit pull requests to the `v2.0-bluetooth-deauth` branch.

---

## 📄 License

This project retains the original **MIT License** from the nRF24_jammer repository.

---

## 🙏 Acknowledgments

- Original **nRF24_jammer** project structure
- **NimBLE** library for lightweight BLE stack
- **RF24** library for nRF24 hardware abstraction
- Bluetooth research community for vulnerability documentation

---

**v2.0 Release Date**: May 2026  
**Status**: Beta / Proof of Concept  
**Maintainer**: Fantanchero  

For issues, questions, or research collaboration, open an issue on GitHub.

---

### Quick Start Checklist
- [ ] Soldered all connections per pinout
- [ ] Installed PlatformIO + ESP32 package
- [ ] Built project successfully
- [ ] Flashed firmware to LOLIN32 Lite
- [ ] Tested OLED display visibility
- [ ] Confirmed button responsiveness
- [ ] Scanned for nearby Bluetooth speakers
- [ ] Selected target and initiated attack
- [ ] Observed speaker disconnection behavior
- [ ] Documented results for analysis

