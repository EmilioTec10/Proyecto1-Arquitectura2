// main.cpp
#include <iostream>
#include <vector>
#include <thread>
#include "PEs.cpp" 



int main() {
    // Instancia del sistema
    Memory mem;
    Interconnect ic(true); // true = QoS
    ic.attachMemory(&mem);

    // Arrancar hilo de procesamiento
    std::thread ic_thread(&Interconnect::processMessages, &ic);

    // Crear un PE que envía un mensaje
    PE pe0(0, &ic);
    pe0.run();

    // Esperamos un poco para asegurarnos que el mensaje se procese
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Verificamos lectura directa desde main
    uint32_t val = mem.read(64);
    std::cout << "[MAIN] Valor leído en addr 64: 0x" << std::hex << val << std::dec << "\n";

    // Finalizar programa (en producción usarías una bandera para parar el hilo)
    std::exit(0);
}