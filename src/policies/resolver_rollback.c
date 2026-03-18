#include "src/policies/resolver_rollback.h"

static void resolve_impl(const DeadlockSet* d, SystemState* s, int now, Metrics* m) {
  (void)now;
  if (d->len == 0) return;

  int pid = d->pids[0];
  int pi = -1;
  for (int i=0;i<s->nprocs;i++) if (s->procs[i].pid == pid) { pi=i; break; }
  if (pi<0) return;

  s->procs[pi].state = PS_ROLLED_BACK;
  for (int r=0;r<s->nres;r++) {
    if (s->alloc[pi][r] > 0) {
      s->avail[r] += s->alloc[pi][r];
      s->alloc[pi][r] = 0;
    }
    s->req[pi][r] = 0;
  }
  m->rollbacks++;
}

Resolver resolver_rollback(void) {
  Resolver r; r.resolve = resolve_impl; return r;
}
