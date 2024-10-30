#include "transaction.h"
#include "../crypto/hash.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

Transaction* create_transaction(void) {
    Transaction* tx = malloc(sizeof(Transaction));
    if (!tx) return NULL;
    
    tx->version = 1;
    tx->input_count = 0;
    tx->output_count = 0;
    tx->inputs = NULL;
    tx->outputs = NULL;
    tx->locktime = 0;
    
    return tx;
}

bool add_input(Transaction* tx, const uint8_t* txid, uint32_t vout, 
               const uint8_t* script, uint32_t script_len) {
    if (script_len > MAX_SCRIPT_SIZE) return false;
    
    // Resize inputs array
    TxInput* new_inputs = realloc(tx->inputs, 
                                 (tx->input_count + 1) * sizeof(TxInput));
    if (!new_inputs) return false;
    tx->inputs = new_inputs;
    
    // Set up new input
    TxInput* input = &tx->inputs[tx->input_count];
    memcpy(input->txid, txid, 64);
    input->vout = vout;
    input->script = malloc(script_len);
    if (!input->script) return false;
    memcpy(input->script, script, script_len);
    input->script_len = script_len;
    input->sequence = 0xffffffff;
    
    tx->input_count++;
    return true;
}

bool add_output(Transaction* tx, uint64_t value, 
                const uint8_t* script, uint32_t script_len) {
    if (script_len > MAX_SCRIPT_SIZE) return false;
    
    // Resize outputs array
    TxOutput* new_outputs = realloc(tx->outputs, 
                                  (tx->output_count + 1) * sizeof(TxOutput));
    if (!new_outputs) return false;
    tx->outputs = new_outputs;
    
    // Set up new output
    TxOutput* output = &tx->outputs[tx->output_count];
    output->value = value;
    output->script = malloc(script_len);
    if (!output->script) return false;
    memcpy(output->script, script, script_len);
    output->script_len = script_len;
    
    tx->output_count++;
    return true;
}

Transaction* create_coinbase(uint32_t height, uint64_t reward, 
                           const uint8_t* pubkey_hash) {
    Transaction* tx = create_transaction();
    if (!tx) return NULL;
    
    // Create input with height in script
    uint8_t height_script[6];
    int script_len = encode_varint(height, height_script);
    
    // Add null input (coinbase has no real inputs)
    uint8_t null_txid[64] = {0};
    if (!add_input(tx, null_txid, 0xffffffff, height_script, script_len)) {
        free_transaction(tx);
        return NULL;
    }
    
    // Create P2PKH output script
    uint8_t output_script[25];
    output_script[0] = 0x76;  // OP_DUP
    output_script[1] = 0xa9;  // OP_HASH160
    output_script[2] = 0x14;  // Push 20 bytes
    memcpy(output_script + 3, pubkey_hash, 20);
    output_script[23] = 0x88;  // OP_EQUALVERIFY
    output_script[24] = 0xac;  // OP_CHECKSIG
    
    if (!add_output(tx, reward, output_script, 25)) {
        free_transaction(tx);
        return NULL;
    }
    
    return tx;
}

bool validate_transaction(const Transaction* tx, uint32_t height) {
    // Basic checks
    if (tx->input_count == 0 || tx->output_count == 0) return false;
    if (get_transaction_size(tx) > MAX_TX_SIZE) return false;
    
    // Check if coinbase
    bool is_cb = is_coinbase(tx);
    if (is_cb) {
        return validate_coinbase(tx, height);
    }
    
    // Calculate total input/output values
    uint64_t total_in = 0;
    uint64_t total_out = 0;
    
    // Validate and sum inputs
    for (uint32_t i = 0; i < tx->input_count; i++) {
        // Get previous output
        TxOutput* prev_output = get_utxo(tx->inputs[i].txid, 
                                       tx->inputs[i].vout);
        if (!prev_output) return false;
        
        // Validate input script
        if (!validate_script(tx, i, prev_output->script, 
                           prev_output->script_len)) {
            return false;
        }
        
        total_in += prev_output->value;
    }
    
    // Sum outputs
    for (uint32_t i = 0; i < tx->output_count; i++) {
        total_out += tx->outputs[i].value;
        if (total_out > total_in) return false;  // Check for overflow
    }
    
    // Ensure outputs don't exceed inputs
    return total_out <= total_in;
}

void get_transaction_hash(const Transaction* tx, uint8_t* hash) {
    // Serialize transaction
    uint8_t* buffer = malloc(get_transaction_size(tx));
    uint32_t size = serialize_transaction(tx, buffer);
    
    // Double SHA512
    double_sha512(buffer, size, hash);
    
    free(buffer);
}

uint32_t get_transaction_size(const Transaction* tx) {
    uint32_t size = 8;  // Version + input/output counts
    
    // Add input sizes
    for (uint32_t i = 0; i < tx->input_count; i++) {
        size += 104;  // txid + vout + sequence
        size += tx->inputs[i].script_len;
    }
    
    // Add output sizes
    for (uint32_t i = 0; i < tx->output_count; i++) {
        size += 8;  // value
        size += tx->outputs[i].script_len;
    }
    
    size += 4;  // locktime
    return size;
}

bool is_coinbase(const Transaction* tx) {
    if (tx->input_count != 1) return false;
    
    // Check for null txid
    const uint8_t* txid = tx->inputs[0].txid;
    for (int i = 0; i < 64; i++) {
        if (txid[i] != 0) return false;
    }
    
    return tx->inputs[0].vout == 0xffffffff;
}

void free_transaction(Transaction* tx) {
    if (!tx) return;
    
    // Free input scripts
    for (uint32_t i = 0; i < tx->input_count; i++) {
        free(tx->inputs[i].script);
    }
    free(tx->inputs);
    
    // Free output scripts
    for (uint32_t i = 0; i < tx->output_count; i++) {
        free(tx->outputs[i].script);
    }
    free(tx->outputs);
    
    free(tx);
}
