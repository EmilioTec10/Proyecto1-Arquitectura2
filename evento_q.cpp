#include "evento_q.h"
void evento_q:: addevento(evento *newevento){ //esto debe ser un bool
    //si es 0, es que no se pudo insertar, bus saturado ,
    //si es 1 se inserto correctamente
    if(this->head != nullptr){
        Node *newnode = new Node(newevento);
        newnode->setNext(this->head);
        this->head=newnode;
        
    }
    else{
        if(this->totaleventos!=this->Maxeventos){
            Node *newnode = new Node(newevento);
            this->head=newnode;
            
        }
        else{
            printf("bus sobrecargado de datos");
            Node *newnode = new Node(newevento);
            this->head=newnode;
        }
    }
    
};

evento* evento_q:: popevento(){
    if (this->head == nullptr) {
        return nullptr;
    }
    else {
        Node* currentTop = this->head;
        evento* ev = currentTop->getevento();  // Obtén el eventoo
        this->head = this->head->getNext();  // Avanza la cabeza de la lista
        //delete currentTop;  // Libera el nodo
        this->totaleventos-=1;

        return ev;          // Devuelve el eventoo (ya no está contenido en el nodo) 
    }
};
/*
evento* evento_q::popevento() {
    if (this->head == nullptr) return nullptr;

    Node* currentTop = this->head;

    evento* ev_copy = new evento(*currentTop->getevento()); // COPIA INDEPENDIENTE

    this->head = this->head->getNext();
    delete currentTop;
    this->totaleventos -= 1;

    return ev_copy;
}*/





evento_q::evento_q(int maxeventos) {
    this->Maxeventos = maxeventos;
}


Node* evento_q::getHead() {
    return this->head;
}