#include "event.h"
class Node{
    private:
        Node *next;
        event *event;
    public:
        Node* getNext();
        event* getEvent();
        void setNext(Node *nextnode);
        Node(Event* e, Node* n = nullptr);
};