#include "policies/resolver_abort.h"

static int held_total(const SystemState* s, int pi) {
  int sum = 0;
  for (int r=0;r<s->nres;r++) if (s->alloc[pi][r] > 0) sum += s->alloc[pi][r];
  return sum;
}

static void resolve_impl(const DeadlockSet* d, SystemState* s, int now, Metrics* m) {
  (void)now;
  if (d->len == 0) return;

  int victim_pi = -1;
  int best = -1;
  for (int k=0;k<d->len;k++) {
    int pid = d->pids[k];
    int pi = -1;
    for (int i=0;i<s->nprocs;i++) if (s->procs[i].pid == pid) { pi=i; break; }
    if (pi<0) continue;
    int h = held_total(s, pi);
    if (h > best) { best = h; victim_pi = pi; }
  }
  if (victim_pi < 0) return;

  s->procs[victim_pi].state = PS_ABORTED;
  for (int r=0;r<s->nres;r++) {
    s->avail[r] += s->alloc[victim_pi][r];
    s->alloc[victim_pi][r] = 0;
    s->req[victim_pi][r] = 0;
  }
  m->aborts++;
}

Resolver resolver_abort(void) {
  Resolver r; r.resolve = resolve_impl; return r;
}
