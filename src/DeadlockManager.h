#ifndef DEADLOCKMANAGER_H
#define DEADLOCKMANAGER_H

#include <vector>
#include <string>
#include <map>

class DeadlockManager {
public:
    DeadlockManager();

    // Prozesse und Ressourcen
    void addProcess(int processId);
    void addResource(int resourceId, int amount);

    // Ressourcen anfordern / warten
    bool requestResource(int processId, int resourceId);

    // Deadlock erkennen und auflösen
    bool detectDeadlock();
    void resolveDeadlock();

    // Systemzustand
    std::string getSystemState();

    // Optional: Warteschlangen für Deadlock-Simulation
    void addWaiting(int processId, int resourceId);

private:
    std::map<int, std::vector<int>> allocation;      // welche Ressourcen hält ein Prozess
    std::map<int, int> resources;                    // wie viele Ressourcen verfügbar
    std::map<int, std::vector<int>> waiting;         // welche Ressourcen werden angefordert
};

#endif