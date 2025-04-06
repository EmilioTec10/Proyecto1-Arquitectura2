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
    PE(uint8_t id, Interconnect* ic);
    void run();
    void receiveMessage(const struct Message& msg);

private:
    uint8_t pe_id;
    Interconnect* interconnect;
    Cache cache;
};

#endif