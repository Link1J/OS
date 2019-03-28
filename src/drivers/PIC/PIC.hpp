#include "IDT.hpp"

class PIC
{
public:
    static void Init();

    static void EnableTimerIRQ();
    static void EnableKeyboardIRQ();
    static void KeyboardInterrupt(IDT::Registers* regs);
};