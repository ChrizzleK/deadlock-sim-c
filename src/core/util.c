#include "src/core/util.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

int str_eq(const char* a, const char* b) { return strcmp(a,b)==0; }

int to_int(const char* s) { return (int)strtol(s, NULL, 10); }

static char* trim(char* s) {
  while (*s && isspace((unsigned char)*s)) s++;
  char* e = s + strlen(s);
  while (e > s && isspace((unsigned char)e[-1])) e--;
  *e = 0;
  return s;
}

size_t split_csv(char* line, char** out, size_t max_out) {
  size_t n = 0;
  char* p = line;
  while (*p && n < max_out) {
    char* start = p;
    while (*p && *p != ',') p++;
    if (*p == ',') { *p = 0; p++; }
    out[n++] = trim(start);
  }
  return n;
}
