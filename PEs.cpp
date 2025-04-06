#include "PEs.h"
#include "interconnect.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>


PE::PE(uint8_t id, Interconnect* ic, const std::string& instrFile)
    : pe_id(id), interconnect(ic), instruction_file(instrFile) {}

    void PE::run() {
        std::ifstream infile(instruction_file);
        if (!infile.is_open()) {
            std::cerr << "[PE " << int(pe_id) << "] No se pudo abrir el archivo " << instruction_file << "\n";
            return;
        }
    
        std::string line;
        while (std::getline(infile, line)) {
            std::istringstream iss(line);
            std::string opcode;
            uint32_t addr, data_or_size;
            int qos;
    
            iss >> opcode >> std::hex >> addr >> std::hex >> data_or_size >> std::hex >> qos;
    
            if (opcode == "WRITE_MEM") {
                Message msg = {
                    MessageType::WRITE_MEM,  // [0] Tipo de mensaje: escritura
                    pe_id,                   // [1] SRC: ID del PE que está enviando el mensaje
                    -1,                      // [2] DEST: no se usa en WRITE_MEM (por eso -1)
                    addr,                   // [3] ADDR: dirección de memoria a escribir
                    0,                      // [4] SIZE: no se usa en escritura (solo en lectura)
                    0,                      // [5] CACHE_LINE: no se usa aquí
                    data_or_size,           // [6] DATA: dato que se quiere escribir
                    1,                      // [7] NUM_OF_CACHE_LINES: valor fijo, por convención
                    0,                      // [8] START_CACHE_LINE: no usado, se deja 0
                    qos                     // [9] QoS: prioridad del mensaje
                };
                interconnect->enqueueMessage(msg);
            }
            else if (opcode == "READ_MEM") {
                Message msg = {
                    MessageType::READ_MEM,     // [0] Tipo de mensaje: lectura
                    pe_id,                     // [1] SRC: ID del PE que solicita lectura
                    -1,                        // [2] DEST: no aplica, el interconnect responde
                    addr,                      // [3] ADDR: dirección de memoria a leer
                    static_cast<int>(data_or_size),  // [4] SIZE: cuántos bytes leer
                    0,                         // [5] CACHE_LINE: no usado aquí
                    0,                         // [6] DATA: sin uso
                    0, 0,                      // [7,8] NUM_OF_CACHE_LINES, START_CACHE_LINE: no usados
                    qos                        // [9] QoS: prioridad
                };
                interconnect->enqueueMessage(msg);
            }

            else if (opcode == "BROADCAST_INVALIDATE") {
                Message msg = {
                    MessageType::BROADCAST_INVALIDATE,
                    pe_id, -1,
                    0, 0,
                    static_cast<int>(addr), // aquí addr se interpreta como línea
                    0, 0, 0, qos
                };
                interconnect->enqueueMessage(msg);
            }

            else {
                std::cerr << "[PE " << int(pe_id) << "] Instrucción desconocida: " << opcode << "\n";
            }
        }
    
        infile.close();
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

    else if (msg.type == MessageType::INV_COMPLETE) {
        std::cout << "[PE " << int(pe_id) << "] INV_COMPLETE recibido: todas las invalidaciones completadas\n";
    }
}

void PE::invalidateCacheLine(int line) {
    cache.invalidateLine(line);
    std::cout << "[PE " << int(pe_id) << "] Línea " << line << " invalidada\n";

    Message ack = {
        MessageType::INV_ACK,
        pe_id, -1, 0, 0, 0,
        0, 0, 0, 0 
    };
    interconnect->enqueueMessage(ack);
}
