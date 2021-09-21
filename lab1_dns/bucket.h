//
// Created by Pavel on 12.09.2021.
//

#ifndef DNS_BUCKET_H
#define DNS_BUCKET_H

#include <malloc.h>
#include <string.h>

typedef unsigned int BOOL;
#define FALSE 0
#define TRUE  1

typedef const char* key_type;
typedef unsigned int value_type;
typedef struct {
    key_type domain;
    value_type ip;
} bucket_item;

typedef struct {
    bucket_item* data;
    size_t size;
    size_t capacity;
} Bucket;
typedef Bucket* BucketHandle;

BOOL BucketCreateWithCapacity(BucketHandle* p_handle, size_t capacity);

BOOL BucketCreate(BucketHandle* p_handle);

void BucketEmplaceback(BucketHandle self, key_type key, size_t domain_length, value_type value);

inline BOOL BucketFind(BucketHandle self, key_type key, value_type* out) {
    // No nullptr check for speed
    bucket_item* data = self->data;
    bucket_item* end = data + self->size;
    for (; data != end; ++data) {
        if (data->domain != NULL && 0 == strcmp(data->domain, key)) {
            *out = data->ip;
            return TRUE;
        }
    }
    return FALSE;
}

bucket_item* At(BucketHandle self, size_t index);

void BucketRelease(Bucket* self);

#endif //DNS_BUCKET_H
