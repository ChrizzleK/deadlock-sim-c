#include "src/policies/avoidance_nohold.h"

static Decision on_request_impl(const Event* e, SystemState* s, Metrics* m) {
  int pi = -1, ri = -1;
  for (int i=0;i<s->nprocs;i++) if (s->procs[i].pid == e->pid) { pi=i; break; }
  for (int r=0;r<s->nres;r++) if (s->res[r].rid == e->rid) { ri=r; break; }
  if (pi<0 || ri<0) return DEC_DENY;

  int holding = 0;
  for (int r=0;r<s->nres;r++) if (s->alloc[pi][r] > 0) holding = 1;

  if (holding) {
    m->defers++;
    return DEC_DEFER;
  }

  if (e->amount > s->avail[ri]) { m->defers++; return DEC_DEFER; }
  s->avail[ri] -= e->amount;
  s->alloc[pi][ri] += e->amount;
  m->grants++;
  return DEC_GRANT;
}

RequestPolicy policy_avoid_nohold(void) {
  RequestPolicy p; p.on_request = on_request_impl; return p;
}
