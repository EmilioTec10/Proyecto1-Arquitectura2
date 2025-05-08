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
        if (!infile.is_open()) { //trata de leer el archivo de instrucciones
            std::cerr << "[PE " << int(pe_id) << "] No se pudo abrir el archivo " << instruction_file << "\n";
            return;
        }
    
        std::string line;
        while (std::getline(infile, line)) {
            std::istringstream iss(line);
            std::string opcode;
            uint32_t addr, data_or_size;
            int qos;
    
            iss >> opcode >> std::hex >> addr >> data_or_size >> qos;
            if (opcode == "WRITE_MEM") {
                Message msg = {
                    MessageType::WRITE_MEM,  // [0] Tipo de mensaje: escritura
                    pe_id,                   // [1] SRC: ID del PE que está enviando el mensaje
                    -1,                      // [2] DEST: no se usa en WRITE_MEM (por eso -1)
                    addr,                   // [3] ADDR: dirección de memoria a escribir
                    0,                      // [4] SIZE: no se usa en escritura (solo en lectura)
                    0,                      // [5] CACHE_LINE: no se usa aquí
                    std::vector<uint8_t>{ 
                        static_cast<uint8_t>(data_or_size & 0xFF),
                        static_cast<uint8_t>((data_or_size >> 8) & 0xFF),
                        static_cast<uint8_t>((data_or_size >> 16) & 0xFF),
                        static_cast<uint8_t>((data_or_size >> 24) & 0xFF)
                    },           // [6] DATA: dato que se quiere escribir
                    1,                      // [7] NUM_OF_CACHE_LINES: valor fijo, por convención
                    0,                      // [8] START_CACHE_LINE: no usado, se deja 0
                    qos                     // [9] QoS: prioridad del mensaje
                };
                interconnect->enqueueMessage(msg);
                //peso del mensaje en bytes
                //tipo de mensaje = 3 bits, por alineacion , 1 BYTE
                //id del pe = 3 , por alineacion 1 BYTE
                //DEST no se utiliza , no se lee como tal y no deberia de estar, ningun byte pesa
                // direccion de memoria , por alineacion de 4 bytes, accedo a 4096 posiciones
                //entonces necesito 2 BYTES
                //tamaño a leer , no utilizado
                // Cache line no se utiliza tampoco
                //data a enviar a memoria, 4 bytes como maximo, dado que escribe a una direccion 
                //no a multiples , 4 BYTES
                //1 bit para representar la cantidad de lineas de cache
                //inicio de la linea de cache  (ARREGLAR)
                //QOS, en standby la prioridad, consultar
                //TOTAL DE MOMENTO:  8 BYTES , incluye el bit de lineas de cache, 
                //falta agregarle la linea de cache a iniciar , y  el valor del Q0S
                //realmente yo deberia de poder escribir multiples lineas de cache en memoria no solo
                //4 lineas unicamente, revisar esto 
                //por ejemplo deberia de poder decirle empieze en la linea 4 de cache y lea
                //desde la x hasta la x , puede leer unicamente de 4 en 4 bytes, entonces el
                //inicio deberia de ser un multiplo de 4 en la linea de cache
            }
            else if (opcode == "READ_MEM") {
                Message msg = {
                    MessageType::READ_MEM,     // [0]
                    pe_id,                     // [1]
                    -1,                        // [2]
                    addr,                      // [3]
                    static_cast<int>(data_or_size),  // [4] SIZE: cuántos bytes leer
                    0,                         // [5]
                    std::vector<uint8_t>{},    // [6] DATA: sin uso
                    0, 0,                      // [7,8]
                    qos                        // [9]
                };
                interconnect->enqueueMessage(msg);
                //peso del mensaje en bytes
                //tipo de mensaje = 3 bits, por alineacion , 1 BYTE
                //id del pe = 3 , por alineacion 1 BYTE
                //2 bytes para la direccion de memoria a leer
                // 4* n bytes a leer, con n=1  , no debe de sobrepasar el tamaño de la cache
                //falta verificar en que linea escribe , stand by , pero deberia representar 128 en bytes
                //7 y 8 tambien deberian de utilizarse
                //total de momento 4 BYTES + cantidad de bytes a leer.
                //
            }

            else if (opcode == "BROADCAST_INVALIDATE") {
                Message msg = {
                    MessageType::BROADCAST_INVALIDATE,
                    pe_id, -1,
                    0, 0,
                    static_cast<int>(addr), // aquí addr se interpreta como línea
                    std::vector<uint8_t>{}, 0, 0, qos
                };
                interconnect->enqueueMessage(msg);
                //desgloce del mensaje
                //1 byte para el tipo de mensaje
                //1 byte para el peid
                // 1 byte de la linea de cache a invalidar.
            }

            else {
                std::cerr << "[PE " << int(pe_id) << "] Instrucción desconocida: " << opcode << "\n";
            }
        }
    
        infile.close();
    }

void PE::receiveMessage(const Message& msg) {
    //aca esta el guardado de la linea de cache
    //guarda N bytes en cache, que especifica el mensaje como tal
    //dependiendo de la verifcacion , consume o no bytes en el bus.
    if (msg.type == MessageType::READ_RESP) {
        std::vector<uint8_t> data(4);
        uint32_t value = 0;
        for (size_t i = 0; i < std::min((size_t)4, msg.DATA.size()); ++i) {
            value |= (msg.DATA[i] << (8 * i));  // little-endian
        }
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
        if (msg.DATA.size() == 1 && msg.DATA[0] == 0x1) {
            std::cout << "[PE " << int(pe_id) << "] WRITE_RESP recibido: escritura OK\n";
        } else {
            std::cout << "[PE " << int(pe_id) << "] WRITE_RESP recibido: escritura FALLÓ\n";
        }
    }

    else if (msg.type == MessageType::INV_COMPLETE) {
        std::cout << "[PE " << int(pe_id) << "] INV_COMPLETE recibido: todas las invalidaciones completadas\n";
    }
}

void PE::invalidateCacheLine(int line) { //metodo que invalida la linea de cache
    cache.invalidateLine(line);
    std::cout << "[PE " << int(pe_id) << "] Línea " << line << " invalidada\n";

    Message ack = { //message aknowledge
        MessageType::INV_ACK,
        pe_id, -1, 0, 0, 0,
        std::vector<uint8_t>{}, 0, 0, 0 
    };
    interconnect->enqueueMessage(ack);
    //toma 
}
