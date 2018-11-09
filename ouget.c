#include <stdio.h>
#include <stdlib.h>
#include "o.h"

uchar get_byte()
{
    int     ch;

    ch = fgetc( o_file );
    if( ch == EOF ) {
        fprintf(stderr, "%s: Premature end of file\n");
        exit(3);
    }
    return( (uchar)ch );
}

int get_int()
{
    int    ch;

    ch = get_byte();
    ch += get_byte() << 8;

    return( ch );
}

word get_word()
{
    word    ch;

    ch = get_byte();
    ch += get_byte() << 8;

    return( ch );
}

dword get_long()
{
    dword   ch;

    ch = get_byte();
    ch += get_byte() << 8;
    ch += get_byte() << 16;
    ch += get_byte() << 24;
    return( ch );
}

void get_str( length, dest_string )
    int     length;
    char    *dest_string;
{
    int     count;

    count = length;

    while( count ) {
      *dest_string++ = get_byte();
      --count;
    }
    *dest_string = '\0';
}


int get_name( dest_string )
    char    *dest_string;
{
    int     length;

    length = get_byte();
    get_str( length, dest_string );

    return( length+1 );
}

int get_index( data )
    int     *data;
{
    word ch;

    ch = get_byte();
    if ( ch > 0x7F ) {
        ch = ((ch & 0x7F) << 8) + get_byte();
        *data = ch;
        return( 2 );
    } else {
        *data = ch;
        return( 1 );
    }
}
