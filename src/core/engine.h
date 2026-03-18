#pragma once
#include "src/core/model.h"
#include "src/core/workload.h"
#include "src/core/metrics.h"
#include "src/policies/policy.h"

typedef struct {
  SystemState s;
  Metrics m;
  Detector det;
  Resolver res;
  RequestPolicy pol;

  Event* q;
  int qlen;
  int qcap;

  int check_interval;
} Engine;

void engine_init(Engine* e);
void engine_set_detector(Engine* e, Detector d);
void engine_set_resolver(Engine* e, Resolver r);
void engine_set_policy(Engine* e, RequestPolicy p);

void engine_add_event(Engine* e, Event ev);
void engine_load_events(Engine* e, const EventList* list);

void engine_run(Engine* e, int until_time);
