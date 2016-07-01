#include <math.h>

#include "timeset.h"
#include "dbg.h"
#include "uthash/uthash.h"


TimeSet *TimeSet_new()
{
  TimeSet *self = NULL;

  self = malloc(sizeof(TimeSet));
  check_mem(self);

  self->set = malloc(sizeof(TimeSetItem *));
  check_mem(self->set);
  *self->set = NULL;
  return self;

error:
  if (self != NULL) {
    free(self->set);
  }
  free(self);
  return NULL;
}


void TimeSet_destroy_(TimeSet *self)
{
  TimeSetItem *time, *tmp;
  HASH_ITER(hh, *self->set, time, tmp) {
    HASH_DEL(*self->set, time);
    free(time);
  }

  free(self->set);
  free(self);
}


void TimeSet_destroy(TimeSet **self)
{
  if (self != NULL && *self != NULL) {
    TimeSet_destroy_(*self);
    *self = NULL;
  }
}


bool TimeSet_contains(TimeSet *self, timestamp_t time)
{
  TimeSetItem *item;
  HASH_FIND(hh, *self->set, &time, sizeof(timestamp_t), item);
  return item != NULL;
}


void TimeSet_add(TimeSet *self, timestamp_t time)
{
  TimeSet_add_error(self, time, 0);
}


void TimeSet_add_error(TimeSet *self, timestamp_t time, double error)
{
  if (!TimeSet_contains(self, time)) {
    TimeSetItem *item = malloc(sizeof(TimeSetItem));
    item->time = time;
    item->error = error;
    HASH_ADD(hh, *self->set, time, sizeof(timestamp_t), item);
  }
}


int TimeSet_size(TimeSet *self)
{
  return HASH_COUNT(*self->set);
}



int TimeSetItem_cmp_by_error(const void *x, const void *y)
{
  TimeSetItem *item1 = (TimeSetItem *) x;
  TimeSetItem *item2 = (TimeSetItem *) y;
  if (fabs(item1->error - item2->error) <= 0.0000001) {
    return 0;
  } else if (item1->error < item2->error) {
    return -1;
  } else {
    return 1;
  }
}


int TimeSetItem_cmp_by_time(const void *x, const void *y)
{
  TimeSetItem *item1 = (TimeSetItem *) x;
  TimeSetItem *item2 = (TimeSetItem *) y;
  return item1->time - item2->time;
}
