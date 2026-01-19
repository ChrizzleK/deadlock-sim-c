#include "core/engine.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static int cmp_ev(const Event* a, const Event* b) {
  if (a->time != b->time) return (a->time < b->time) ? -1 : 1;
  return (a->seq < b->seq) ? -1 : (a->seq > b->seq);
}

void engine_init(Engine* e) {
  memset(e, 0, sizeof(*e));
  state_init(&e->s);
  e->qcap = 256;
  e->q = (Event*)malloc(sizeof(Event)*e->qcap);
  e->check_interval = 1;
  ensure_res(&e->s, 0, 1);
  ensure_res(&e->s, 1, 1);
}

void engine_set_detector(Engine* e, Detector d) { e->det = d; }
void engine_set_resolver(Engine* e, Resolver r) { e->res = r; }
void engine_set_policy(Engine* e, RequestPolicy p) { e->pol = p; }

void engine_add_event(Engine* e, Event ev) {
  if (e->qlen == e->qcap) {
    e->qcap *= 2;
    e->q = (Event*)realloc(e->q, sizeof(Event)*e->qcap);
  }
  e->q[e->qlen++] = ev;
}

void engine_load_events(Engine* e, const EventList* list) {
  for (size_t i=0;i<list->len;i++) engine_add_event(e, list->items[i]);
}

static void sort_queue(Engine* e) {
  for (int i=0;i<e->qlen;i++) {
    for (int j=i+1;j<e->qlen;j++) {
      if (cmp_ev(&e->q[j], &e->q[i]) < 0) {
        Event tmp = e->q[i];
        e->q[i] = e->q[j];
        e->q[j] = tmp;
      }
    }
  }
}

static void do_deadlock_check(Engine* e, int now) {
  DeadlockSet sets[8];
  int n = 0;
  if (e->det.detect) n = e->det.detect(&e->s, sets, 8);
  if (n > 0) e->m.deadlocks_detected += n;
  for (int i=0;i<n;i++) if (e->res.resolve) e->res.resolve(&sets[i], &e->s, now, &e->m);
}

static void handle_arrive(Engine* e, const Event* ev) {
  int pi = ensure_proc(&e->s, ev->pid);
  e->s.procs[pi].state = PS_READY;
  for (int r=0;r<e->s.nres;r++) {
    if (e->s.maxc[pi][r] == 0) e->s.maxc[pi][r] = e->s.res[r].capacity;
  }
}

static void handle_finish(Engine* e, const Event* ev) {
  for (int i=0;i<e->s.nprocs;i++) {
    if (e->s.procs[i].pid == ev->pid) {
      e->s.procs[i].state = PS_FINISHED;
      for (int r=0;r<e->s.nres;r++) {
        e->s.avail[r] += e->s.alloc[i][r];
        e->s.alloc[i][r] = 0;
        e->s.req[i][r] = 0;
      }
      break;
    }
  }
}

static void handle_rel(Engine* e, const Event* ev) {
  int pi=-1, ri=-1;
  for (int i=0;i<e->s.nprocs;i++) if (e->s.procs[i].pid == ev->pid) { pi=i; break; }
  for (int r=0;r<e->s.nres;r++) if (e->s.res[r].rid == ev->rid) { ri=r; break; }
  if (pi<0 || ri<0) return;
  int k = ev->amount;
  if (k > e->s.alloc[pi][ri]) k = e->s.alloc[pi][ri];
  e->s.alloc[pi][ri] -= k;
  e->s.avail[ri] += k;
}

static void handle_req(Engine* e, const Event* ev) {
  int pi=-1, ri=-1;
  for (int i=0;i<e->s.nprocs;i++) if (e->s.procs[i].pid == ev->pid) { pi=i; break; }
  for (int r=0;r<e->s.nres;r++) if (e->s.res[r].rid == ev->rid) { ri=r; break; }
  if (pi<0 || ri<0) return;
  if (!proc_active(e->s.procs[pi].state)) return;

  Decision d = DEC_DEFER;
  if (e->pol.on_request) d = e->pol.on_request(ev, &e->s, &e->m);
  else {
    if (ev->amount <= e->s.avail[ri]) {
      e->s.avail[ri] -= ev->amount;
      e->s.alloc[pi][ri] += ev->amount;
      e->m.grants++;
      d = DEC_GRANT;
    } else {
      e->m.defers++;
      d = DEC_DEFER;
    }
  }

  if (d == DEC_DEFER) {
    e->s.req[pi][ri] += ev->amount;
    e->s.procs[pi].state = PS_BLOCKED;
  } else if (d == DEC_DENY) {
    e->s.procs[pi].state = PS_BLOCKED;
  } else {
    e->s.procs[pi].state = PS_READY;
    if (ev->hold > 0) {
      Event rel = *ev;
      rel.type = EV_REL;
      rel.time = ev->time + ev->hold;
      rel.hold = 0;
      engine_add_event(e, rel);
    }
  }
}

static void try_unblock(Engine* e) {
  for (int pi=0; pi<e->s.nprocs; pi++) {
    if (e->s.procs[pi].state != PS_BLOCKED) continue;
    int unblocked = 1;
    for (int ri=0; ri<e->s.nres; ri++) {
      int need = e->s.req[pi][ri];
      if (need <= 0) continue;
      if (need > e->s.avail[ri]) { unblocked = 0; break; }
    }
    if (!unblocked) continue;
    for (int ri=0; ri<e->s.nres; ri++) {
      int need = e->s.req[pi][ri];
      if (need <= 0) continue;
      e->s.avail[ri] -= need;
      e->s.alloc[pi][ri] += need;
      e->s.req[pi][ri] = 0;
    }
    e->s.procs[pi].state = PS_READY;
  }
}

void engine_run(Engine* e, int until_time) {
  sort_queue(e);

  int idx = 0;
  while (idx < e->qlen) {
    Event ev = e->q[idx++];
    if (ev.time > until_time) break;
    e->s.time = ev.time;

    if (ev.type == EV_ARRIVE) handle_arrive(e, &ev);
    else if (ev.type == EV_REQ) handle_req(e, &ev);
    else if (ev.type == EV_REL) handle_rel(e, &ev);
    else if (ev.type == EV_FINISH) handle_finish(e, &ev);
    else if (ev.type == EV_CHECK) do_deadlock_check(e, ev.time);

    if (e->check_interval > 0) do_deadlock_check(e, ev.time);
    try_unblock(e);

    if (idx < e->qlen) {
      for (int i=idx;i<e->qlen;i++) {
        for (int j=i+1;j<e->qlen;j++) {
          if (cmp_ev(&e->q[j], &e->q[i]) < 0) {
            Event tmp = e->q[i]; e->q[i] = e->q[j]; e->q[j] = tmp;
          }
        }
      }
    }
  }
}
