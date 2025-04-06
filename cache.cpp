#include <iostream>
#include "cache.h"

Cache::Cache() {
    lines.fill(CacheLine{});
}

void Cache::write(uint32_t addr, const std::vector<uint8_t>& data) {
    int index = (addr / CACHE_LINE_SIZE) % CACHE_LINES;
    lines[index].valid = true;
    lines[index].tag = addr / CACHE_LINE_SIZE;

    for (size_t i = 0; i < data.size() && i < CACHE_LINE_SIZE; ++i) {
        lines[index].data[i] = data[i];
    }
}

std::vector<uint8_t> Cache::read(uint32_t addr) const {
    int index = (addr / CACHE_LINE_SIZE) % CACHE_LINES;

    if (lines[index].valid && lines[index].tag == addr / CACHE_LINE_SIZE) {
        return std::vector<uint8_t>(lines[index].data.begin(), lines[index].data.end());
    }

    return {}; // Miss
}
