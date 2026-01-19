#include "policies/detector_wfg.h"
#include <string.h>

static int detect_impl(const SystemState* s, DeadlockSet* out, int max_sets) {
  int n = s->nprocs;
  int g[MAX_PROCS][MAX_PROCS];
  memset(g, 0, sizeof(g));

  for (int pi=0; pi<n; pi++) {
    if (!proc_active(s->procs[pi].state)) continue;
    for (int r=0; r<s->nres; r++) {
      if (s->req[pi][r] <= 0) continue;
      for (int qi=0; qi<n; qi++) {
        if (qi==pi) continue;
        if (!proc_active(s->procs[qi].state)) continue;
        if (s->alloc[qi][r] > 0) g[pi][qi] = 1;
      }
    }
  }

  int color[MAX_PROCS];
  int stack[MAX_PROCS];
  int sp = 0;
  memset(color, 0, sizeof(color));

  int found = 0;

  for (int start=0; start<n && found<max_sets; start++) {
    if (!proc_active(s->procs[start].state)) continue;
    if (color[start] != 0) continue;

    int iters[MAX_PROCS];
    memset(iters, 0, sizeof(iters));
    int parent[MAX_PROCS];
    for (int i=0;i<MAX_PROCS;i++) parent[i] = -1;

    int cur = start;
    while (1) {
      if (color[cur] == 0) {
        color[cur] = 1;
        stack[sp++] = cur;
      }

      int advanced = 0;
      for (int v=iters[cur]; v<n; v++) {
        iters[cur] = v+1;
        if (!g[cur][v]) continue;
        if (color[v] == 0) { parent[v] = cur; cur = v; advanced = 1; break; }
        if (color[v] == 1) {
          DeadlockSet d; d.len = 0;
          int k = sp-1;
          while (k>=0 && stack[k] != v) { k--; }
          if (k>=0) {
            for (int i=k;i<sp;i++) d.pids[d.len++] = s->procs[stack[i]].pid;
            out[found++] = d;
          }
        }
      }
      if (found>=max_sets) break;
      if (advanced) continue;

      if (sp == 0) break;
      int popped = stack[--sp];
      color[popped] = 2;
      if (sp == 0) break;
      cur = stack[sp-1];
    }
  }

  return found;
}

Detector detector_wfg(void) {
  Detector d; d.detect = detect_impl; return d;
}
