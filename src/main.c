#include "core/engine.h"
#include "core/workload.h"
#include "policies/detector_wfg.h"
#include "policies/detector_matrix.h"
#include "policies/resolver_abort.h"
#include "policies/resolver_rollback.h"
#include "policies/prevention_banker.h"
#include "policies/avoidance_nohold.h"
#include "policies/avoidance_ordering.h"
#include "policies/avoidance_preempt.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static const char* argval(int* i, int argc, char** argv) {
  if (*i + 1 >= argc) { fprintf(stderr,"missing value\n"); exit(1); }
  (*i)++;
  return argv[*i];
}

int main(int argc, char** argv) {
  const char* workload = "workloads/demo.csv";
  const char* mode = "detect_abort";
  const char* detector = "matrix";

  for (int i=1;i<argc;i++) {
    if (!strcmp(argv[i],"--workload")) workload = argval(&i,argc,argv);
    else if (!strcmp(argv[i],"--mode")) mode = argval(&i,argc,argv);
    else if (!strcmp(argv[i],"--detector")) detector = argval(&i,argc,argv);
  }

  Engine e;
  engine_init(&e);

  if (!strcmp(detector,"wfg")) engine_set_detector(&e, detector_wfg());
  else engine_set_detector(&e, detector_matrix());

  if (!strcmp(mode,"detect_abort")) {
    engine_set_resolver(&e, resolver_abort());
  } else if (!strcmp(mode,"detect_rollback")) {
    engine_set_resolver(&e, resolver_rollback());
  } else if (!strcmp(mode,"banker")) {
    engine_set_policy(&e, policy_banker());
  } else if (!strcmp(mode,"avoid_nohold")) {
    engine_set_policy(&e, policy_avoid_nohold());
  } else if (!strcmp(mode,"avoid_order")) {
    engine_set_policy(&e, policy_avoid_ordering());
  } else if (!strcmp(mode,"avoid_preempt")) {
    engine_set_policy(&e, policy_avoid_preempt());
  } else {
    fprintf(stderr,"unknown mode\n");
    return 1;
  }

  EventList list = load_workload_csv(workload);
  engine_load_events(&e, &list);
  engine_run(&e, 100000);

  printf("deadlocks=%d aborts=%d rollbacks=%d grants=%d denies=%d defers=%d\n",
    e.m.deadlocks_detected, e.m.aborts, e.m.rollbacks, e.m.grants, e.m.denies, e.m.defers);

  free(list.items);
  return 0;
}
