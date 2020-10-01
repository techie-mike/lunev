#define NUM_BYTES_DATA 46

struct Message {
    unsigned int packege_number;
    unsigned int total_packages;
    unsigned int serial_number;
    unsigned int used_symbols;
    unsigned char data[NUM_BYTES_DATA];
};