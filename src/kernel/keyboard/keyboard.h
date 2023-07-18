#ifndef KEYBOARD_H

#define KEYBOARD_H

#include "../../std/stddef.h"

void update_keyboard();
char scan_code_as_ascii(char c);
void init_keyboard();
bool getc(char key);
bool keyboard_state[128];
char getk();

enum special_keys
{
    LEFT_SHIFT = 200,
    RIGHT_SHIFT,
    LEFT_ALT,
    LEFT_CNTRL,
    RIGHT_CNTRL,
    ESCAPE,
    BACKSPACE,
};
#endif
