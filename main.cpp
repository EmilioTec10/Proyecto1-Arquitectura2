// main.cpp
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <array>
#include <chrono>
#include <cstdint>
#include <queue>
#include <condition_variable>
#include <map>

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

    void write(uint32_t addr, const std::vector<uint8_t>& data) {
        int index = (addr / CACHE_LINE_SIZE) % CACHE_LINES;
        lines[index].valid = true;
        lines[index].tag = addr / CACHE_LINE_SIZE;
        for (size_t i = 0; i < data.size() && i < CACHE_LINE_SIZE; ++i) {
            lines[index].data[i] = data[i];
        }
    }

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
    PE(uint8_t id) : pe_id(id) {}

    void run() {
        std::cout << "[PE " << int(pe_id) << "] Empezando ejecución...\n";

        for (int i = 0; i < 10; ++i) {
            uint32_t addr = i * 16;
            std::vector<uint8_t> data(CACHE_LINE_SIZE, pe_id);
            cache.write(addr, data);

            auto read_data = cache.read(addr);
            std::cout << "[PE " << int(pe_id) << "] Read data size: " << read_data.size() << "\n";
        }

        std::cout << "[PE " << int(pe_id) << "] Finalizó ejecución.\n";
    }

private:
    uint8_t pe_id;
    Cache cache;
};



int main() {
    std::vector<std::thread> threads;
    std::vector<PE> pes;

    for (int i = 0; i < NUM_PES; ++i) {
        pes.emplace_back(i);
    }

    for (int i = 0; i < NUM_PES; ++i) {
        threads.emplace_back(&PE::run, &pes[i]);
    }

    for (auto& t : threads) {
        t.join();
    }

    std::cout << "Simulación completada.\n";
    return 0;
}
