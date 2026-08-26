#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <regex>
#include <map>
#include <functional>

using namespace std;

string templ = R"(#include "EEPROM_IO.h"

EEPROM_IO io(0x50);

uint8_t bytecode[] = {%DATA%};
void setup() {
    Serial.begin(9600);
    while(!Serial);
    Serial.println("ЗАПИСЬ ИГРЫ НАЧАЛАСЬ\nВЕРСИЯ SDK: 0.1");
    io.begin();
    io.clear();
    if (sizeof(bytecode) / sizeof(uint8_t) > io.size) {
        Serial.println("ERROR: Ваш чип памяти не поддерживает размер игры! Замените чип и повторите запуск прошивки.");
    } else {
        for (int i = 0; i != sizeof(bytecode) / sizeof(uint8_t); i++) {
            io.write_b(i, bytecode[i]);
            Serial.print("Записан байт "); Serial.print(i); Serial.println("");
        }
    }
}
void loop() {}
)";

vector<uint8_t> bytecode;

void add_byte(uint8_t b) {
    bytecode.push_back(b);
}

void add_word(uint16_t w) {
    add_byte(w & 0xFF);
    add_byte((w >> 8) & 0xFF);
}

void add_dword(uint32_t dw) {
    add_byte(dw & 0xFF);
    add_byte((dw >> 8) & 0xFF);
    add_byte((dw >> 16) & 0xFF);
    add_byte((dw >> 24) & 0xFF);
}

struct Command {
    regex pattern;
    function<void(const smatch&)> handler;
};

map<string, Command> commands;

void register_command(const string& name, const string& pattern_str, function<void(const smatch&)> handler) {
    commands[name] = {regex(pattern_str), handler};
}

void init_commands() {
    register_command("tone", R"(tone\((\d+)\))", [](const smatch& m) {
        uint16_t freq = stoi(m[1].str());
        add_byte(0x01);
        add_word(freq);
    });
    register_command("delay", R"(delay\((\d+)\))", [](const smatch& m) {
        uint16_t time = stoi(m[1].str());
        add_byte(0x02);
        add_word(time);
    });
}

bool parse_line(const string& line) {
    for (auto& [name, cmd] : commands) {
        smatch match;
        if (regex_search(line, match, cmd.pattern)) {
            cmd.handler(match);
            return true;
        }
    }
    return false;
}

string bytecode_to_string() {
    stringstream ss;
    for (size_t i = 0; i < bytecode.size(); i++) {
        if (i > 0) ss << ", ";
        ss << "0x" << hex << uppercase << (int)bytecode[i];
    }
    return ss.str();
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Использование: " << argv[0] << " <ARG-файл>" << endl;
        return 1;
    }

    init_commands();

    ifstream file(argv[1]);
    if (!file.is_open()) {
        cerr << "ERR: невозможно открыть файл " << argv[1] << endl;
        return 1;
    }

    add_byte('A');
    add_byte('R');
    add_byte('G');
    
    string line;
    int line_num = 0;
    while (getline(file, line)) {
        line_num++;
        if (line.empty() || line[0] == '#') continue;

        if (!parse_line(line)) {
            cerr << "WARN: неизвестная команда на линии " << line_num << ": " << line << endl;
        }
    }

    add_byte(0xFF);

    string data = bytecode_to_string();
    string output = templ;
    size_t pos = output.find("%DATA%");
    if (pos != string::npos) {
        output.replace(pos, 6, data);
    }

    cout << output;
    return 0;
}
