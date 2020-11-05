struct Message {
    size_t  package_number;
    size_t  used_symbols;
    char    data[16];
};

const char* NAME_COMMON_FIFO          = "/tmp/common_fifo"; 
const int   NUMBER_OF_ERRONEOUS_SENDS = 10000;
