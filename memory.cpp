#include "memory.h"
#include <stdexcept>

Memory::Memory() {
    memory.resize(SIZE, 0); // inicializa toda la memoria en cero
}

void Memory::write(uint32_t addr, uint32_t value) { //escritura en memoria , maneja el addr
    //y el offset
    std::lock_guard<std::mutex> lock(mem_mutex);
    size_t index = addr / 4;

    if (index >= SIZE)
        throw std::out_of_range("Acceso fuera de memoria");

    memory[index] = value;
}

uint32_t Memory::read(uint32_t addr) {
    std::lock_guard<std::mutex> lock(mem_mutex);
    size_t index = addr / 4;

    if (index >= SIZE)
        throw std::out_of_range("Acceso fuera de memoria");

    return memory[index];
}
