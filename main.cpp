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

     // Pre-cargar memoria para prueba
     mem.write(128, 0xCAFEBABE);

     // Lanzar hilo del Interconnect
     std::thread ic_thread(&Interconnect::processMessages, &ic);
 
     // Crear PEs
     PE pe0(0, &ic, "Instrucciones/pe0.txt");
     PE pe1(1, &ic, "Instrucciones/pe1.txt");
 
     ic.registerPE(0, &pe0);
     ic.registerPE(1, &pe1);
 
     // Lanzar PEs en hilos independientes
     std::thread pe0_thread(&PE::run, &pe0);
     std::thread pe1_thread(&PE::run, &pe1);
 
     // Esperar ejecución de los PEs
     pe0_thread.join();
     pe1_thread.join();
 
     // Esperar que el Interconnect procese todo antes de terminar
     std::this_thread::sleep_for(std::chrono::milliseconds(100));
     std::exit(0);
}
