#pragma once

#include <stdbool.h>
#include "uthash/uthash.h"
#include "api.h"

typedef struct {
  timestamp_t time;
  double error;
  UT_hash_handle hh;
} TimeSetItem;


typedef struct {
  TimeSetItem **set;
} TimeSet;


TimeSet *TimeSet_new();

void TimeSet_destroy(TimeSet **self);

bool TimeSet_contains(TimeSet *self, timestamp_t time);

void TimeSet_add(TimeSet *self, timestamp_t time);

void TimeSet_add_error(TimeSet *self, timestamp_t time, double error);

int TimeSet_size(TimeSet *self);

int TimeSetItem_cmp_by_error(const void *x, const void *y);

int TimeSetItem_cmp_by_time(const void *x, const void *y);
