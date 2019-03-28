#ifndef __TTY_HPP__
#define __TTY_HPP__

#include "Device.hpp"
#include "Vector2.hpp"
#include "Screen.hpp"

class TTY : public Device
{
public:
    TTY(const char* name);
    uint64_t Read(uint64_t pos, void* buffer, uint64_t bufferSize);
    virtual void Write(uint64_t pos, void* buffer, uint64_t bufferSize) = 0;
};

class TTYScreen : public TTY
{
    float scale = 1;
	Color	background;
	Color	foreground;
	Vector2	cursor;
	Vector2	charCount;

    void PrintChar(char c);
    void PrintSymbol(char c);
    void NewLine();
public:
    TTYScreen(Color foregound, Color backgound);
    void Write(uint64_t pos, void* buffer, uint64_t bufferSize);
};

#endif