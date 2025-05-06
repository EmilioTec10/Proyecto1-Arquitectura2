#include "event.h"

void event::insert_peid(int pe_id){
    this->pe_ids = pe_id; //se inserta en la cola el peid

};
void event::addBytes(int bytes){//esto se debe de hacer mas elegante a futuro
    //que no muestre los repetidos
    this->total_bytes_event += bytes
}
void event::removeBytes(int bytes){//esto se debe de hacer mas elegante a futuro
    //que no muestre los repetidos
    this->total_bytes_event -= bytes
    if(total_bytes_event<0){
        this->total_bytes_event=0
    }
}
event::event(const std::string& nombre, int pe_id_,int totalbytes){
    this->event_name=nombre;
    this->pe_id = pe_id_;
    this->total_bytes_event = totalbytes;
}

