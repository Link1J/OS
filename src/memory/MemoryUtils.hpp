#include <stddef.h>

extern "C" void* memcpy(void* dsc, void* src, size_t n);
extern "C" void* memset(void* dsc, int val, size_t n);
extern "C" int memcmp(const void *s1, const void *s2, size_t n);