#ifndef event
#define event
#include <string>
#include <iostream>
#include <vector>
class event{ //voy a tener 3 tipos de evento, read message, execute, memoria
//  y una cola de eventos de sucesos , yo creo eventos cada que calendarizo, y si existe competencia
//agrego el pe_id a la lista de pe_id
    //aca debe de haber un array de peid para el pe asociado.
    public:
        event(const std::string& nombre_) :event_name(nombre_) {} //constructor
        void insert_peid(int pe_id); //insertar pe a la lista
        void pop_peid(); //pop del pe a la lista
        //int get_current_State();
        void mostrarNombre();
        void addBytes(int number);
        void removeBytes(int number);
    private:
        td::vector<int> pe_ids; //id de los vectores en el proceso asociado.
        std::string event_name = ""; //nombre del evento.
        //analizar si incluir bytes totales del proceso a la lista, como una lista de bytes
        //siendo procesador por el bus.
        //necesito una busqueda por nombre
        int total_bytes_event;
        //son diferenciados ,
        //crear cola de eventos con un tope de eventos.es como mi buffer
        
};
#endif