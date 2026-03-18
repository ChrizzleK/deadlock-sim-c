QT += core gui widgets

TEMPLATE = app
TARGET = deadlock_gui
CONFIG += c++11
QMAKE_CFLAGS += -std=c11

# Pfad zum C-Projekt (mit Bindestrichen!)
CPROJECT = $$PWD/..

# WICHTIG: Repo-Root als Include-Pfad, nicht src/
INCLUDEPATH += "$$CPROJECT"

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    "$$CPROJECT/src/core/util.c" \
    "$$CPROJECT/src/core/workload.c" \
    "$$CPROJECT/src/core/model.c" \
    "$$CPROJECT/src/core/metrics.c" \
    "$$CPROJECT/src/core/engine.c" \
    "$$CPROJECT/src/policies/detector_wfg.c" \
    "$$CPROJECT/src/policies/detector_matrix.c" \
    "$$CPROJECT/src/policies/resolver_abort.c" \
    "$$CPROJECT/src/policies/resolver_rollback.c" \
    "$$CPROJECT/src/policies/prevention_banker.c" \
    "$$CPROJECT/src/policies/avoidance_nohold.c" \
    "$$CPROJECT/src/policies/avoidance_ordering.c" \
    "$$CPROJECT/src/policies/avoidance_preempt.c"

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui
