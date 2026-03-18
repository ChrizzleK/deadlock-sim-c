#include "src/core/workload.h"
#include "src/core/util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* event_type_name(EventType t) {
  switch (t) {
    case EV_ARRIVE: return "ARRIVE";
    case EV_REQ: return "REQ";
    case EV_REL: return "REL";
    case EV_FINISH: return "FINISH";
    case EV_CHECK: return "CHECK";
  }
  return "?";
}

int parse_event_type(const char* s, EventType* out) {
  if (str_eq(s,"ARRIVE")) { *out = EV_ARRIVE; return 1; }
  if (str_eq(s,"REQ")) { *out = EV_REQ; return 1; }
  if (str_eq(s,"REL")) { *out = EV_REL; return 1; }
  if (str_eq(s,"FINISH")) { *out = EV_FINISH; return 1; }
  if (str_eq(s,"CHECK")) { *out = EV_CHECK; return 1; }
  return 0;
}

EventList load_workload_csv(const char* path) {
  FILE* f = fopen(path, "r");
  if (!f) { fprintf(stderr, "cannot open workload %s\n", path); exit(1); }

  size_t cap = 256;
  Event* arr = (Event*)malloc(sizeof(Event) * cap);
  size_t len = 0;

  char line[512];
  int seq = 0;
  while (fgets(line, sizeof(line), f)) {
    if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
    line[strcspn(line, "\r\n")] = 0;

    char* cols[8];
    size_t n = split_csv(line, cols, 8);
    if (n < 6) { fprintf(stderr, "bad line\n"); exit(1); }

    EventType t;
    if (!parse_event_type(cols[1], &t)) { fprintf(stderr, "bad event type\n"); exit(1); }

    if (len == cap) { cap *= 2; arr = (Event*)realloc(arr, sizeof(Event)*cap); }

    arr[len].time = to_int(cols[0]);
    arr[len].type = t;
    arr[len].pid = to_int(cols[2]);
    arr[len].rid = to_int(cols[3]);
    arr[len].amount = to_int(cols[4]);
    arr[len].hold = to_int(cols[5]);
    arr[len].seq = seq++;
    len++;
  }

  fclose(f);
  EventList out = { arr, len };
  return out;
}
