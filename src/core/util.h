#pragma once
#include <stddef.h>

int str_eq(const char* a, const char* b);
int to_int(const char* s);
size_t split_csv(char* line, char** out, size_t max_out);
