# Deadlock Simulator (C)

## Projektübersicht
Im Rahmen dieses Projekts wurde eine Simulationsumgebung für **Deadlocks** entwickelt.  
Ziel war es, verschiedene Deadlock-Algorithmen aus der Vorlesung praktisch umzusetzen und miteinander vergleichen zu können.

Der Simulator basiert auf dem **Discrete-Event-Simulation (DES)** Ansatz und ist komplett in **C** implementiert.  
Der Fokus lag dabei weniger auf einer grafischen Oberfläche, sondern auf einer sauberen Kernfunktionalität und einer klaren Trennung zwischen Simulation und Algorithmen.

---

## Ziel des Projekts
Das Ziel des Projekts ist es,
- Deadlocks realistisch zu simulieren,
- verschiedene Strategien zur **Erkennung**, **Behebung**, **Vermeidung** und **Verhinderung** von Deadlocks umzusetzen,
- und deren Verhalten anhand von Metriken vergleichen zu können.

Der Simulator soll außerdem so aufgebaut sein, dass neue Algorithmen leicht ergänzt werden können.

---

## Simulationsansatz
Die Simulation ist ereignisbasiert. Das bedeutet, dass der Ablauf des Systems durch diskrete Ereignisse beschrieben wird, die zeitlich sortiert verarbeitet werden.

### Verwendete Ereignisse
- `ARRIVE` – ein Prozess tritt ins System ein
- `REQ` – ein Prozess fordert eine Ressource an
- `REL` – ein Prozess gibt eine Ressource frei
- `FINISH` – ein Prozess beendet sich
- `CHECK` – Deadlock-Überprüfung

Alle Ereignisse werden in einer Event-Queue verwaltet und der Reihe nach abgearbeitet.

---

## Modellierung
### Prozesse
- Jeder Prozess besitzt einen Zustand (`NEW`, `READY`, `BLOCKED`, `FINISHED`, `ABORTED`, `ROLLED_BACK`)
- Prozesse können mehrere Ressourcen gleichzeitig anfordern oder halten

### Ressourcen
- Es können mehrere Ressourcentypen existieren
- Ressourcen können eine oder mehrere Instanzen besitzen
- Die Verwaltung erfolgt über klassische Matrizen (Allocation, Request, Available)

---

## Implementierte Deadlock-Algorithmen

### Deadlock-Erkennung
- **Wait-For-Graph (WFG)**  
  Geeignet für Szenarien mit einer Instanz pro Ressource
- **Matrix-basierte Erkennung**  
  Für mehrere Instanzen pro Ressourcentyp

### Deadlock-Behebung
- **Prozessabbruch (Abort)**  
  Ein Prozess wird ausgewählt und beendet, um den Deadlock aufzulösen
- **Rollback**  
  Ein Prozess gibt alle Ressourcen frei und wird zurückgesetzt

### Deadlock-Verhinderung
- **Bankier-Algorithmus**  
  Sowohl für einfache als auch für mehrere Ressourcen umgesetzt  
  Jede Anforderung wird mit einem Safety-Check geprüft

### Deadlock-Vermeidung
- **No Hold & Wait**
- **Ressourcenordnung (Circular Wait vermeiden)**
- **Preemption**

Bedienung / Anwendung:

**Installation unter Ubuntu / WSL:**

sudo apt update
sudo apt install build-essential

**Im Hauptverzeichnis:**

make

**zum Ausführen:**

./deadlock_sim --workload workloads/demo.csv --mode detect_abort --detector matrix
hier kann "--workload workloads/demo.csv" geändert werden, die Datei enthält Ereignisse wie Prozessstart, Ressourcenanforderung und -freigabe.

**--mode <modus>** 
Legt fest, welche Deadlock-Strategie verwendet wird.

Verfügbare Modi:

detect_abort – Deadlock-Erkennung mit Prozessabbruch
detect_rollback – Deadlock-Erkennung mit Rollback
banker – Deadlock-Verhinderung mittels Bankier-Algorithmus
avoid_nohold – Vermeidung durch No Hold & Wait
avoid_order – Vermeidung durch feste Ressourcenreihenfolge
avoid_preempt – Vermeidung durch Preemption

z.B. --mode banker

Die Ausgabe sollte dann: deadlocks=1 aborts=1 rollbacks=0 grants=2 denies=0 defers=2
sein.
