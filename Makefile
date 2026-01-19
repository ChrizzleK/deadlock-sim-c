CC=gcc
CFLAGS=-std=c11 -O2 -Wall -Wextra -Isrc

SRC= \
  src/main.c \
  src/core/util.c \
  src/core/workload.c \
  src/core/model.c \
  src/core/metrics.c \
  src/core/engine.c \
  src/policies/detector_wfg.c \
  src/policies/detector_matrix.c \
  src/policies/resolver_abort.c \
  src/policies/resolver_rollback.c \
  src/policies/prevention_banker.c \
  src/policies/avoidance_nohold.c \
  src/policies/avoidance_ordering.c \
  src/policies/avoidance_preempt.c

OUT=deadlock_sim

all:
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

clean:
	rm -f $(OUT)
