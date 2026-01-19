#include "policies/avoidance_ordering.h"

static Decision on_request_impl(const Event* e, SystemState* s, Metrics* m) {
  int pi = -1, ri = -1;
  for (int i=0;i<s->nprocs;i++) if (s->procs[i].pid == e->pid) { pi=i; break; }
  for (int r=0;r<s->nres;r++) if (s->res[r].rid == e->rid) { ri=r; break; }
  if (pi<0 || ri<0) return DEC_DENY;

  int maxHeldRid = -1;
  for (int r=0;r<s->nres;r++) if (s->alloc[pi][r] > 0 && s->res[r].rid > maxHeldRid) maxHeldRid = s->res[r].rid;
  if (maxHeldRid != -1 && e->rid < maxHeldRid) { m->denies++; return DEC_DENY; }

  if (e->amount > s->avail[ri]) { m->defers++; return DEC_DEFER; }
  s->avail[ri] -= e->amount;
  s->alloc[pi][ri] += e->amount;
  m->grants++;
  return DEC_GRANT;
}

RequestPolicy policy_avoid_ordering(void) {
  RequestPolicy p; p.on_request = on_request_impl; return p;
}
