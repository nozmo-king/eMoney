#ifndef EMONEY_BLOCK_H
#define EMONEY_BLOCK_H

#include <stdint.h>
#include <stdbool.h>

#define HASH_SIZE 64  // SHA512 hash size
#define MAX_BLOCK_SIZE 1000000  // Initial 1MB limit

typedef struct {
    uint32_t version;
    uint8_t prev_block[HASH_SIZE];
    uint8_t merkle_root[HASH_SIZE];
    uint32_t timestamp;
    uint32_t bits;          // Difficulty target
    uint32_t nonce;
} BlockHeader;

typedef struct {
    BlockHeader header;
    uint32_t tx_count;
    struct Transaction** transactions;
} Block;

// Block functions
Block* create_block(const uint8_t* prev_block_hash);
void free_block(Block* block);
bool add_transaction(Block* block, struct Transaction* tx);
bool validate_block(const Block* block, uint32_t height);
void get_block_hash(const Block* block, uint8_t* hash);
uint32_t get_block_size(const Block* block);

// Header serialization
uint32_t serialize_block_header(const BlockHeader* header, uint8_t* buffer);
bool deserialize_block_header(const uint8_t* buffer, uint32_t size, BlockHeader* header);

#endif // EMONEY_BLOCK_H
