#include "Node"
class event_q(){
    private:
    int MaxEvents; //numero maximo de eventos posibles en la cola de eventos
    //esto es para modelar un "tamaño del bus"
    int totalEvents; //eventos actuales en la cola.
    Node *head; //cabeza del nodo
    public:
        void addEvent(event *newevent);
        Node* popEvent();
        event_q(int maxevents);
};