#ifndef CACHE_H
#define CACHE_H

#include <array>
#include <vector>
#include <cstdint>

constexpr int CACHE_LINES = 128;
constexpr int CACHE_LINE_SIZE = 16; // bytes

struct CacheLine {
    std::array<uint8_t, CACHE_LINE_SIZE> data{};
    uint32_t tag = 0;
    bool valid = false;
};

class Cache {
public:
    Cache();

    void write(uint32_t addr, const std::vector<uint8_t>& data);
    std::vector<uint8_t> read(uint32_t addr) const;
    void invalidateLine(int line);

private:
    std::array<CacheLine, CACHE_LINES> lines;
};

#endif
