#ifndef EVENTO_H
#define EVENTO_H
#include <string>
#include <iostream>
#include <vector>
enum class MessageType;
class evento{ //voy a tener 3 tipos de eventoo, read message, execute, memoria
//  y una cola de eventoos de sucesos , yo creo eventoos cada que calendarizo, y si existe competencia
//agrego el pe_id a la lista de pe_id
    //aca debe de haber un array de peid para el pe asociado.
    public:
    
    evento(const std::string& nombre_, int peid, int totalbytes, MessageType tipo);
    //constructor
        
        void insert_peid(int pe_id); //insertar pe a la lista
        void addBytes(int number);
        void removeBytes(int number);
        int getpe_id();
        int getBytes();
        std::string get_evento_name();
        MessageType getTipoInstruccion();
    private:
        int pe_id; //id de los vectores en el proceso asociado.
        std::string evento_name = ""; //nombre del eventoo.
        //analizar si incluir bytes totales del proceso a la lista, como una lista de bytes
        //siendo procesador por el bus.
        //necesito una busqueda por nombre
        int total_bytes_evento; //cambiar por unsigned bytes.
        //son diferenciados ,
        //crear cola de eventoos con un tope de eventoos.es como mi buffer
        evento *mi_evento;
        MessageType tipo_instruccion; // para guarardar el tipo de mensaje asociado

        
};
#endif
