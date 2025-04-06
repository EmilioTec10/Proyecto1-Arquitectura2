#include "interconnect.h"
#include "memory.h"
#include "PEs.h"
#include <iostream>

Interconnect::Interconnect(bool useQoS) : useQoS(useQoS), memory(nullptr) {}

void Interconnect::enqueueMessage(const Message& msg) {
    std::lock_guard<std::mutex> lock(queue_mutex);
    if (useQoS) {
        qos_queue.emplace(msg.QoS, msg);
    } else {
        fifo_queue.push(msg);
    }
    queue_cv.notify_one();
}

Message Interconnect::getNextMessage() {
    std::unique_lock<std::mutex> lock(queue_mutex);

    // Esperar hasta que haya al menos un mensaje disponible
    queue_cv.wait(lock, [this]() {
        return !fifo_queue.empty() || !qos_queue.empty();
    });

    if (useQoS && !qos_queue.empty()) {
        auto top = qos_queue.top();
        qos_queue.pop();
        return top.second;
    } else {
        Message msg = fifo_queue.front();
        fifo_queue.pop();
        return msg;
    }
}

void Interconnect::processMessages() {
    while (true) {
        Message msg = getNextMessage();
        handleMessage(msg);
    }
}

void Interconnect::attachMemory(Memory* mem) {
    memory = mem;
}

void Interconnect::registerPE(int id, PE* pe) {
    pe_map[id] = pe;
}

void Interconnect::handleMessage(const Message& msg) {
    switch (msg.type) {
        case MessageType::WRITE_MEM: {
            std::cout << "[IC] WRITE_MEM from PE" << msg.SRC
                      << " at address " << msg.ADDR << "\n";
            
            uint32_t status = 0x0;

            if (memory) {
                try {
                    memory->write(msg.ADDR, msg.DATA);
                    std::cout << " -> Datos escritos: 0x" << std::hex << msg.DATA << std::dec << "\n";
                    status = 0x1; // Indica éxito
                } catch (const std::out_of_range& e) {
                    std::cerr << " -> Error: " << e.what() << "\n";
                }
            } else {
                std::cerr << " -> Memoria no conectada.\n";
            }
            // Enviar WRITE_RESP al PE origen
            Message response = {
                MessageType::WRITE_RESP,
                -1,            // SRC no aplica
                msg.SRC,       // DEST: PE que hizo la solicitud
                msg.ADDR,
                0,             // SIZE
                0,
                status,        // STATUS como DATA (1 byte)
                0, 0,
                msg.QoS
            };

            enqueueMessage(response);

            break;
        }

        case MessageType::READ_MEM: {
            std::cout << "[IC] READ_MEM from PE" << msg.SRC
                      << " at address " << msg.ADDR
                      << ", size = " << msg.SIZE << "\n";

            if (!memory) {
                std::cerr << " -> Memoria no conectada.\n";
                break;
            }

            try {
                uint32_t result = memory->read(msg.ADDR);
                Message response = {
                    MessageType::READ_RESP,
                    -1,             // SRC no aplica
                    msg.SRC,        // DEST: PE que pidió
                    msg.ADDR,
                    msg.SIZE,
                    0,
                    result,
                    0, 0,
                    msg.QoS
                };
                enqueueMessage(response);
            } catch (const std::out_of_range& e) {
                std::cerr << " -> Error: " << e.what() << "\n";
            }

            break;
        }

        case MessageType::READ_RESP: {
            std::cout << "[IC] READ_RESP to PE" << msg.DEST
                      << " with data: 0x" << std::hex << msg.DATA << std::dec << "\n";

            if (pe_map.find(msg.DEST) != pe_map.end()) {
                pe_map[msg.DEST]->receiveMessage(msg);
            } else {
                std::cerr << " -> PE " << msg.DEST << " no registrado.\n";
            }

            break;
        }

        case MessageType::WRITE_RESP: {
            std::cout << "[IC] WRITE_RESP to PE" << msg.DEST
                      << " with status: 0x" << std::hex << msg.DATA << std::dec << "\n";
        
            if (pe_map.find(msg.DEST) != pe_map.end()) {
                pe_map[msg.DEST]->receiveMessage(msg);
            } else {
                std::cerr << " -> PE " << msg.DEST << " no registrado.\n";
            }
        
            break;
        }

        default:
            std::cout << "[IC] Mensaje tipo no implementado aún.\n";
            break;
    }
}
