// main.cpp
#include <iostream>
#include <vector>
#include <thread>
#include <memory> // para std::unique_ptr
#include <sstream> // para construir nombres de archivo
#include "PEs.h"
#include "interconnect.h"
#include "memory.h"
#include "cache.h" 

int main() {
    constexpr int NUM_PES = 8;
    Memory mem;
    Interconnect ic(true);
    ic.attachMemory(&mem);

    // Pre-cargar memoria
    mem.write(128, 0xCAFEBABE);

    // Hilo del interconnect
    std::thread ic_thread(&Interconnect::processMessages, &ic);

    // Crear vectores de PEs y threads
    std::vector<std::unique_ptr<PE>> pes;
    std::vector<std::thread> pe_threads;
    for (int i = 0; i < NUM_PES; ++i) {
        std::ostringstream filename;
        //filename << "Instrucciones/pe" << i <<https://prod.liveshare.vsengsaas.visualstudio.com/join?844EBEFA3462EF69B20757541A07656EBB37 ".txt";
        filename << "/home/mrr79/Documents/Arqui 2/Proy1/Proyecto1-Arquitectura2/Instrucciones/pe" << i << ".txt";
    
        pes.push_back(std::make_unique<PE>(i, &ic, filename.str()));
        ic.registerPE(i, pes.back().get());

        pe_threads.emplace_back(&PE::run, pes.back().get());
    }
    // Esperar a que terminen los PEs
    for (auto& t : pe_threads) {
        t.join();
    }
    ic.requestStop();
    ic_thread.join();
    return 0;
}

