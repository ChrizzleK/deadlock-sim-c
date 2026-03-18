TEMPLATE = app
CONFIG += console c11
CONFIG -= app_bundle
CONFIG -= qt

# Includes für deine Unterordner
INCLUDEPATH += src/core src/policies

SOURCES += \
    src/main.c \
    src/core/engine.c \
    src/core/metrics.c \
    src/core/model.c \
    src/core/util.c \
    src/core/workload.c \
    src/policies/avoidance_nohold.c \
    src/policies/avoidance_ordering.c \
    src/policies/avoidance_preempt.c \
    src/policies/detector_matrix.c \
    src/policies/detector_wfg.c \
    src/policies/prevention_banker.c \
    src/policies/resolver_abort.c \
    src/policies/resolver_rollback.c

HEADERS += \
    src/core/engine.h \
    src/core/metrics.h \
    src/core/model.h \
    src/core/util.h \
    src/core/workload.h \
    src/policies/avoidance_nohold.h \
    src/policies/avoidance_ordering.h \
    src/policies/avoidance_preempt.h \
    src/policies/detector_matrix.h \
    src/policies/detector_wfg.h \
    src/policies/policy.h \
    src/policies/prevention_banker.h \
    src/policies/resolver_abort.h \
    src/policies/resolver_rollback.h
