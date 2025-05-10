#include "Node.h"
evento* Node:: getevento(){
    return this->mi_evento;
}

Node* Node:: getNext(){
    return this->next;
}
void Node::setNext(Node *nextnode){
    this->next = nextnode;
}
Node::Node(evento* e, Node* n) {
    this->mi_evento = e;
    this->next = n;
}