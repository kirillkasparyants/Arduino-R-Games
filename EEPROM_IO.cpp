#include "EEPROM_IO.h"

EEPROM_IO::EEPROM_IO(uint8_t addr) {
    _addr = addr;
}

void EEPROM_IO::begin() {
    Wire.begin();
    _check();
}

void EEPROM_IO::write_b(uint32_t maddr, uint8_t data) {
    Wire.beginTransmission(_addr);
    if (_is_large) {
        Wire.write((uint8_t)(maddr >> 8));
        Wire.write((uint8_t)(maddr & 0xFF));
    } else {
        Wire.write((uint8_t)(maddr & 0xFF));
    }
    Wire.write(data);
    Wire.endTransmission();
    delay(5);
}

uint8_t EEPROM_IO::read_b(uint32_t maddr) {
    Wire.beginTransmission(_addr);
    if (_is_large) {
        Wire.write((uint8_t)(maddr >> 8));
        Wire.write((uint8_t)(maddr & 0xFF));
    } else {
        Wire.write((uint8_t)(maddr & 0xFF));
    }
    Wire.endTransmission(false);
    Wire.requestFrom(_addr, (uint8_t)1);
    return (Wire.available() ? Wire.read() : 0xFF);
}

void EEPROM_IO::_check() {
    bool mode_detected = false;

    _is_large = false;

    uint8_t orig_zero = read_b(0);
    write_b(0, 0xAA);

    uint8_t orig_256 = read_b(256);
    write_b(256, 0x55);

    if (read_b(0) == 0x55) {
        size = 256;
        mode_detected = true;
        write_b(256, orig_256);
    }
    write_b(0, orig_zero);

    if (!mode_detected) {
        _is_large = true;

        uint8_t orig_zero_l = read_b(0);
        write_b(0, 0xAA);

        for (uint32_t addr = 512; addr <= 65536; addr *= 2) {
            uint8_t orig_addr = read_b(addr);
            write_b(addr, 0x55);

            if (read_b(0) == 0x55) {
                size = addr;
                break;
            }
            write_b(addr, orig_addr);
        }
        write_b(0, orig_zero_l);
    }
}

void EEPROM_IO::clear() {
    for (uint32_t i = 0; i != size; i++) write_b(i, 0);
}
