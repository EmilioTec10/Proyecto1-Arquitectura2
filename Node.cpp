#include "Node.h"
event* Node:: getEvent(){
    return this->event;
}

Node* Node:: getNext(){
    return this->next;
}
void Node::setNext(Node *nextnode){
    this->next = nextnode;
}
Node::Node(Event* e, Node* n) {
    this->event = e;
    this->next = n;
}