#include <stdio.h>
#include <string.h>
#include "o.h"

/*
** Local Variables
*/
int tab_at = 0;
static int col_cnt = 0;

/*
** Local Prototypes
*/
void tab_count( char * );
void tab_seek( int );
void tab_next( void );
void out_label( char * );
void out_opcode( char * );
void out_operand( char * );
void out_comment( char * );
void out_endline( void );
void out_newline( void );
void out_directive( char * );
void out_line( char *, char *, char *, char * );

void tab_count( text )
    char    *text;
{
    int     count;

    count = strlen( text );
    while ( count  ) {
        if ( *text == '\t' ) {
            col_cnt = (col_cnt | 0x07) + 1;
        } else {
            col_cnt++;
        }
        text++;
        --count;
    }
    tab_at = (col_cnt >> 3) + 1;
}


void tab_seek( tab_count )
    int     tab_count;
{
    if ( tab_count <= tab_at ) {
        putc( '\n', stdout );
        tab_count -= tab_offset;
        tab_offset = 0;
        tab_at = 0;
        col_cnt = 0;
    }
    if ( tab_at == 0 ) {
        tab_at = 1;
    }

    while( tab_at < tab_count ) {
        putc( '\t', stdout );
        tab_at++;
        col_cnt = (tab_at - 1) << 3;
    }
}

void tab_next()
{
    tab_seek( tab_at + 1 );
}

void out_label( label )
    char    *label;
{
    tab_seek( 1 );                  /* Start a new line if needed */
    printf( "%s", label );
    tab_count( label );
}

void out_opcode( opcode )
    char    *opcode;
{
    tab_seek( 2 + tab_offset );     /* Indent to first tab stop */
    printf( "%s", opcode );
    tab_count( opcode );
}

void out_operand( operand )
    char    *operand;
{
    if ( strcmp(operand,"") ) {
        tab_next();                 /* Indent to next tab stop */
        printf( "%s", operand );
        tab_count( operand );
    }
}

void out_comment( comment )
    char    *comment;
{
    if ( strcmp(comment,"") ) {
        tab_seek( 7 );          /* Indent to next tab stop */
        printf( "; %s", comment );
        col_cnt += 2;
        tab_count( comment );
    }
}

void out_endline()
{
    putc( '\n', stdout );
    tab_offset = 0;
    tab_at = 0;
    col_cnt = 0;
}

void out_newline()
{
// /*    if ( tab_at != 0 ) {          /* Finish off previous line if needed */
//        out_endline();
//    } */
    out_endline();
}

void out_directive( direct )
    char    *direct;
{
    tab_next();
    if ( tab_at < 2 ) {
        tab_next();
    }
    printf( "%s", direct );
    tab_count( direct );
}

void out_line( label, opcode, operand, comment )
    char    *label;
    char    *opcode;
    char    *operand;
    char    *comment;
{
    out_label( label );
    out_directive( opcode );
    out_operand( operand );
    out_comment( comment );
}

char *out_hexize( offset, text, bytes )
    dword   offset;
    char            *text;
    int             bytes;
{
    char            temp[15];

    switch( bytes ) {
        case 1: sprintf( temp, "%02Xh", (uchar)offset );
                break;
        case 2: sprintf( temp, "%04Xh", (word)offset );
                break;
        case 4: sprintf( temp, "%08lXh", offset );
                break;
        default:
                fmt_error( "Internal error calling out_hexize()" );
                break;
    }

    if ( temp[0] >= '0' && temp[0] <= '9' ) {
        strcpy( text, temp );
    } else {
        sprintf( text, "0%s", temp );
    }
    return( text );
}
