#include "interconnect.h"
#include "memory.h"
#include "PEs.h"
#include <iostream>
#include <chrono>
#include <fstream>
#include "evento.h" //añadido del eventoo
#include <unordered_map>
#include <string>
struct InstructionTiming {
    uint64_t enqueue_time = 0;
    uint64_t start_process_time = 0; //se mete en la cola
    uint64_t finish_time = 0; //termina hasta que
    uint32_t bytes = 0;
};

std::string messageTypeToString(MessageType type) {
    switch (type) {
        case MessageType::WRITE_MEM: return "WRITE_MEM";
        case MessageType::READ_MEM: return "READ_MEM";
        case MessageType::WRITE_RESP: return "WRITE_RESP";
        case MessageType::READ_RESP: return "READ_RESP";
        case MessageType::BROADCAST_INVALIDATE: return "BROADCAST_INVALIDATE";
        case MessageType::INV_ACK: return "INV_ACK";
        case MessageType::INV_COMPLETE: return "INV_COMPLETE";
        default: return "UNKNOWN";
    }
}
//bytes_totales_eventoos , pe_id
//nota , al revisar el encolador q0s, se debe de garantizar que 
//al añadir el eventoo ymluego sacarlo, si tiene un mismo qos, ver si se debe resolver
//o si entra dual al bus, y entraria por eventoos dobles triples etc, pregutnar al profe
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



//se encola el mensaje
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
//se hace pop a la cola del mensaje
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
        Message msg = fifo_queue.front(); //obtiene el mensaje
        fifo_queue.pop(); //le hace pop al mensaje
        return msg;
    }
    //aca se obtiene el mensaje, se crean los eventoos.

}
//aca se cuenta los ciclos y los bytes
//pendiente en readResponse.
//falta una verificacion de que exista un readmem para un read response


void Interconnect::processMessages() {
    uint64_t total_bytes_programa = 0;
    uint64_t total_ciclos_programa = 0;
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
        handleMessage(msg); //aca procesa el mensaje , deberia de llamar a la clase eventoo. TO TEST
        
        global_clock++;
        // Registrar finalización si es una respuesta final al PE
        //le hace un proceso de dequeue
        int flag = 0;
        int eventoCount=0;
        //verifico que el puntero no sea nulo
        int evento_pe_id=0;
        std::string e_nombre = "";
        MessageType evento_tipo_instruccion; 
        int bytes = 0;
        
        evento *mi_evento = nullptr; //declara como nullptr
        if (eventoq->getHead() != nullptr){
            mi_evento = eventoq->popevento(); //aca obtengo el evento
            //ya que checkee que no este nulo el nodo
            evento_pe_id= mi_evento->getpe_id();
            bytes+= mi_evento->getBytes();
            evento_tipo_instruccion = mi_evento->getTipoInstruccion();
            delete mi_evento;
            while (flag !=1) {
                if (eventoq->getHead() != nullptr){
                    if(eventoq->getHead()->getevento()->getpe_id()==evento_pe_id){
                        mi_evento = eventoq->popevento(); //aca obtengo el evento
                        bytes+= mi_evento->getBytes();
                        eventoCount +=1;
                        delete mi_evento;
                    }
                    else{
                        flag=1;
                    }
                }
                else{ //si es nulo sale
                    flag=1;
                }



            }
                
        }

        
        //const auto& stat = instruction_stats[response_key];
        if (eventoCount!=0 && bytes !=0){
            uint64_t en_cola = 0;
            if (msg.type == MessageType::WRITE_RESP || msg.type == MessageType::READ_RESP) {
                // Usamos el DEST porque el PE que recibe es el que inició la instrucción
                std::string response_key = std::to_string(msg.DEST) + "_" + std::to_string(msg.ADDR);
                instruction_stats[response_key].finish_time = global_clock;
                const auto& stat = instruction_stats[response_key];
                en_cola = stat.start_process_time - stat.enqueue_time;
                //log_file << "Instr (" << response_key << ") En cola: "
                  //      << en_cola << " ciclos\n";
            }
            log_file << "Instr (PE " << evento_pe_id
                << " - " << messageTypeToString(evento_tipo_instruccion) << ") "
                << "Ciclos totales: " << (eventoCount + en_cola) << " ciclos, "
                << "En cola: " << en_cola << " ciclos, "
                << "Conteo de ciclos (eventos): " << eventoCount << " ciclos, "
                << "Bytes: " << bytes << ", "
                << "Metrica BW: ";
            
            if ((eventoCount + en_cola) > 0) {
                log_file << (bytes / (eventoCount + en_cola));
            } else {
                log_file << "N/A";
            }
            log_file << "\n";

            total_bytes_programa += bytes;
            total_ciclos_programa += (eventoCount + en_cola);

            

            // Instr rd es la direccion a la que esta accediendo
            // en cola: desde que se encolo hasta qantes de emepzar a procesarse
            // transfer: desde procesarse hasta que se completa operacion= generar respuesta. 
            // BYTES ya sirve
        }
            
        
    }

    log_file << "\n=============================================\n";
    log_file << "BW TOTAL DEL PROGRAMA:\n";
    log_file << "Bytes totales: " << total_bytes_programa << "\n";
    log_file << "Ciclos totales: " << total_ciclos_programa << "\n";
    log_file << "BW total: ";

    if (total_ciclos_programa > 0) {
        log_file << (static_cast<double>(total_bytes_programa) / total_ciclos_programa) << "\n";
    } else {
        log_file << "N/A\n";
    }
    log_file << "=============================================\n";

    std::cout << "[IC] Finalizó procesamiento de mensajes.\n";
    log_file.close();
}


