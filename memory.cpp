#include "memory.h"
#include <stdexcept>

Memory::Memory() {
    memory.resize(SIZE * 4, 0);  // 4096 posiciones × 4 bytes = 16 KB
}


void Memory::write(uint32_t addr, uint32_t value) {
    std::lock_guard<std::mutex> lock(mem_mutex);

    if (addr + 4 > memory.size())
        throw std::out_of_range("Acceso fuera de memoria");

    // Escribir 4 bytes en little-endian
    memory[addr]     = static_cast<uint8_t>(value & 0xFF);
    memory[addr + 1] = static_cast<uint8_t>((value >> 8) & 0xFF);
    memory[addr + 2] = static_cast<uint8_t>((value >> 16) & 0xFF);
    memory[addr + 3] = static_cast<uint8_t>((value >> 24) & 0xFF);
}

uint32_t Memory::read(uint32_t addr) {
    std::lock_guard<std::mutex> lock(mem_mutex);

    if (addr + 4 > memory.size())
        throw std::out_of_range("Acceso fuera de memoria");

    return static_cast<uint32_t>(memory[addr])
         | (static_cast<uint32_t>(memory[addr + 1]) << 8)
         | (static_cast<uint32_t>(memory[addr + 2]) << 16)
         | (static_cast<uint32_t>(memory[addr + 3]) << 24);
}


std::vector<uint8_t> Memory::readBlock(uint32_t addr, int size) {
    if (addr + size > memory.size()) {
        throw std::out_of_range("Intento de leer fuera de la memoria.");
    }
    return std::vector<uint8_t>(memory.begin() + addr, memory.begin() + addr + size);
}

void Memory::writeBlock(uint32_t addr, const std::vector<uint8_t>& data) {
    if (addr + data.size() > memory.size()) {
        throw std::out_of_range("Intento de escribir fuera de la memoria.");
    }
    std::copy(data.begin(), data.end(), memory.begin() + addr);
}


