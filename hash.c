#ifndef EMONEY_HASH_H
#define EMONEY_HASH_H

#include <stdint.h>
#include <stddef.h>
#include <openssl/sha.h>

#define HASH_SIZE 64  // SHA512 output size

// Core hashing functions
void sha512(const uint8_t* data, size_t len, uint8_t* hash);
void double_sha512(const uint8_t* data, size_t len, uint8_t* hash);

// Mining-specific hash checking
int check_hash_meets_target(const uint8_t* hash, const uint8_t* target);

// Utility functions
void hash_to_string(const uint8_t* hash, char* string);
int compare_hashes(const uint8_t* a, const uint8_t* b);

#endif // EMONEY_HASH_H