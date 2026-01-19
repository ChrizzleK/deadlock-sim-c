#include "core/model.h"
#include <string.h>

void state_init(SystemState* s) {
  memset(s, 0, sizeof(*s));
  for (int i=0;i<MAX_PROCS;i++) s->pid_to_idx[i] = -1;
  for (int i=0;i<MAX_RES;i++) s->rid_to_idx[i] = -1;
}

int proc_active(ProcState st) {
  return st != PS_FINISHED && st != PS_ABORTED;
}

int ensure_proc(SystemState* s, int pid) {
  for (int i=0;i<s->nprocs;i++) if (s->procs[i].pid == pid) return i;
  int idx = s->nprocs++;
  s->procs[idx].pid = pid;
  s->procs[idx].state = PS_NEW;
  s->procs[idx].priority = 0;
  s->procs[idx].progress = 0;
  return idx;
}

int ensure_res(SystemState* s, int rid, int capacity) {
  for (int i=0;i<s->nres;i++) if (s->res[i].rid == rid) return i;
  int idx = s->nres++;
  s->res[idx].rid = rid;
  s->res[idx].capacity = capacity;
  s->avail[idx] = capacity;
  return idx;
}
