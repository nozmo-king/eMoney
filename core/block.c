#include "block.h"
#include "../crypto/hash.h"
#include <stdlib.h>
#include <string.h>

Block* create_block(const uint8_t* prev_block_hash) {
    Block* block = malloc(sizeof(Block));
    if (!block) return NULL;
    
    // Initialize header
    block->header.version = 1;
    memcpy(block->header.prev_block, prev_block_hash, HASH_SIZE);
    block->header.timestamp = (uint32_t)time(NULL);
    block->header.bits = 0x1d00ffff;  // Initial difficulty
    block->header.nonce = 0;
    
    // Initialize transactions
    block->tx_count = 0;
    block->transactions = NULL;
    
    return block;
}

void free_block(Block* block) {
    if (!block) return;
    
    // Free all transactions
    for (uint32_t i = 0; i < block->tx_count; i++) {
        free(block->transactions[i]);
    }
    free(block->transactions);
    free(block);
}

bool add_transaction(Block* block, struct Transaction* tx) {
    // Check block size limit
    if (get_block_size(block) + get_transaction_size(tx) > MAX_BLOCK_SIZE) {
        return false;
    }
    
    // Reallocate transaction array
    struct Transaction** new_txs = realloc(block->transactions, 
                                         (block->tx_count + 1) * sizeof(struct Transaction*));
    if (!new_txs) return false;
    
    block->transactions = new_txs;
    block->transactions[block->tx_count++] = tx;
    
    // Update merkle root
    calculate_merkle_root(block);
    
    return true;
}

bool validate_block(const Block* block, uint32_t height) {
    // Check block size
    if (get_block_size(block) > get_max_block_size(height)) {
        return false;
    }
    
    // Validate header
    if (!validate_pow(block)) {
        return false;
    }
    
    // Check timestamp
    if (block->header.timestamp > time(NULL) + 2 * 60 * 60) {
        return false;
    }
    
    // Validate transactions
    for (uint32_t i = 0; i < block->tx_count; i++) {
        if (!validate_transaction(block->transactions[i], height)) {
            return false;
        }
    }
    
    // Verify merkle root
    uint8_t calculated_root[HASH_SIZE];
    calculate_merkle_root_hash(block, calculated_root);
    if (memcmp(calculated_root, block->header.merkle_root, HASH_SIZE) != 0) {
        return false;
    }
    
    return true;
}

void get_block_hash(const Block* block, uint8_t* hash) {
    uint8_t header_bytes[80];
    serialize_block_header(&block->header, header_bytes);
    sha512(header_bytes, 80, hash);
}

uint32_t get_block_size(const Block* block) {
    uint32_t size = sizeof(BlockHeader);
    
    // Add transaction sizes
    for (uint32_t i = 0; i < block->tx_count; i++) {
        size += get_transaction_size(block->transactions[i]);
    }
    
    return size;
}

uint32_t serialize_block_header(const BlockHeader* header, uint8_t* buffer) {
    uint32_t pos = 0;
    
    // Version
    memcpy(buffer + pos, &header->version, 4);
    pos += 4;
    
    // Previous block hash
    memcpy(buffer + pos, header->prev_block, HASH_SIZE);
    pos += HASH_SIZE;
    
    // Merkle root
    memcpy(buffer + pos, header->merkle_root, HASH_SIZE);
    pos += HASH_SIZE;
    
    // Timestamp
    memcpy(buffer + pos, &header->timestamp, 4);
    pos += 4;
    
    // Bits
    memcpy(buffer + pos, &header->bits, 4);
    pos += 4;
    
    // Nonce
    memcpy(buffer + pos, &header->nonce, 4);
    pos += 4;
    
    return pos;
}

bool deserialize_block_header(const uint8_t* buffer, uint32_t size, BlockHeader* header) {
    if (size < 80) return false;
    
    uint32_t pos = 0;
    
    // Version
    memcpy(&header->version, buffer + pos, 4);
    pos += 4;
    
    // Previous block hash
    memcpy(header->prev_block, buffer + pos, HASH_SIZE);
    pos += HASH_SIZE;
    
    // Merkle root
    memcpy(header->merkle_root, buffer + pos, HASH_SIZE);
    pos += HASH_SIZE;
    
    // Timestamp
    memcpy(&header->timestamp, buffer + pos, 4);
    pos += 4;
    
    // Bits
    memcpy(&header->bits, buffer + pos, 4);
    pos += 4;
    
    // Nonce
    memcpy(&header->nonce, buffer + pos, 4);
    
    return true;
}