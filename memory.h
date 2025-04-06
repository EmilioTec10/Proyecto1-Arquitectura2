#ifndef MEMORY_H
#define MEMORY_H

#include <vector>
#include <cstdint>
#include <mutex>

class Memory {
public:
    static constexpr size_t SIZE = 4096; // cantidad de palabras de 32 bits

    Memory();

    void write(uint32_t addr, uint32_t value);
    uint32_t read(uint32_t addr);
    std::vector<uint32_t> readBlock(uint32_t addr, size_t num_words);

private:
    std::vector<uint32_t> memory; // cada posición es de 4 bytes (32 bits)
    std::mutex mem_mutex; // para asegurar acceso concurrente seguro
};

#endif
