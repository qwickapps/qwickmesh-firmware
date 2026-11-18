# QwickMesh LilyGO T-Deck board port

## Scope

This port reuses the upstream MeshCore LilyGO T-Deck definition as a QwickMesh BLE companion/client. It does not add or redesign user-interface behaviour. CI builds `LilyGo_TDeck_companion_radio_ble`.

## Board metadata

| MCU / toolchain board | Radio | Companion link | Update path |
|---|---|---|---|
| ESP32-S3 / `t-deck` | SX1262 | BLE | USB CDC through PlatformIO |

The definition enables PSRAM and uses the ESP32-S3 board profile. External PSRAM is an enhancement, not a substitute for the baseline internal-RAM requirement. The linker-reported firmware RAM budget and image use are captured in CI; the selected application partition must retain at least 768 KiB, and available internal firmware RAM must remain at least 224 KiB.

## Pin and radio map

The SX1262 uses NSS 9, reset 17, BUSY 13, DIO1/IRQ 45, SPI SCLK 40, MISO 38, and MOSI 41. The port enables the 1.8 V TCXO and boosted RX gain, does not use DIO2 RF switching, sets the radio-current limit to 140 mA, and configures a 22 dBm maximum. Display wiring is ST7789: CS 12, DC 11, SCL 40, SDA 41, backlight control 42, with no reset or VDD control pin. The trackball button is GPIO 0; peripheral power is GPIO 10. GPS is on RX 43 and TX 44 at 38,400 baud.

The build default is 869.618 MHz. A release must configure a permitted regional plan and legal transmit power before any radio test.

## Optional and known limits

Keyboard/trackball and display support are inherited from the existing upstream board implementation; this port introduces no QwickMesh UI work. GPS and environmental sensors are optional for the baseline mesh role. BLE pairing, configuration transfer, and reconnect-after-reboot require physical validation. Radio range, RF stability, display usability, and keyboard input are unverified until tested on a physical T-Deck.

## Validation checklist

- [x] Board metadata, build environment, radio type, pin map, update path, and BLE companion transport are documented.
- [ ] Clean companion-role build and size report: recorded by this PR's CI run.
- [ ] First flash and recovery on physical hardware.
- [ ] Cold boot restores identity and configuration.
- [ ] Regional transmit/receive test with recorded frequency and power.
- [ ] Mesh interoperability with a released QwickMesh node.
- [ ] BLE pairing, configuration transfer, and reconnect after reboot.
- [x] Keyboard/display are inherited upstream features; no new UI behaviour is claimed.
- [x] Optional GPS/sensor features are not required for the QwickMesh mesh role.
