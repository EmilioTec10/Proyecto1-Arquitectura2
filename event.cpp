#include "event.h"

void event::insert_peid(int pe_id){
    this->pe_ids.push_back(pe_id); //se inserta en la cola el peid

};
void event::pop_peid(){
    for (size_t i = 0; i < this->pe_id.size(); ++i) {
        this->pe_ids.pop_back();
    }
    this->pe_ids.pop_back();
}void event::mostrarNombre(){//esto se debe de hacer mas elegante a futuro
    //que no muestre los repetidos
    for (size_t i = 0; i < this->pe_id.size(); ++i) {
        std::cout << this->pe_id[i] << std::endl;
    }
}
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

