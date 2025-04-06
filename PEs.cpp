#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <array>
#include <chrono>
#include <cstdint>
#include "interconnect.cpp"


constexpr int NUM_PES = 8;
constexpr int CACHE_LINES = 128;
constexpr int CACHE_LINE_SIZE = 16; // bytes

// Simulación simple de línea de cache
struct CacheLine {
    std::array<uint8_t, CACHE_LINE_SIZE> data{};
    uint32_t tag = 0;
    bool valid = false;
};


// Cache privada de cada PE
class Cache {
public:
    Cache() {
        lines.fill(CacheLine{});
    }

    // Escritura de datos en la cache
    void write(uint32_t addr, const std::vector<uint8_t>& data) {
        int index = (addr / CACHE_LINE_SIZE) % CACHE_LINES;
        lines[index].valid = true;
        lines[index].tag = addr / CACHE_LINE_SIZE;
        for (size_t i = 0; i < data.size() && i < CACHE_LINE_SIZE; ++i) {
            lines[index].data[i] = data[i];
        }
    }

    // Lectura de datos de la cache
    std::vector<uint8_t> read(uint32_t addr) {
        int index = (addr / CACHE_LINE_SIZE) % CACHE_LINES;
        if (lines[index].valid && lines[index].tag == addr / CACHE_LINE_SIZE) {
            return std::vector<uint8_t>(lines[index].data.begin(), lines[index].data.end());
        }
        return {}; // Miss
    }

private:
    std::array<CacheLine, CACHE_LINES> lines;
};

// PE con cache privada e instrucciones
class PE {
    public:
        PE(uint8_t id, Interconnect* ic) : pe_id(id), interconnect(ic) {}
    
        void run() {
            std::cout << "[PE " << int(pe_id) << "] Ejecutando...\n";
    
            Message writeMsg = {
                MessageType::WRITE_MEM,
                pe_id,        // SRC
                -1,           // DEST no aplica
                64,           // ADDR
                0,            // SIZE no aplica aquí
                0,            // CACHE_LINE
                0xABCD1234,   // DATA de prueba
                1,            // NUM_OF_CACHE_LINES
                0,            // START_CACHE_LINE
                0x10          // QoS
            };
    
            interconnect->enqueueMessage(writeMsg);
        }
    
    private:
        uint8_t pe_id;
        Interconnect* interconnect;
    };
    