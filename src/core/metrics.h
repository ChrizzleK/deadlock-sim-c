#pragma once
#include <stddef.h>

typedef struct {
  int deadlocks_detected;
  int aborts;
  int rollbacks;
  int grants;
  int denies;
  int defers;
} Metrics;
