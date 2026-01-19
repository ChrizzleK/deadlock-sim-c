#pragma once
#include <stddef.h>

#define MAX_PROCS 128
#define MAX_RES 64

typedef enum { PS_NEW, PS_READY, PS_RUNNING, PS_BLOCKED, PS_FINISHED, PS_ABORTED, PS_ROLLED_BACK } ProcState;

typedef struct {
  int pid;
  ProcState state;
  int priority;
  int progress;
} Process;

typedef struct {
  int rid;
  int capacity;
} Resource;

typedef struct {
  int time;
  int nprocs;
  int nres;

  Process procs[MAX_PROCS];
  Resource res[MAX_RES];

  int alloc[MAX_PROCS][MAX_RES];
  int req[MAX_PROCS][MAX_RES];
  int maxc[MAX_PROCS][MAX_RES];
  int avail[MAX_RES];

  int pid_to_idx[MAX_PROCS];
  int rid_to_idx[MAX_RES];
} SystemState;

void state_init(SystemState* s);
int ensure_proc(SystemState* s, int pid);
int ensure_res(SystemState* s, int rid, int capacity);
int proc_active(ProcState st);
