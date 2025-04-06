// main.cpp
#include <iostream>
#include <vector>
#include <thread>
#include "PEs.cpp"
#include "interconnect.cpp"
#include "memory.cpp"
#include "cache.cpp" 



int main() {
    Memory mem;
    Interconnect ic(true);
    ic.attachMemory(&mem);

    // Pre-cargar la memoria para que haya algo que leer
    mem.write(128, 0xCAFEBABE);

    // Arrancamos el hilo del Interconnect
    std::thread ic_thread(&Interconnect::processMessages, &ic);

    // Crear PE
    PE pe0(0, &ic);
    ic.registerPE(0, &pe0); // Para que pueda recibir READ_RESP
    pe0.run();

    // Enviar READ_MEM
    Message readMsg = {
        MessageType::READ_MEM,
        0, -1,            // SRC, DEST
        128,              // ADDR
        4,                // SIZE
        0, 0, 0, 0,
        0x20              // QoS
    };

    ic.enqueueMessage(readMsg);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::exit(0);
}
