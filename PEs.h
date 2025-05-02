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
PE(uint8_t id, Interconnect* ic, const std::string& instrFile);//recibe el interconect y el txt
    void run(); //ejecuta el pe
    void receiveMessage(const struct Message& msg); //recibe un struct que es el mensaje
    void invalidateCacheLine(int line); //invalida una linea de cache

private:
    uint8_t pe_id; //id del pe
    Interconnect* interconnect; //puntero al interconnect
    Cache cache; //cache privado
    std::string instruction_file;
};

#endif