# QwickMesh LilyGO T-Beam board port

## Scope

This port reuses the upstream MeshCore LilyGO T-Beam definitions for the QwickMesh companion-radio role. The CI environments are `Tbeam_SX1276_companion_radio_ble` and `Tbeam_SX1262_companion_radio_ble`; the radio variants are distinct and must not be flashed interchangeably.

## Board metadata

| Variant | MCU / toolchain board | Radio | Companion link | Update path |
|---|---|---|---|---|
| T-Beam SX1276 | ESP32 / `ttgo-t-beam` | SX1276 | BLE | USB serial through PlatformIO |
| T-Beam SX1262 | ESP32 / `ttgo-t-beam` | SX1262 | BLE | USB serial through PlatformIO |

Both variants use the ESP32 PlatformIO target with a 4 MiB application-partition layout (`min_spiffs.csv`). ESP32 internal RAM is 520 KiB physical; the linker-reported firmware RAM budget and actual image use are recorded by the CI build. That physical memory exceeds the 256 KiB minimum, and the selected partition must retain at least 768 KiB for the application image.

## Pin and radio map

Shared pins: SPI SCLK 5, MISO 19, MOSI 27, NSS 18, reset 23, DIO0 26, DIO1 33, I2C SDA 21, I2C SCL 22, GPS RX 12, GPS TX 34, user button 38, and TX LED 4.

The SX1276 port uses `CustomSX1276` with a 120 mA radio current limit and a 20 dBm configured maximum. The SX1262 port uses `CustomSX1262`, DIO2 RF switching, a 1.8 V TCXO setting, a 140 mA current limit, boosted RX gain, and a 22 dBm configured maximum. The build default is 869.618 MHz; deployments must select a permitted regional plan and legal power before radio testing.

## Optional and known limits

GPS, OLED display, battery/power-management reporting, and sensors are optional to the baseline QwickMesh mesh role. The companion role requires BLE; its stable pairing, configuration transfer, and reconnect-after-reboot behaviour need physical validation. The SX1262 definition does not declare a separate BUSY GPIO, so radio initialization and recovery must be checked on the target hardware before release.

## Validation checklist

- [x] Board metadata, build environment, radio type, pin map, update path, and BLE companion transport are documented.
- [ ] Clean companion-role build and size report: recorded by this PR's CI run.
- [ ] First flash and recovery on physical hardware.
- [ ] Cold boot restores identity and configuration.
- [ ] Regional transmit/receive test with recorded frequency and power.
- [ ] Mesh interoperability with a released QwickMesh node.
- [ ] BLE pairing, configuration transfer, and reconnect after reboot.
- [x] Optional GPS/display/sensor features are not required for the QwickMesh mesh role.
