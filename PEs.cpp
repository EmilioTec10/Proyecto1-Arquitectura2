#include "PEs.h"
#include "interconnect.h"
#include <iostream>


PE::PE(uint8_t id, Interconnect* ic) : pe_id(id), interconnect(ic) {}

void PE::run() {
    std::cout << "[PE " << int(pe_id) << "] Ejecutando...\n";

    Message writeMsg = {
        MessageType::WRITE_MEM,
        pe_id, -1, 64, 0, 0,
        0xABCD1234,
        1, 0, 0x10
    };

    interconnect->enqueueMessage(writeMsg);
}

void PE::receiveMessage(const Message& msg) {
    if (msg.type == MessageType::READ_RESP) {
        std::vector<uint8_t> data(4);
        uint32_t value = msg.DATA;

        data[0] = value & 0xFF;
        data[1] = (value >> 8) & 0xFF;
        data[2] = (value >> 16) & 0xFF;
        data[3] = (value >> 24) & 0xFF;

        for (int i = 0; i < CACHE_LINES; ++i) {
            auto read = cache.read(i * CACHE_LINE_SIZE);
            if (read.empty()) {
                cache.write(i * CACHE_LINE_SIZE, data);
                std::cout << "[PE " << int(pe_id) << "] READ_RESP con el valor 0x" << std::hex << value << std::dec  << " almacenado en bloque #" << i << "\n";
                return;
            }
        }

        std::cout << "[PE " << int(pe_id) << "] No hay bloques libres en cache\n";
    }

    else if (msg.type == MessageType::WRITE_RESP) {
        if (msg.DATA == 0x1) {
            std::cout << "[PE " << int(pe_id) << "] WRITE_RESP recibido: escritura OK\n";
        } else {
            std::cout << "[PE " << int(pe_id) << "] WRITE_RESP recibido: escritura FALLÓ\n";
        }
    }
}
