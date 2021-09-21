//
// Created by Pavel on 17.09.2021.
//
#include "hashtable.h"
#define HashFunc Hash1
#define hash_const 0x5bd1e995
#define seed 0xbc9f1d34

// Self-made hash function
static inline hash_type Hash1(key_type key, size_t length, size_t size) {
    unsigned int hash = 0;
    int shift = 0;
    length++;
    key--;
    while (shift < 32 && --length > 0) {
        hash += (*(++key) & 0xDF) << shift;
        shift += 2;
    }
    hash = (hash << 13) | (hash >> 19);
    return hash % size;
}

// Part of MurMur2
static inline hash_type Hash2(key_type key, size_t length, size_t size) {
    unsigned int hash = seed;
    unsigned int k;
    while (length >= 4) {
        k = *(unsigned int*)key;
        hash ^= k;
        length -= 4;
        key += 4;
    }
    hash ^= hash_const;
    return hash % size;
}

HashtableHandle HashCreate(size_t size) {
    Hashtable* hashtable = (Hashtable*)malloc(sizeof(Hashtable));
    if (hashtable == NULL) {
        printf("Error in HashCreate (not enough memory)!\n");
        return NULL;
    }

    hashtable->hash_lookup = (BucketHandle*)malloc(sizeof(BucketHandle) * size);
    if (hashtable->hash_lookup == NULL) {
        free(hashtable);
        printf("Error in HashCreate (not enough memory)!\n");
        return NULL;
    }

    BucketHandle* end = hashtable->hash_lookup + size;
    for (BucketHandle* p_bucket_handle = hashtable->hash_lookup; p_bucket_handle != end; ++p_bucket_handle) {
        if(!BucketCreate(p_bucket_handle)) {
            for (BucketHandle* bucket_eraser = hashtable->hash_lookup; bucket_eraser != p_bucket_handle; ++bucket_eraser) {
                BucketRelease(*bucket_eraser);
            }
            free(hashtable->hash_lookup);
            free(hashtable);
            printf("Error in HashCreate (not enough memory)!\n");
            return NULL;
        }
    }
    hashtable->size = size;
    return hashtable;
}

void HashInsertLength(HashtableHandle self, key_type key, size_t domain_length, value_type value) {
    // No nullptr check for speed
    hash_type hash = HashFunc(key, domain_length, self->size);
    BucketEmplaceback(self->hash_lookup[hash], key, domain_length, value);
}

void HashInsert(HashtableHandle self, key_type key, value_type value) {
    size_t len = strlen(key);
    HashInsertLength(self, key, len, value);
}

BOOL HashFind(Hashtable* self, key_type key, value_type* out_value) {
    // No nullptr check for speed
    size_t len = strlen(key);
    hash_type hash = HashFunc(key, len, self->size);
    BucketHandle bucket = self->hash_lookup[hash];
    return BucketFind(bucket, key, out_value);
}

void HashRelease(HashtableHandle self) {
    if (NULL == self) {
        printf("Error in HashRelease (nullptr)!\n");
        return;
    }

    for (int i = 0; i < self->size; i++) {
        BucketRelease(self->hash_lookup[i]);
    }
    free(self->hash_lookup);
    free(self);
}

//Testing
double GetAverageSize(HashtableHandle self) {
    if (NULL == self) {
        printf("Error in GetAverageSize (nullptr)!\n");
        return -1;
    }

    long long sum = 0;
    size_t amount = 0;
    for (size_t i = 0; i < self->size; i++) {
        if ((self->hash_lookup[i])->size > 0) {
            amount++;
            sum += (self->hash_lookup[i])->size;
        }
    }

    return (double)sum / amount;
}

size_t GetMaximumSize(HashtableHandle self) {
    if (NULL == self) {
        printf("Error in GetMaximumSize (nullptr)!\n");
        return -1;
    }

    size_t max = 0;
    for (size_t i = 0; i < self->size; i++) {
        if ((self->hash_lookup[i])->size > max) {
            max = (self->hash_lookup[i])->size;
        }
    }

    return max;
}
