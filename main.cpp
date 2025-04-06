// main.cpp
#include <iostream>
#include <vector>
#include <thread>
#include "PEs.cpp" 



int main() {
    // Crear el Interconnect con política FIFO (false) o QoS (true)
    Interconnect ic(true); // usamos QoS para ver que funcione

    // Lanzar el hilo de procesamiento de mensajes
    std::thread ic_thread(&Interconnect::processMessages, &ic);

    // Crear 1 PE (podés crear los 8 luego)
    PE pe0(0, &ic);

    // Ejecutar el PE (esto manda un mensaje WRITE_MEM al Interconnect)
    pe0.run();

    // Esperamos un poco para ver el resultado del mensaje
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Como no tenemos forma de detener el hilo, detenemos el programa aquí a la fuerza
    std::exit(0);
}