void Interconnect::attachMemory(Memory* mem) {
    memory = mem;
}

void Interconnect::registerPE(int id, PE* pe) {
    pe_map[id] = pe;
}
//en este metodo se realiza el procesado , se indica que se hace 
//aca es donde deberia de estar el eventoo de procesado , lectura y lectura/escritura
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
            //3 eventoos, 11 bytes 3 lectura de instruc y uno por cada linea de cache. 
            //2 eventoos de lectura
            evento *evento1 = new evento("lectura", msg.SRC,4,msg.type);
            evento *evento2 = new evento("lectura", msg.SRC,4,msg.type);
            evento *evento3 = new evento("lectura", msg.SRC,4,msg.type);
            this->eventoq->addevento(evento1);
            this->eventoq->addevento(evento2);
            this->eventoq->addevento(evento3);
            //eventoos de lectura/escritura
            for(int i=0;i<msg.NUM_OF_CACHE_LINES;i++){//hay que verificar que contiene numofcachelines
                evento *lcevento = new evento("Memoria", msg.SRC,16,msg.type);
                this->eventoq->addevento(lcevento); //añadimos N eventoos por cantidad de lineas de cache.
            }
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
                //2 eventoos de lectura
                evento *evento1 = new evento("lectura", msg.SRC,4, msg.type);
                evento *evento2 = new evento("lectura", msg.SRC,4, msg.type);
                evento *evento3 = new evento("excecute", msg.SRC,2, msg.type);
                this->eventoq->addevento(evento1);
                this->eventoq->addevento(evento2);
                this->eventoq->addevento(evento3);
                
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
                // 1 eventoo escritura, 1 execute
                evento* evento_lectura = new evento("lectura", msg.DEST, 4, msg.type);
                this->eventoq->addevento(evento_lectura);
                evento* evento_execute = new evento("execute", msg.DEST, 4, msg.type);
                this->eventoq->addevento(evento_execute);


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
            //aca es 1 de lectura, y 1 de excecute , con 7 bytes, 1 para cada pe
            //al poner 1 bit en 0, (PEOR ESCENARIO , TODOS TIENEN LA LINEA DE CACHE DEL
            //PE QUIEN LA MANDO , CONSULTAR)
            evento *evento1 = new evento("lectura", msg.SRC,4, msg.type);
            evento *evento2 = new evento("excecute", msg.SRC,7, msg.type);
            this->eventoq->addevento(evento1);
            this->eventoq->addevento(evento2);
            
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

                    // 3 bytes, 1 eventoo:  1 de escritura y 1 de execute. 
                    evento *evento1 = new evento("lectura", msg.SRC,3, msg.type);
                    evento *evento2 = new evento("execute", msg.SRC,3, msg.type);
                    this->eventoq->addevento(evento1);
                    this->eventoq->addevento(evento2);

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
                // 1 eventoo de lectura de 3bytes 
            evento* evento_lectura = new evento("lectura", msg.DEST, 4, msg.type);
            this->eventoq->addevento(evento_lectura);

            // 1 eventoo de execute de 3bytes
            evento* evento_execute = new evento("execute", msg.DEST, 4, msg.type);
            this->eventoq->addevento(evento_execute);
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
