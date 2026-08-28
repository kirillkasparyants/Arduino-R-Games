#include "EEPROM_IO.h"

EEPROM_IO io(0x50);

void run() {
    Serial.println("Запуск программы с EEPROM...");
    Serial.print("Размер чипа: ");
    Serial.println(io.size);
    uint32_t crs = 3;
    runner:
    switch (io.read_b(crs)) {
        case 0x00: { // nop
            break;
        }
        case 0x01: { // tone
            crs++;
            uint16_t freq = io.read_b(crs) | (io.read_b(crs + 1) << 8);
            if (freq > 0) tone(3, freq);
            else noTone(3);
            crs++;
            break;
        }
        case 0x02: { // delay
            crs++;
            uint16_t duration = io.read_b(crs) | (io.read_b(crs + 1) << 8);
            delay(duration);
            crs++;
            break;
        }
        case 0xAA: {
            Serial.println("Ошибка получения данных с EEPROM.");
            return;
            break;
        }
        case 0xFF: { // end of code
            return;
            break;
        }
        default: {
            Serial.print("Неизвестная команда: 0x");
            Serial.println((int)io.read_b(crs), HEX);
            break;
        }
    }
    crs++;
    goto runner;
}

void setup() {
    io.begin();
    Serial.begin(9600);
    while(!Serial);
    
    for (char i = 0; i != 5; i++) {
        digitalWrite(2, HIGH);
        delay(500);
        digitalWrite(2, LOW);
        delay(500);
    }
    digitalWrite(2, HIGH);
    
    tone(3, 1000);
    delay(700);
    noTone(3);
    
    if (io.read_b(0) == 'A' and io.read_b(1) == 'R' and io.read_b(2) == 'G') {
        Serial.println("Чип содержит валидную игру.");
        run();
    }
    else {
        Serial.println("Чип содержит не валидную игру или данные другого типа.");
    }
}

void loop() {}
