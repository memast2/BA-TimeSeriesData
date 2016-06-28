#pragma once

#include <stdio.h>

typedef unsigned long timestamp_t;

typedef struct {
  timestamp_t timestamp;
  double value;
} Measurement;


/*
 * Represents the combined data structure consisting of
 * the circular array and the B+ tree. You can call this
 * data structure whatever you want, I simply called it
 * SBTree for now.
 */
typedef struct {
  /*
   * you need to fill this in here with whatever
   * data you need.
   */
} SBTree


/*
 * Initializes the combined data structure
 *
 * Parameters:
 *   size: length of the circular array
 *   order: number of values per node in the B+ tree
 */
SBTree *SBTree_new(int size, int order);


/*
 * Destroys the SBTree and frees all allocated memory
 *
 * Parameters:
 *   self: the SBTree
 */
void SBTree_destroy(SBTree *self);


/*
 * Adds the given measurement to the SBTree and removes the last
 * measurement if the data structure reached its maximum capacity
 *
 * Parameters:
 *   self: the SBTree
 *   measurement: the measurement to insert
 */
void SBTree_shift(SBTree *self, Measurement *measurement);


/*
 * Returns the value of the measurement taken at a given time point
 * vai the value parameter. The function returns true if the value
 * was found and false otherwise.
 *
 * Parameters:
 *   self: the SBTree
 *   time: the time point of the measurement
 *   value: a pointer where the value of the measurement should be
 *          stored in
 */
bool SBTree_lookup(SBTree *self, timestamp_t time, double *value);


/*
 * Initializes a new neighborhood in the B+ tree.
 *
 * Parameters:
 *   tree: The  SBTree for this neighborhood
 *   measurement: measurement that constitutes this pattern cell
 *   pattern_length: length of the query pattern
 *   offset: position of the measurement within the query pattern
 *           cell. offset=1 means the oldest time point in the
 *           query pattern, offset=pattern_length means the latest
 *           time point in the query pattern.
 */
Neighborhood *Neighborhood_new(SBTree *tree, Measurement *measurement,
    int pattern_length, int offset);



/*
 * Destroys the Neighborhood and frees all allocated memory
 *
 * Parameters:
 *   self: the Neighborhood
 */
void Neighborhood_destroy(Neighborhood *self);



/*
 * Grows the neighborhood by one new value and returns its time point via the timestamp
 * parameter. The function returns true if there was a new unseen value and false otherwise
 *
 * Parameters
 *   self: the neighborhood
 *   timeset: a set of seen time points
 *   timestamp: used as a return value, contains the time point of the
 *              new still unseen value discovered by this function
 */
bool Neighborhood_grow(Neighborhood *self, TimeSet *timeset, timestamp_t *timestamp);
