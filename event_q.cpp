#include "event_q.h"
void event_q:: addEvent(event *newevent){ //esto debe ser un bool
    //si es 0, es que no se pudo insertar, bus saturado ,
    //si es 1 se inserto correctamente
    if(this->head != nullptr){
        Node *newnode = new Node(newevent);
        newevent->setNext(this->head);
        this->head=newnode;
        
    }
    else{
        if(this->totalEvents!=this->maxEvents){
            Node *newnode = new Node(newevent);
            this->head=newnode;
            
        }
        else{
            print("bus sobrecargado de datos");
            Node *newnode = new Node(newevent);
            this->head=newnode;
        }
    }
    
};

Node* event_q:: popEvent(){
    if (this->head == nullptr) {
        return nullptr;
    }
    Node* currentTop = this->head;
    event* ev = currentTop->getEvent();  // Obtén el evento
    this->head = this->head->getNext();  // Avanza la cabeza de la lista
    delete currentTop;  // Libera el nodo
    this->totalEvents-=1

    return ev;          // Devuelve el evento (ya no está contenido en el nodo)

};
event_q::event_q(int maxEvents) {
    this->MaxEvents = maxEvents;
}
event* event::getHead(){
    return this->head->getEvent();
}