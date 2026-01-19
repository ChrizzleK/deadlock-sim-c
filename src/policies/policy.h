#pragma once
#include "core/model.h"
#include "core/workload.h"
#include "core/metrics.h"

typedef enum { DEC_GRANT, DEC_DENY, DEC_DEFER } Decision;

typedef struct {
  int pids[MAX_PROCS];
  int len;
} DeadlockSet;

typedef struct {
  int (*detect)(const SystemState* s, DeadlockSet* out_sets, int max_sets);
} Detector;

typedef struct {
  void (*resolve)(const DeadlockSet* d, SystemState* s, int now, Metrics* m);
} Resolver;

typedef struct {
  Decision (*on_request)(const Event* e, SystemState* s, Metrics* m);
} RequestPolicy;
