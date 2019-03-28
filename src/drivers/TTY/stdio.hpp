#include "TTY.hpp"

class stdio : public Device
{
    uint64_t inputFile, outputFile;

public:
    static void Init();
    static uint64_t File();
    
    stdio(const char* input, const char* output);
    uint64_t Read(uint64_t pos, void* buffer, uint64_t bufferSize);
    void Write(uint64_t pos, void* buffer, uint64_t bufferSize);
};