//
// Created by Pavel on 12.09.2021.
//

#ifndef DNS_HASHTABLE_H
#define DNS_HASHTABLE_H

#include <stdio.h>
#include "bucket.h"
#include <math.h>

typedef unsigned int BOOL;
#define FALSE 0
#define TRUE  1
typedef unsigned int hash_type;
typedef const char* key_type;
typedef unsigned int value_type;

typedef struct {
    BucketHandle* hash_lookup;
    size_t size;
} Hashtable;
typedef Hashtable* HashtableHandle;

HashtableHandle HashCreate(size_t size);

void HashRelease(HashtableHandle self);

// Insert new entry when domain length is known
void HashInsertLength(HashtableHandle self, key_type key, size_t domain_length, value_type value);

// Insert new entry
void HashInsert(HashtableHandle self, key_type key, value_type value);

/* Find entry with specified key and outputs value (in out_value)
 * Returns: TRUE if key was found, FALSE if not.
 * */
BOOL HashFind(HashtableHandle self, key_type key, value_type* out_value);

//Functions to measure HashFunc effectiveness
double GetAverageSize(HashtableHandle self);
size_t GetMaximumSize(HashtableHandle self);

#endif //DNS_HASHTABLE_H
