#include <stdio.h>
#include <stdlib.h>
#include "ouget.h"
#include "o.h"

uint8_t get_byte(void)
{
    int ch = fgetc(o_file);

    if (ch == EOF) {
        fprintf(stderr, "Premature end of input file\n");
        exit(3);
    }

    return ch;
}

int16_t get_int(void)
{
    int ch = get_byte();
    ch += get_byte() << 8;

    return ch;
}

uint16_t get_word(void)
{
    uint16_t ch = get_byte();
    ch += get_byte() << 8;

    return ch;
}

uint32_t get_long(void)
{
    uint32_t ch = get_byte();
    ch += get_byte() << 8;
    ch += get_byte() << 16;
    ch += get_byte() << 24;
    return ch;
}

void get_str(size_t length, char *dest_string)
{
    size_t count = length;

    while (count) {
        *dest_string++ = get_byte();
        --count;
    }

    *dest_string = '\0';
}

size_t get_name(char *dest_string)
{
    size_t length = get_byte();
    get_str(length, dest_string);

    return length + 1;
}

int16_t get_index(int *data)
{
    uint16_t ch = get_byte();

    if (ch > 0x7F) {
        ch = ((ch & 0x7F) << 8) + get_byte();
        *data = ch;
        return 2;
    }
    else {
        *data = ch;
        return 1;
    }
}
