#ifndef EMONEY_TRANSACTION_H
#define EMONEY_TRANSACTION_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_TX_SIZE 100000
#define MAX_SCRIPT_SIZE 10000

typedef struct {
    uint8_t txid[64];       // SHA512 transaction ID
    uint32_t vout;          // Output index
    uint8_t* script;        // Input script
    uint32_t script_len;
    uint32_t sequence;      // Sequence number
} TxInput;

typedef struct {
    uint64_t value;         // Amount in satoshis
    uint8_t* script;        // Output script
    uint32_t script_len;
} TxOutput;

typedef struct {
    uint32_t version;
    uint32_t input_count;
    TxInput* inputs;
    uint32_t output_count;
    TxOutput* outputs;
    uint32_t locktime;
} Transaction;

// Core transaction functions
Transaction* create_transaction(void);
void free_transaction(Transaction* tx);
bool add_input(Transaction* tx, const uint8_t* txid, uint32_t vout, 
               const uint8_t* script, uint32_t script_len);
bool add_output(Transaction* tx, uint64_t value, 
                const uint8_t* script, uint32_t script_len);

// Validation and processing
bool validate_transaction(const Transaction* tx, uint32_t height);
void get_transaction_hash(const Transaction* tx, uint8_t* hash);
uint32_t get_transaction_size(const Transaction* tx);

// Coinbase specific
Transaction* create_coinbase(uint32_t height, uint64_t reward, 
                           const uint8_t* pubkey_hash);
bool is_coinbase(const Transaction* tx);

#endif // EMONEY_TRANSACTION_H
