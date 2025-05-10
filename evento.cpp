#include "evento.h"
#include "interconnect.h" 

void evento::insert_peid(int pe_id){
    this->pe_id = pe_id; //se inserta en la cola el peid

};
void evento::addBytes(int bytes){//esto se debe de hacer mas elegante a futuro
    //que no muestre los repetidos
    this->total_bytes_evento += bytes;
}
void evento::removeBytes(int bytes){//esto se debe de hacer mas elegante a futuro
    //que no muestre los repetidos
    this->total_bytes_evento -= bytes;
    if(total_bytes_evento<0){
        this->total_bytes_evento=0;
    }
}
evento::evento(const std::string& nombre, int pe_id_, int totalbytes, MessageType tipo) {
    this->evento_name = nombre;
    this->pe_id = pe_id_;
    this->total_bytes_evento = totalbytes;
    this->tipo_instruccion = tipo;
}

int evento::getpe_id(){
    return this->pe_id;
}
int evento::getBytes(){
    return this->total_bytes_evento;
}
std::string evento::get_evento_name() {
    return this->evento_name;
}

MessageType evento::getTipoInstruccion() {
    return this->tipo_instruccion;
}
