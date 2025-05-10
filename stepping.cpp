
#include "stepping.h"
std::atomic<bool> stepping_enabled(false);
bool stepping_enabled_flag = false;

void setStepping(bool valor) {
    stepping_enabled_flag = valor;
}

void setStepping_enabled(bool valor) {
    stepping_flag =valor;
}