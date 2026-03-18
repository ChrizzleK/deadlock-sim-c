#include "DeadlockManager.h"
#include <iostream>
#include <stack>
#include <set>
#include <functional>

DeadlockManager::DeadlockManager() {}

// Prozesse hinzufügen
void DeadlockManager::addProcess(int processId) {
    allocation[processId] = {};
    waiting[processId] = {};
}

// Ressourcen hinzufügen
void DeadlockManager::addResource(int resourceId, int amount) {
    resources[resourceId] = amount;
}

// Ressource anfordern
bool DeadlockManager::requestResource(int processId, int resourceId) {
    if (resources[resourceId] > 0) {
        resources[resourceId]--;
        allocation[processId].push_back(resourceId);
        return true;
    } else {
        waiting[processId].push_back(resourceId);
        return false;
    }
}

// Warteschlange für Deadlock-Test
void DeadlockManager::addWaiting(int processId, int resourceId) {
    waiting[processId].push_back(resourceId);
}

// Deadlock erkennen
bool DeadlockManager::detectDeadlock() {
    // Wait-For-Graph bauen
    std::map<int, std::vector<int>> waitForGraph;
    for (auto &p : waiting) {
        int pid = p.first;
        for (int rid : p.second) {
            for (auto &q : allocation) {
                int otherPid = q.first;
                for (int held : q.second) {
                    if (held == rid && pid != otherPid) {
                        waitForGraph[pid].push_back(otherPid);
                    }
                }
            }
        }
    }

    // Zyklus erkennen (DFS)
    std::set<int> visited;
    std::set<int> recStack;

    std::function<bool(int)> dfs = [&](int p) -> bool {
        if (visited.find(p) == visited.end()) {
            visited.insert(p);
            recStack.insert(p);
            for (int neighbor : waitForGraph[p]) {
                if (recStack.find(neighbor) != recStack.end()) return true;
                if (dfs(neighbor)) return true;
            }
        }
        recStack.erase(p);
        return false;
    };

    for (auto &p : allocation) {
        if (dfs(p.first)) return true;
    }
    return false;
}

// Deadlock auflösen
void DeadlockManager::resolveDeadlock() {
    std::cout << "⚠ Deadlock wird aufgeloest: Prozess 1 wird beendet.\n";

    // Prozess 1 Ressourcen freigeben
    for (int r : allocation[1]) {
        resources[r]++;
    }
    allocation[1].clear();
    waiting[1].clear();

    // Prüfen, ob andere Prozesse jetzt Ressourcen bekommen können
    for (auto &p : waiting) {
        auto it = p.second.begin();
        while (it != p.second.end()) {
            int resId = *it;
            if (resources[resId] > 0) {
                resources[resId]--;
                allocation[p.first].push_back(resId);
                it = p.second.erase(it);
            } else {
                ++it;
            }
        }
    }
}

// Systemzustand als Text
std::string DeadlockManager::getSystemState() {
    std::string state = "--- Systemzustand ---\n";
    for (auto &p : allocation) {
        state += "Prozess " + std::to_string(p.first) + " hält: ";
        for (int r : p.second) {
            state += "R" + std::to_string(r) + " ";
        }
        if (!waiting[p.first].empty()) {
            state += "| Wartet auf: ";
            for (int w : waiting[p.first]) state += "R" + std::to_string(w) + " ";
        }
        state += "\n";
    }
    return state;
}