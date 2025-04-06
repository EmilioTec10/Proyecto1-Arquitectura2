#ifndef MEMORY_H
#define MEMORY_H

#include <vector>
#include <cstdint>
#include <mutex>
#include <stdexcept>

class Memory {
public:
    static constexpr size_t SIZE = 4096; // cantidad de palabras de 32 bits

    Memory() {
        memory.resize(SIZE, 0); // inicializa toda la memoria en cero
    }

    // Escritura: almacena un valor de 32 bits en la posición alineada
    void write(uint32_t addr, uint32_t value) {
        std::lock_guard<std::mutex> lock(mem_mutex);
        size_t index = addr / 4;

        if (index >= SIZE)
            throw std::out_of_range("Acceso fuera de memoria");

        memory[index] = value;
    }

    // Lectura: devuelve un valor de 32 bits desde una posición alineada
    uint32_t read(uint32_t addr) {
        std::lock_guard<std::mutex> lock(mem_mutex);
        size_t index = addr / 4;

        if (index >= SIZE)
            throw std::out_of_range("Acceso fuera de memoria");

        return memory[index];
    }

    // Lectura de múltiples palabras (útil para bloques)
    std::vector<uint32_t> readBlock(uint32_t addr, size_t num_words) {
        std::lock_guard<std::mutex> lock(mem_mutex);
        std::vector<uint32_t> block;
        size_t start = addr / 4;

        if (start + num_words > SIZE)
            throw std::out_of_range("Lectura fuera de rango");

        for (size_t i = 0; i < num_words; ++i) {
            block.push_back(memory[start + i]);
        }
        return block;
    }

private:
    std::vector<uint32_t> memory; // cada posición es de 4 bytes (32 bits)
    std::mutex mem_mutex; // para asegurar acceso concurrente seguro
};

#endif
