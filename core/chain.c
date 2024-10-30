#include "chain.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

ChainState* init_chain(void) {
    ChainState* chain = malloc(sizeof(ChainState));
    if (!chain) return NULL;

    // Initialize genesis block
    chain->genesis = create_genesis_block();
    chain->tip = chain->genesis;
    chain->height = 0;
    chain->total_work = 0;
    chain->total_supply = 0;

    return chain;
}

bool add_block(ChainState* chain, Block* block) {
    // Validate block
    if (!validate_block(block, chain->height + 1)) {
        return false;
    }

    // Check if it builds on current tip
    if (memcmp(block->header.prev_block, chain->tip->header.merkle_root, HASH_SIZE) == 0) {
        // Simple case: extending current tip
        if (connect_block(chain, block)) {
            chain->tip = block;
            chain->height++;
            return true;
        }
        return false;
    }

    // Potential fork - need to handle reorg
    ChainIndex* new_tip = build_chain_index(chain, block);
    if (!new_tip) return false;

    if (new_tip->chain_work > get_chain_work(chain->tip)) {
        return handle_reorg(chain, new_tip);
    }

    return false;
}

static bool connect_block(ChainState* chain, Block* block) {
    // Calculate reward for this height
    uint64_t reward = calculate_block_reward(chain->height + 1);
    
    // Validate coinbase reward
    if (!validate_coinbase_reward(block->transactions[0], reward)) {
        return false;
    }

    // Update chain state
    chain->total_supply += reward;
    chain->total_work += calculate_block_work(block);

    return true;
}

static Block* create_genesis_block(void) {
    Block* genesis = malloc(sizeof(Block));
    if (!genesis) return NULL;

    // Genesis block header
    genesis->header.version = 1;
    memset(genesis->header.prev_block, 0, HASH_SIZE);
    genesis->header.timestamp = 1698652800; // October 30, 2024
    genesis->header.bits = 0x1d00ffff;
    genesis->header.nonce = 0;

    // Genesis coinbase
    genesis->tx_count = 1;
    genesis->transactions = malloc(sizeof(struct Transaction*));
    genesis->transactions[0] = create_genesis_coinbase();

    // Calculate merkle root
    calculate_merkle_root(genesis);

    return genesis;
}

uint32_t calculate_next_work_required(const ChainState* chain) {
    if ((chain->height + 1) % DIFFICULTY_ADJUSTMENT_INTERVAL != 0) {
        return chain->tip->header.bits;
    }

    // Get block from 2016 blocks ago
    Block* first = get_block_by_height(chain, chain->height - 2015);
    if (!first) return chain->tip->header.bits;

    // Calculate actual timespan
    int64_t actual_timespan = chain->tip->header.timestamp - first->header.timestamp;
    if (actual_timespan < TARGET_TIMESPAN / 4) actual_timespan = TARGET_TIMESPAN / 4;
    if (actual_timespan > TARGET_TIMESPAN * 4) actual_timespan = TARGET_TIMESPAN * 4;

    // Retarget
    uint256_t target;
    bits_to_target(chain->tip->header.bits, &target);
    target *= actual_timespan;
    target /= TARGET_TIMESPAN;

    uint32_t new_bits;
    target_to_bits(&target, &new_bits);

    return new_bits;
}

bool handle_reorg(ChainState* chain, ChainIndex* new_tip) {
    // Find fork point
    ChainIndex* fork = find_fork_point(chain->tip, new_tip);
    if (!fork) return false;

    // Disconnect blocks from current tip to fork
    ChainIndex* current = chain->tip;
    while (current != fork) {
        if (!disconnect_block(chain, current)) return false;
        current = current->prev;
    }

    // Connect blocks from fork to new tip
    current = new_tip;
    Block** blocks_to_connect = malloc(sizeof(Block*) * MAX_REORG_DEPTH);
    int connect_count = 0;

    while (current != fork) {
        blocks_to_connect[connect_count++] = current->block;
        current = current->prev;
    }

    // Connect in forward order
    for (int i = connect_count - 1; i >= 0; i--) {
        if (!connect_block(chain, blocks_to_connect[i])) {
            free(blocks_to_connect);
            return false;
        }
    }

    free(blocks_to_connect);
    chain->tip = new_tip;
    return true;
}

uint32_t get_height(const ChainState* chain) {
    return chain->height;
}

uint64_t get_total_supply(const ChainState* chain) {
    return chain->total_supply;
}

void free_chain(ChainState* chain) {
    if (!chain) return;
    
    // Free all blocks
    ChainIndex* current = chain->tip;
    while (current) {
        ChainIndex* prev = current->prev;
        free_block(current->block);
        free(current);
        current = prev;
    }

    free(chain);
}