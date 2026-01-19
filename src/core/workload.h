#pragma once
#include <stddef.h>

typedef enum { EV_ARRIVE, EV_REQ, EV_REL, EV_FINISH, EV_CHECK } EventType;

typedef struct {
  int time;
  EventType type;
  int pid;
  int rid;
  int amount;
  int hold;
  int seq;
} Event;

typedef struct {
  Event* items;
  size_t len;
} EventList;

EventList load_workload_csv(const char* path);
const char* event_type_name(EventType t);
int parse_event_type(const char* s, EventType* out);
