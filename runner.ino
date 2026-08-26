#include "EEPROM_IO.h"

EEPROM_IO io(0x50);

void run() {
    uint32_t crs = 3;
    runner:
    switch (io.read_b(crs)) {
        case 0x00: // nop
            continue;
        case 0x01: // tone
            ((io.read_b(++crs) << 2) | io.read_b(++crs) > 1) ? (tone(3, (io.read_b(++crs) << 2) | io.read_b(++crs))) : noTone(3);
            break;
        case 0x02: // delay
            delay((io.read_b(++crs) << 2) | io.read_b(++crs));
            break;
        case 0xFF: // end of code
            return;
    }
    crs++;
    goto runner;
}

void setup() {
    io.begin();
    Serial.begin(9600);
    while(!Serial);
    if (io.read_b(0) == 'A' and io.read_b(1) == 'R' and io.read_b(2) == 'G') {
        Serial.println("Чип содержит валидную игру.");
        run();
    }
    else {
        Serial.println("Чип содержит не валидную игру или данные другого типа.");
        
    }
}

void loop() {}
