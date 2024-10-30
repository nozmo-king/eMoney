#ifndef EMONEY_CHAIN_H
#define EMONEY_CHAIN_H

#include "block.h"
#include <stdint.h>
#include <stdbool.h>

#define MAX_REORG_DEPTH 100
#define DIFFICULTY_ADJUSTMENT_INTERVAL 2016
#define TARGET_TIMESPAN 14 * 24 * 60 * 60  // Two weeks

typedef struct ChainState {
    Block* genesis;
    Block* tip;
    uint32_t height;
    uint32_t total_work;
    uint64_t total_supply;
} ChainState;

typedef struct {
    Block* block;
    uint32_t height;
    struct ChainIndex* prev;
    struct ChainIndex* next;
    uint64_t chain_work;
} ChainIndex;

// Chain initialization
ChainState* init_chain(void);
void free_chain(ChainState* chain);

// Block processing
bool add_block(ChainState* chain, Block* block);
bool remove_block(ChainState* chain, const uint8_t* hash);
Block* get_block(const ChainState* chain, const uint8_t* hash);

// Chain queries
uint32_t get_height(const ChainState* chain);
bool get_block_by_height(const ChainState* chain, uint32_t height, Block** block);
uint64_t get_total_supply(const ChainState* chain);

// Difficulty adjustment
uint32_t calculate_next_work_required(const ChainState* chain);

// Chain validation
bool is_chain_valid(const ChainState* chain);
bool handle_reorg(ChainState* chain, ChainIndex* new_tip);

#endif // EMONEY_CHAIN_H