#include "policies/detector_matrix.h"
#include <string.h>

static int leq_need(const int* need, const int* work, int nres) {
  for (int r=0;r<nres;r++) if (need[r] > work[r]) return 0;
  return 1;
}

static int detect_impl(const SystemState* s, DeadlockSet* out, int max_sets) {
  (void)max_sets;
  int work[MAX_RES];
  int finish[MAX_PROCS];
  memcpy(work, s->avail, sizeof(int)*s->nres);
  memset(finish, 0, sizeof(finish));

  int progress = 1;
  while (progress) {
    progress = 0;
    for (int i=0;i<s->nprocs;i++) {
      if (!proc_active(s->procs[i].state)) { finish[i]=1; continue; }
      if (finish[i]) continue;
      if (leq_need(s->req[i], work, s->nres)) {
        for (int r=0;r<s->nres;r++) work[r] += s->alloc[i][r];
        finish[i] = 1;
        progress = 1;
      }
    }
  }

  DeadlockSet d; d.len = 0;
  for (int i=0;i<s->nprocs;i++) {
    if (!proc_active(s->procs[i].state)) continue;
    if (!finish[i]) d.pids[d.len++] = s->procs[i].pid;
  }
  if (d.len == 0) return 0;
  out[0] = d;
  return 1;
}

Detector detector_matrix(void) {
  Detector d; d.detect = detect_impl; return d;
}
