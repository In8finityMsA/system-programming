//
// Created by Pavel on 17.09.2021.
//
#include <stdio.h>
#include "bucket.h"

#define INCREASE_COEF 2
#define DEFAULT_CAPACITY 2

BOOL BucketCreateWithCapacity(BucketHandle* p_handle, size_t capacity) {
    if (NULL == p_handle || capacity == 0) {
        printf("Error in BucketCreateWithCapacity!\n");
        return FALSE;
    }

    BucketHandle bucket = (BucketHandle)malloc(sizeof(Bucket));
    if (bucket == NULL) {
        printf("Error in BucketCreateWithCapacity (not enough memory)!\n");
        return FALSE;
    }
    bucket->size = 0;
    bucket->capacity = capacity;
    bucket->data = (bucket_item*)malloc(sizeof(bucket_item) * bucket->capacity);

    if (bucket->data == NULL) {
        free(bucket);
        printf("Error in BucketCreateWithCapacity (not enough memory)!\n");
        return FALSE;
    }
    memset(bucket->data, 0, sizeof(bucket_item) * bucket->capacity);

    *p_handle = bucket;
    return TRUE;
}

BOOL BucketCreate(BucketHandle* p_handle) {
    return BucketCreateWithCapacity(p_handle, DEFAULT_CAPACITY);
}

void BucketEmplaceback(BucketHandle self, key_type key, size_t domain_length, value_type value) {
    if (NULL == self || NULL == key) {
        printf("Error in BucketEmplaceback!\n");
        return;
    }

    if (self->size >= self->capacity) {
        bucket_item* new_data;
        new_data = (bucket_item*)malloc(sizeof(bucket_item) * self->capacity * INCREASE_COEF);
        if (new_data == NULL) {
            printf("Error in BucketEmplaceback (not enough memory)!\n");
            return;
        }
        memcpy_s(new_data, sizeof(bucket_item) * self->capacity * INCREASE_COEF,
                 self->data, sizeof(bucket_item) * self->capacity);

        free(self->data);
        self->data = new_data;
        self->capacity *= INCREASE_COEF;
    }

    char* name = (char*)malloc(domain_length + 1);
    strcpy_s(name, domain_length + 1, key);
    bucket_item item = {name, value};
    self->data[self->size++] = item;
}

bucket_item* BucketAt(BucketHandle self, size_t index) {
    if (NULL == self || index >= self->size) {
        printf("Error in BucketAt!\n");
        return NULL;
    }
    return self->data + index;
}

void BucketRelease(BucketHandle self) {
    if (NULL == self) {
        printf("Error in BucketRelease (nullptr)!\n");
        return;
    }

    for (int i = 0; i < self->size; i++) {
        free(self->data[i].domain);
    }
    free(self->data);
    free(self);
}

