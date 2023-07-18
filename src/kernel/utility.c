#include "../std/utility.h"

#include "../std/stddef.h"
uint32_t align(uint32_t number, uint32_t alignTo)
{
    if (alignTo == 0)
        return number;

    uint32_t rem = number % alignTo;
    return (rem > 0) ? (number + alignTo - rem) : number;
}
bool islower(char chr) { return chr >= 'a' && chr <= 'z'; }

char toupper(char chr) { return islower(chr) ? (chr - 'a' + 'A') : chr; }