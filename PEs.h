#ifndef PE_H
#define PE_H

#include <cstdint>
#include <vector>
#include "cache.h"
#include "interconnect.h"

// Forward declaration
class Interconnect;

class PE {
public:
PE(uint8_t id, Interconnect* ic, const std::string& instrFile);
    void run();
    void receiveMessage(const struct Message& msg);
    void invalidateCacheLine(int line);

private:
    uint8_t pe_id;
    Interconnect* interconnect;
    Cache cache;
    std::string instruction_file;
};

#endif