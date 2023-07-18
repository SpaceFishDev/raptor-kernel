#include "keyboard.h"
#include "../x86.h"
#include "../../std/stdio.h"

char kbd_US[128] =
    {
        0,
        ESCAPE,
        '1',
        '2',
        '3',
        '4',
        '5',
        '6',
        '7',
        '8',
        '9',
        '0',
        '-',
        '=',
        BACKSPACE,
        '\t',
        'q',
        'w',
        'e',
        'r',
        't',
        'y',
        'u',
        'i',
        'o',
        'p',
        '[',
        ']',
        '\n',
        LEFT_CNTRL,
        'a',
        's',
        'd',
        'f',
        'g',
        'h',
        'j',
        'k',
        'l',
        ';',
        '\'',
        '`',
        LEFT_SHIFT,
        '\\',
        'z',
        'x',
        'c',
        'v',
        'b',
        'n',
        'm',
        ',',
        '.',
        '/',
        RIGHT_SHIFT,
        '*',
        LEFT_ALT, /* Alt */
        ' ',      /* Space bar */
        0,        /* Caps lock */
        0,        /* 59 - F1 key ... > */
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0, /* < ... F10 */
        0, /* 69 - Num lock*/
        0, /* Scroll Lock */
        0, /* Home key */
        0, /* Up Arrow */
        0, /* Page Up */
        '-',
        0, /* Left Arrow */
        0,
        0, /* Right Arrow */
        '+',
        0, /* 79 - End key*/
        0, /* Down Arrow */
        0, /* Page Down */
        0, /* Insert Key */
        0, /* Delete Key */
        0,
        0,
        0,
        0, /* F11 Key */
        0, /* F12 Key */
        0,
};

char scan_code_as_ascii(char c)
{
    return kbd_US[c];
}
bool getk(char *key)
{
    char c = inb(0x60);
    if ((c) > 128)
    {
        *key = scan_code_as_ascii(c & ~0x80);
        return false;
    }
    *key = scan_code_as_ascii(c);
    return true;
}

void update_keyboard()
{
    char c = inb(0x60);
    if (c > 128)
    {
        keyboard_state[kbd_US[c & ~0x80]] = false;
        return;
    }
    keyboard_state[kbd_US[c]] = true;
}

void init_keyboard()
{
    for (int i = 0; i < 128; ++i)
    {
        keyboard_state[i] = false;
    }
}

bool getc(char key)
{
    return keyboard_state[key];
}