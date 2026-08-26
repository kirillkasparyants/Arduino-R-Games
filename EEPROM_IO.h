#ifndef EEPROM_IO_H
#define EEPROM_IO_H

#include <Arduino.h>
#include <Wire.h>

class EEPROM_IO {
private:
    uint8_t _addr;
    bool _is_large;
    void _check();
public:
    uint32_t size = 0;
    EEPROM_IO(uint8_t addr = 0x50);
    void begin();
    void write_b(uint32_t maddr, uint8_t data);
    uint8_t read_b(uint32_t maddr);
    void clear();
};

#endif
