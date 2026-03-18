#include <iostream>
#include "DeadlockManager.h"

int main() {
    DeadlockManager manager;

    // Prozesse und Ressourcen
    manager.addProcess(1);
    manager.addProcess(2);

    manager.addResource(1, 1);
    manager.addResource(2, 1);

    // Deadlock erzeugen
    manager.requestResource(1, 1); // P1 hält R1
    manager.requestResource(2, 2); // P2 hält R2
    manager.addWaiting(1, 2);       // P1 wartet auf R2
    manager.addWaiting(2, 1);       // P2 wartet auf R1

    std::cout << manager.getSystemState();

    if(manager.detectDeadlock()) {
        std::cout << "❌ Deadlock erkannt!\n";
        manager.resolveDeadlock();
    }

    std::cout << manager.getSystemState();

    return 0;
}