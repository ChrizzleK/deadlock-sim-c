#include "src/policies/prevention_banker.h"
#include <string.h>

static int safety_check(const SystemState* s) {
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

      int ok = 1;
      for (int r=0;r<s->nres;r++) {
        int need = s->maxc[i][r] - s->alloc[i][r];
        if (need > work[r]) { ok = 0; break; }
      }
      if (ok) {
        for (int r=0;r<s->nres;r++) work[r] += s->alloc[i][r];
        finish[i] = 1;
        progress = 1;
      }
    }
  }

  for (int i=0;i<s->nprocs;i++) if (proc_active(s->procs[i].state) && !finish[i]) return 0;
  return 1;
}

static Decision on_request_impl(const Event* e, SystemState* s, Metrics* m) {
  int pi = -1, ri = -1;
  for (int i=0;i<s->nprocs;i++) if (s->procs[i].pid == e->pid) { pi=i; break; }
  for (int r=0;r<s->nres;r++) if (s->res[r].rid == e->rid) { ri=r; break; }
  if (pi<0 || ri<0) return DEC_DENY;

  int need = s->maxc[pi][ri] - s->alloc[pi][ri];
  if (e->amount > need) { m->denies++; return DEC_DENY; }
  if (e->amount > s->avail[ri]) { m->defers++; return DEC_DEFER; }

  s->avail[ri] -= e->amount;
  s->alloc[pi][ri] += e->amount;

  int safe = safety_check(s);
  if (!safe) {
    s->alloc[pi][ri] -= e->amount;
    s->avail[ri] += e->amount;
    m->denies++;
    return DEC_DENY;
  }

  m->grants++;
  return DEC_GRANT;
}

RequestPolicy policy_banker(void) {
  RequestPolicy p; p.on_request = on_request_impl; return p;
}
