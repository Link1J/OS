#include "Error.hpp"

extern "C" void __cxa_pure_virtual()
{
    Error::Panic("Pure Virtual Function Call");
}