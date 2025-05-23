#include "interconnect.h"
#include "memory.h"
#include "PEs.h"
#include <iostream>
#include <chrono>
#include <fstream>
#include "evento.h" //añadido del eventoo
#include <unordered_map>
#include <string>
#include <iomanip>
#include <algorithm>

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

    std::string key;
    if (msg.type == MessageType::READ_RESP || msg.type == MessageType::WRITE_RESP || msg.type == MessageType::INV_COMPLETE) {
        // Las respuestas usan DEST como clave (el PE original que hizo la petición)
        key = std::to_string(msg.DEST) + "_" + std::to_string(msg.ADDR);
    } else {
        // Las instrucciones originales usan SRC como clave
        key = std::to_string(msg.SRC) + "_" + std::to_string(msg.ADDR);
    }

    // Guardar en tiempo real la estadística de la instrucción
    auto &stat = instruction_stats[key];
    stat.enqueue_time = global_clock;

    // Encolar mensaje en la cola correspondiente
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

    std::ofstream steps_file("steps.txt", std::ios::trunc);
    if (!steps_file.is_open()) {
        std::cerr << "Error al abrir steps.txt\n";
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
        //std::string response_key;
        //if (msg.type == MessageType::READ_RESP || msg.type == MessageType::WRITE_RESP || msg.type == MessageType::INV_COMPLETE) {
           // response_key = std::to_string(msg.DEST) + "_" + std::to_string(msg.ADDR);
        //} else {
         //   response_key = std::to_string(msg.SRC) + "_" + std::to_string(msg.ADDR);
        //}

        // Registrar tiempo de entrada a procesamiento
        std::string key;
        if (msg.type == MessageType::READ_RESP || msg.type == MessageType::WRITE_RESP || msg.type == MessageType::INV_COMPLETE) {
            key = std::to_string(msg.DEST) + "_" + std::to_string(msg.ADDR);
        } else {
            key = std::to_string(msg.SRC) + "_" + std::to_string(msg.ADDR);
        }
        auto &stat = instruction_stats[key];
        stat.start_process_time = global_clock;

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
            eventoCount = 1;  
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
        if (eventoCount != 0 && bytes != 0) {
            stat.finish_time = global_clock;
        
            uint64_t en_cola = (stat.start_process_time >= stat.enqueue_time)
            ? (stat.start_process_time - stat.enqueue_time)
            : 0;

            // Penalizacion por Memoria
            if (msg.type == MessageType::READ_MEM || msg.type == MessageType::WRITE_RESP) {
                en_cola += 5;  // No pudimos sumar el tiempo dinamico de la espera. Lo sumamos de esta forma
                //para simular el efecto de la espera. 
            }
        
            uint64_t ciclos_instr = eventoCount + en_cola;   // ciclos que costó esta instrucción
            uint64_t bytes_instr  = bytes;                   // bytes que transfirió
        
            // 2)  ACTUALIZA los acumulados
            total_ciclos_programa += ciclos_instr;
            total_bytes_programa  += bytes_instr;
        
            steps_file << "Instr (PE " << evento_pe_id
            << " - " << messageTypeToString(evento_tipo_instruccion) << ") "
            << "Ciclos hasta el momento: " << total_ciclos_programa << " "
            << "Bytes: " << bytes_instr << "\n";
        

            log_file << "Instr (PE " << evento_pe_id
            << " - " << messageTypeToString(evento_tipo_instruccion) << ") "
            << "Ciclos totales: " << (eventoCount + en_cola) << " ciclos, "
            << "En cola: " << en_cola << " ciclos, "
            << "Conteo de ciclos (eventos): " << eventoCount << " ciclos, "
            << "Bytes: " << bytes << ", "
            << "Metrica BW: ";
        
            if ((eventoCount + en_cola) > 0) {
                double bw = static_cast<double>(bytes) / static_cast<double>(eventoCount + en_cola);
            
                if (bw < 1.0) {
                    // Mostramos con decimales si es menor a 1
                    log_file << std::fixed << std::setprecision(3) << bw;
                } else {
                    // Mostramos como entero si es 1 o mayor
                    log_file << static_cast<uint64_t>(bw);
                }
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
    steps_file.close();
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
            // Validaciones para WRITE_MEM
            if (msg.ADDR % 4 != 0) {
                std::cerr << " -> ERROR: Dirección " << msg.ADDR << " no está alineada a 4 bytes.\n";
                break;
            }

            if (msg.ADDR % 16 != 0) {
            std::cerr << " -> ERROR: Dirección " << msg.ADDR << " no está alineada a una línea de caché ("
                    << 16 << " bytes).\n";
            break;
}
            if (msg.ADDR + msg.DATA.size() > 4096) {
                std::cerr << " -> ERROR: La escritura excede el límite de memoria (4096 bytes).\n";
                break;
            }

            if (msg.DATA.empty()) {
                std::cerr << " -> ERROR: No se especificaron datos para escribir (DATA vacío).\n";
                break;
            }
            if (msg.DATA.size() > 2048) {  // Límite práctico: tamaño máximo de escritura aceptable
                std::cerr << " -> ADVERTENCIA: La escritura excede tamaño máximo recomendado (2048 bytes).\n";
            }

            if (msg.ADDR + msg.DATA.size() > 4096) {
                std::cerr << " -> ERROR: Escritura excede el límite de memoria.\n";
                break;
            }

            if (msg.NUM_OF_CACHE_LINES * CACHE_LINE_SIZE != msg.DATA.size()) {
                std::cerr << " -> ERROR: Tamaño de DATA no coincide con número de líneas especificadas.\n";
                break;
            }

            if (msg.NUM_OF_CACHE_LINES > 129) {
                std::cerr << " -> ERROR: Tamaño de NUM_OF_CACHE_LINES invalido.\n";
                break;
            }



            if (!memory) {
                std::cerr << " -> Memoria no conectada.\n";
                break;
            }

            //3 eventoos, 11 bytes 3 lectura de instruc y uno por cada linea de cache. 
            //2 eventoos de lectura
            evento *evento1 = new evento("lectura", msg.SRC,4,msg.type);
            evento *evento2 = new evento("lectura", msg.SRC,4,msg.type);
            evento *evento3 = new evento("lectura", msg.SRC,4,msg.type);
            this->eventoq->addevento(evento1);
            this->eventoq->addevento(evento2);
            this->eventoq->addevento(evento3);
            //eventoos de lectura/escritura
            std::size_t byte_count = msg.DATA.size();
            std::size_t num_eventos = (byte_count + 15) / 16;  // cada 16 bytes generan 1 evento

            for (std::size_t i = 0; i < num_eventos; ++i) {
                evento *lcevento = new evento("Memoria", msg.SRC, 4, msg.type);
                this->eventoq->addevento(lcevento);
            }


            uint32_t status = 0x0;
            try {
                memory->writeBlock(msg.ADDR, msg.DATA);
                std::cout << " -> Datos escritos (" << msg.DATA.size() << " bytes): ";
                for (uint8_t byte : msg.DATA) {
                    std::cout << std::hex << static_cast<int>(byte) << " ";
                }
                std::cout << std::dec << "\n";

                status = 0x1; // Indica éxito
            } catch (const std::out_of_range& e) {
                std::cerr << " -> Error: " << e.what() << "\n";
            }

            // Enviar WRITE_RESP al PE origen
            Message response = {
                MessageType::WRITE_RESP,
                -1,            // SRC no aplica
                msg.SRC,       // DEST: PE que hizo la solicitud
                msg.ADDR,
                0,             // SIZE
                0,
                std::vector<uint8_t>{ static_cast<uint8_t>(status) },        // STATUS como DATA (1 byte)
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

            // alineacion
            if (msg.ADDR % 4 != 0) {
                std::cerr << " -> ERROR: Dirección " << msg.ADDR << " no está alineada a 4 bytes.\n";
                break;
            }

            // memoria
            if (msg.ADDR + msg.SIZE > 4096) {
                std::cerr << " -> ERROR: La lectura excede el límite de memoria (4096 bytes).\n";
                break;
            }

            // SIZE
            if (msg.SIZE <= 0) {
                std::cerr << " -> ERROR: SIZE inválido (debe ser >0).\n";
                break;
            }
            if (msg.SIZE > 2048) {  // límite práctico: tamaño máximo del cache
                std::cerr << " -> ADVERTENCIA: SIZE excede tamaño del cache PE (2048 bytes).\n";
            }

            if (!memory) {
                std::cerr << " -> Memoria no conectada.\n";
                break;
            }

            try {
                //para leer bloque completo. 
                std::vector<uint8_t> result = memory->readBlock(msg.ADDR, msg.SIZE);
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
                    << " with data (" << msg.DATA.size() << " bytes): ";

            for (uint8_t byte : msg.DATA) {
                std::cout << std::hex << static_cast<int>(byte) << " ";
            }
            std::cout << std::dec << "\n";

            if (pe_map.find(msg.DEST) != pe_map.end()) {
                pe_map[msg.DEST]->receiveMessage(msg);

                evento* evento_fijo = new evento("lectura_fija", msg.DEST, 4, msg.type);
                this->eventoq->addevento(evento_fijo);

                // ✅ Ahora usamos el tamaño real del vector de datos
                int byte_count = msg.DATA.size();
                if (byte_count == 0) byte_count = 1;  // Por seguridad

                int num_eventos = (byte_count + 15) / 16;  // Cada 16 bytes suma un evento
                if (num_eventos == 0) num_eventos = 1;     // Mínimo 1 evento

                std::cout << "[IC] Generando " << num_eventos 
                        << " eventoss de ejecucion para bloque de " 
                        << byte_count << " bytes\n";

                for (int i = 0; i < num_eventos; ++i) {
                    evento* evento_exec = new evento("execute", msg.DEST, 16, msg.type);
                    this->eventoq->addevento(evento_exec);
                }

            } else {
                std::cerr << " -> PE " << msg.DEST << " no registrado.\n";
            }

            break;
        }

        case MessageType::WRITE_RESP: {
            std::cout << "[IC] WRITE_RESP to PE" << msg.DEST
                    << " with status (" << msg.DATA.size() << " bytes): ";
            for (uint8_t byte : msg.DATA) {
                std::cout << std::hex << static_cast<int>(byte) << " ";
            }
            std::cout << std::dec << "\n";

        
            if (pe_map.find(msg.DEST) != pe_map.end()) {
                pe_map[msg.DEST]->receiveMessage(msg);
                // 1 eventoo escritura, 1 execute
                evento* evento_lectura = new evento("lectura", msg.DEST, 4, msg.type);
                this->eventoq->addevento(evento_lectura);
                evento* evento_execute = new evento("execute", msg.DEST, 4, msg.type);
                this->eventoq->addevento(evento_execute);

                // ✅ Ahora usamos el tamaño real del vector de datos
                int byte_count = msg.DATA.size();
                if (byte_count == 0) byte_count = 1;  // Por seguridad

                int num_eventos = (byte_count + 15) / 16;  // Cada 16 bytes suma un evento
                if (num_eventos == 0) num_eventos = 1;     // Mínimo 1 evento

                std::cout << "[IC] (wr) Generando " << num_eventos 
                        << " eventoss de ejecucion para bloque de " 
                        << byte_count << " bytes\n";

                for (int i = 0; i < num_eventos; ++i) {
                    evento* evento_exec = new evento("execute", msg.DEST, 1, msg.type);
                    this->eventoq->addevento(evento_exec);
                }


            } else {
                std::cerr << " -> PE " << msg.DEST << " no registrado.\n";
            }

            break;
        }

        case MessageType::BROADCAST_INVALIDATE: {

            if (msg.CACHE_LINE > 129) {
                std::cerr << " -> ERROR (BI): Tamaño de CACHE_LINE invalido.\n";
                break;
            }

            // 1️⃣  Comprobación temprana
            bool ya_activo = false;
            for (const auto& kv : invalidation_map) {
                const auto& t = kv.second;
                if (t.source_pe == msg.SRC && t.cache_line == msg.CACHE_LINE) {
                    ya_activo = true;
                    break;
                }
            }
            if (ya_activo) {
                std::cout << "[IC] (omitido) BROADCAST duplicado de PE"
                        << msg.SRC << ", línea " << msg.CACHE_LINE << '\n';
                break;                          // ⬅️  Salir ANTES de tocar a los PEs
            }

            // 2️⃣  Ahora sí invalidamos y contamos
            int num_targets = 0;
            for (const auto& [id, pe_ptr] : pe_map) {
                if (id == msg.SRC) continue;
                pe_ptr->invalidateCacheLine(msg.CACHE_LINE);
                ++num_targets;                        // contamos siempre
                evento* ev = new evento("lectura_invalida_por_pe",
                                        msg.SRC, 1, msg.type);
                eventoq->addevento(ev);
            }

            if (num_targets > 0) {
                InvalidationTracker tr;
                tr.expected_acks = num_targets;
                tr.source_pe     = msg.SRC;
                tr.qos           = msg.QoS;
                tr.cache_line    = msg.CACHE_LINE;
                invalidation_map[next_inv_id++] = std::move(tr);
            }
            //aca es 1 de lectura, y 1 de excecute , con 7 bytes, 1 para cada pe
            //al poner 1 bit en 0, (PEOR ESCENARIO , TODOS TIENEN LA LINEA DE CACHE DEL
            //PE QUIEN LA MANDO , CONSULTAR)
            evento *evento1 = new evento("lectura", msg.SRC,4, msg.type);
            evento *evento2 = new evento("excecute que notifica", msg.SRC,1, msg.type);
            this->eventoq->addevento(evento1);
            this->eventoq->addevento(evento2);
            
            break;
        }


        case MessageType::INV_ACK: {
            bool contado = false;

            for (auto it = invalidation_map.begin();
                it != invalidation_map.end() && !contado; /* ++ dentro */) {

                auto &tracker = it->second;

                // ¿PE repetido para ESTE tracker?
                if (tracker.recvd.count(msg.SRC)) { ++it; continue; }

                tracker.recvd.insert(msg.SRC);            // lo contamos
                contado = true;

                std::cout << "[IC] INV_ACK recibido de PE"
                        << msg.SRC << " (INV " << it->first << ")\n";

                if (static_cast<int>(tracker.recvd.size()) >= tracker.expected_acks) {
                    std::cout << "[IC] Todos los INV_ACK recibidos para INV "
                            << it->first << "\n";

                    Message complete = {
                        MessageType::INV_COMPLETE,
                        -1,                   // ADDR
                        tracker.source_pe,    // DEST
                        0, 0, 0,              // DATA, SIZE, CACHE_LINE
                        {},                   // (si tienes un vector, ponlo aquí);
                        0, 0,                 // extras
                        tracker.qos
                    };

                    evento *evento_inv = new evento("invalidacion completada", msg.SRC,1, msg.type);

                    enqueueMessage(complete);
                    it = invalidation_map.erase(it);
                } else {
                    ++it;
                }
            }

            if (!contado) {
                std::cerr << "[IC] INV_ACK inesperado de PE"
                        << msg.SRC << " (duplicado en todos los trackers)\n";
            }

            evento *evento1 = new evento("lectura", msg.SRC,3, msg.type);
            evento *evento2 = new evento("excecute", msg.SRC,1, msg.type);
            this->eventoq->addevento(evento1);
            this->eventoq->addevento(evento2);

            break;
            
        }


        case MessageType::INV_COMPLETE: {
            std::cout << "[IC] INV_COMPLETE to PE" << msg.DEST
                      << " (QoS: " << msg.QoS << ")\n";
        
            if (pe_map.find(msg.DEST) != pe_map.end()) {
                // 1 eventoo de lectura de 3bytes 
            evento* evento_lectura = new evento("lectura", msg.DEST, 3, msg.type);
            this->eventoq->addevento(evento_lectura);

            // 1 eventoo de execute de 3bytes
            evento* evento_execute = new evento("execute", msg.DEST, 1, msg.type);
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
