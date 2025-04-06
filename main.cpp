// main.cpp
#include <iostream>
#include <vector>
#include <thread>
#include "PEs.cpp" 



int main() {
    std::vector<std::thread> threads;
    std::vector<PE> pes;

    for (int i = 0; i < NUM_PES; ++i) {
        pes.emplace_back(i);
    }

    for (int i = 0; i < NUM_PES; ++i) {
        threads.emplace_back(&PE::run, &pes[i]);
    }

    for (auto& t : threads) {
        t.join();
    }

    std::cout << "Simulación completada.\n";
    return 0;
}
