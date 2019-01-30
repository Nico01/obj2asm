#ifndef OUGET_H
#define OUGET_H

#include <stdint.h>

uint8_t get_byte(void);
int16_t get_int(void);
uint16_t get_word(void);
uint32_t get_long(void);
void get_str(size_t length, char *dest_string);
size_t get_name(char *dest_string);
int16_t get_index(int *data);

#endif // OUGET_H
