#include "evento.h"
class Node{
    private:
        Node *next;
        evento *mi_evento;
    public:
        Node* getNext();
        evento* getevento();
        void setNext(Node *nextnode);
        Node(evento* e, Node* n = nullptr);
};