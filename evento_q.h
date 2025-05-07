#include "Node.h"
class evento_q{
    private:
    int Maxeventos; //numero maximo de eventoos posibles en la cola de eventoos
    //esto es para modelar un "tamaño del bus"
    int totaleventos; //eventoos actuales en la cola.
    Node *head; //cabeza del nodo
    public:
        void addevento(evento *newevento);
        evento* popevento();
        evento_q(int maxeventos);
        Node* getHead();
};