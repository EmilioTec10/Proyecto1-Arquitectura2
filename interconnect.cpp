#include "interconnect.h"
#include "memory.h"
#include "PEs.h"
#include <iostream>
#include <chrono>
#include <fstream>

#include <unordered_map>
#include <string>

struct InstructionTiming {
    uint64_t enqueue_time = 0;
    uint64_t start_process_time = 0;
    uint64_t finish_time = 0;
    uint32_t bytes = 0;
};

// Variables globales
std::unordered_map<std::string, InstructionTiming> instruction_stats;
uint64_t global_clock = 0;

std::map<int, InvalidationTracker> invalidation_map;
int next_inv_id = 0;

std::unordered_map<std::string, std::chrono::steady_clock::time_point> enqueue_times;
namespace std {
    // A
}


Interconnect::Interconnect(bool useQoS) : useQoS(useQoS), memory(nullptr), stop_requested(false) {}


void Interconnect::requestStop() {
    stop_requested = true;
    queue_cv.notify_all(); // para desbloquear el wait
}




void Interconnect::enqueueMessage(const Message& msg) {
    std::lock_guard<std::mutex> lock(queue_mutex);

    std::string key = std::to_string(msg.SRC) + "_" + std::to_string(msg.ADDR);
    instruction_stats[key].enqueue_time = global_clock;
    instruction_stats[key].bytes = (msg.type == MessageType::WRITE_MEM) ? 3 : msg.SIZE;

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
    std::ofstream log_file("message_times.txt", std::ios::trunc);
    if (!log_file.is_open()) {
        std::cerr << "Error al abrir el archivo de registro de tiempos.\n";
        return;
    }

    while (true) {
        Message msg;
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            queue_cv.wait(lock, [this]() {
                return stop_requested || !fifo_queue.empty() || !qos_queue.empty();
            });

            if (stop_requested && fifo_queue.empty() && qos_queue.empty()) {
                break;
            }

            if (useQoS && !qos_queue.empty()) {
                msg = qos_queue.top().second;
                qos_queue.pop();
            } else if (!fifo_queue.empty()) {
                msg = fifo_queue.front();
                fifo_queue.pop();
            } else {
                continue; // en caso de notificación sin contenido
            }
        }

        // Clave única para esta instrucción
        std::string key = std::to_string(msg.SRC) + "_" + std::to_string(msg.ADDR);

        // Registrar tiempo de entrada a procesamiento
        instruction_stats[key].start_process_time = global_clock;

        // Procesar mensaje y avanzar el reloj lógico
        handleMessage(msg);
        global_clock++;

        // Registrar finalización si es una respuesta final al PE
        if (msg.type == MessageType::WRITE_RESP || msg.type == MessageType::READ_RESP) {
            // Usamos el DEST porque el PE que recibe es el que inició la instrucción
            std::string response_key = std::to_string(msg.DEST) + "_" + std::to_string(msg.ADDR);
            instruction_stats[response_key].finish_time = global_clock;

            const auto& stat = instruction_stats[response_key];

            log_file << "Instr (" << response_key << ") Total: "
                     << (stat.finish_time - stat.enqueue_time) << " ciclos, "
                     << "En cola: " << (stat.start_process_time - stat.enqueue_time) << " ciclos, "
                     << "Transfer: " << (stat.finish_time - stat.start_process_time) << " ciclos, "
                     << "Bytes: " << stat.bytes << "\n";
                     // Instr rd es la direccion a la que esta accediendo
                     // en cola: desde que se encolo hasta qantes de emepzar a procesarse
                    // transfer: desde procesarse hasta que se completa operacion= generar respuesta. 
                    // BYTES NO ENTIENDO
        }
        
    }

    std::cout << "[IC] Finalizó procesamiento de mensajes.\n";
    log_file.close();
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

        case MessageType::BROADCAST_INVALIDATE: {
            std::cout << "[IC] BROADCAST_INVALIDATE from PE" << msg.SRC
                      << ", line " << msg.CACHE_LINE << "\n";
        
            int inv_id = next_inv_id++;
            int num_targets = 0;
        
            for (const auto& [id, pe_ptr] : pe_map) {
                if (id != msg.SRC) {
                    pe_ptr->invalidateCacheLine(msg.CACHE_LINE);
                    ++num_targets;
                }
            }
        
            if (num_targets > 0) {
                invalidation_map[inv_id] = {
                    .expected_acks = num_targets,
                    .received_acks = 0,
                    .source_pe = msg.SRC,
                    .qos = msg.QoS
                };
            }
        
            break;
        }

        case MessageType::INV_ACK: {
            std::cout << "[IC] INV_ACK recibido de PE" << msg.SRC << "\n";

            for (auto& [id, tracker] : invalidation_map) {
                tracker.received_acks += 1;

                if (tracker.received_acks >= tracker.expected_acks) {
                    std::cout << "[IC] Todos los INV_ACK recibidos para INV " << id << "\n";

                    Message complete = {
                        MessageType::INV_COMPLETE,
                        -1,
                        tracker.source_pe, // el PE original del broadcast
                        0, 0, 0,
                        0, 0, 0,
                        tracker.qos
                    };

                    enqueueMessage(complete);
                    invalidation_map.erase(id);
                    break; 
                }
            }

            break;
        }

        case MessageType::INV_COMPLETE: {
            std::cout << "[IC] INV_COMPLETE to PE" << msg.DEST
                      << " (QoS: " << msg.QoS << ")\n";
        
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
