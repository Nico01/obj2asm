#include <stdio.h>
#include <string.h>
#include "o.h"

#define NOTHING 0
#define ASCII   1
#define SPECIAL 2

#define BUFFSIZE    0x1000+0x10

char *buff_beg;                     /* Pointer to first character in buffer */
char *buff_end;                     /* Pointer to last character in buffer */
char *buff_cur;                     /* Pointer to last getc()'d character */

char buff[BUFFSIZE] = { '\0' };     /* Array for actual buffer contents */

void empty_string( length )
    int     length;
{
    int     prev_mode;
    int     curr_mode;
    int     out_count;
    char    out_buff[OPSIZE+1];
    int     this_char;
    char    byte_buff[5];
    int     byte_len;
    char    *pre_text;
    int     pre_len;

    prev_mode = NOTHING;
    out_count = 0;
    out_buff[0] = '\0';
    buff_reseek();
    while ( length ) {
        this_char = buff_regetc();
        if ( this_char < ' ' || this_char > '~' ) {
            curr_mode = SPECIAL;
            sprintf( byte_buff, "0%02Xh", this_char );
            byte_len = 4;
        } else {
            curr_mode = ASCII;
            if ( this_char == '\'' ) {
                strcpy( byte_buff, "''" );  /* Two apostrophes to indicate one */
                byte_len = 2;
            } else {
                sprintf( byte_buff, "%c", this_char );
                byte_len = 1;
            }
        }
        pre_text = "";                  /* Default to no separator */
        pre_len = 0;
        if ( prev_mode == ASCII && curr_mode == SPECIAL ) {
            pre_text = "',";            /* Separator between ascii and hex */
            pre_len = 2;
        }
        if ( prev_mode == SPECIAL && curr_mode == ASCII ) {
            pre_text = ",'";            /* Separator between hex and ascii */
            pre_len = 2;
        }
        if ( prev_mode == SPECIAL && curr_mode == SPECIAL ) {
            pre_text = ",";             /* Separator between hex codes */
            pre_len = 1;
        }
        if ( out_count + pre_len + byte_len >= OPSIZE ) {
            /*
            ** Output current line and proceed to next line
            */
            if ( prev_mode == ASCII ) {
                strcat( out_buff, "'" );
            }
            if ( pass == 3 ) {
                out_directive( "db" );
                out_operand( out_buff );
                out_endline();
            }
            out_count = 0;
            out_buff[0] = '\0';
            prev_mode = NOTHING;
            pre_text = "";              /* Back to no seperator */
            pre_len = 0;
        }
        if ( prev_mode == NOTHING && curr_mode == ASCII ) {
            pre_text = "'";
            pre_len = 1;
        }
        strcat( out_buff, pre_text );
        strcat( out_buff, byte_buff );
        out_count += pre_len + byte_len;
        prev_mode = curr_mode;
        if ( this_char == '\0' || this_char == '$' ) {
            /*
            ** Output current line (with this_char) and proceed to next line
            */
            if ( prev_mode == ASCII ) {
                strcat( out_buff, "'" );
            }
            if ( pass == 3 ) {
                out_directive( "db" );
                out_operand( out_buff );
                out_endline();
            }
            out_count = 0;
            out_buff[0] = '\0';
            prev_mode = NOTHING;
            pre_text = "";              /* Back to no seperator */
            pre_len = 0;
        }
        --length;
    }
    if ( prev_mode != NOTHING ) {
        if ( prev_mode == ASCII ) {
            strcat( out_buff, "'" );
        }
        if ( pass == 3 ) {
            out_directive( "db" );
            out_operand( out_buff );
            out_endline();
        }
    }
}

/*------------------------------------------------------------------------------
** buff_init() - Initialize the buffer variables for an empty buffer.
**------------------------------------------------------------------------------
*/
int buff_init( length )
    int     length;
{
    if ( length > BUFFSIZE ) fmt_error("Record too large");
    buff_beg = &buff[0];
    buff_cur = buff_beg;
    length = fread( buff_beg, sizeof(char), length, o_file );
    buff_end = buff_beg + length;  /* Last character will be at buff[length-1] */
    return( length );
}

/*------------------------------------------------------------------------------
** buff_add() - Add to an initialized the buffer.
**------------------------------------------------------------------------------
*/
int buff_add( length )
    int     length;
{
    if ( length + (buff_end-buff_beg) > BUFFSIZE ) 
        fmt_error("Adding too much to internal buffer");
    length = fread( buff_end, sizeof(char), length, o_file );
    buff_end = buff_end + length;   /* Last character at buff[length-1] */
    return( length );
}

/* --------------------------------------------------------------------------
** buff_getc() - Get a character from the buffer, if the buffer is empty, the
**              stream is read.  The buffer position pointer is adjusted
**              foreward 1 byte.  If the buffer position pointer is equal to
**              the buffer end pointer, then and EOF value is returned.
**------------------------------------------------------------------------------
*/
int buff_getc()
{
    uchar   ch;
    int             result;

    if ( buff_cur == buff_end ) {                   /* At End of Buffer? */
        result = EOF;
    } else {
        ch = *buff_cur++;
        result = ch;
    }
    return( result );
}

/*------------------------------------------------------------------------------
** buff_regetc() - Get a character which has already been getc'd.  The first
**                character regotten will be the first character after any
**                previous buff_regetc() or buff_empty().   If the count
**                of the number of characters in the buffer is zero, an EOF
**                value is returned.  Otherwise, The buffer beginning pointer
**                is adjusted foreward 1 byte and the character is returned.
**------------------------------------------------------------------------------
*/
int buff_regetc()
{
    int     result;

    if ( buff_beg == buff_end ) {                          /* Buffer empty? */
        result = EOF;
    } else {
        result = *buff_beg++;
        buff_cur = buff_beg;
    }
    return( result );
}

/*------------------------------------------------------------------------------
** buff_reseek() - Move the buffer position pointer back to the buffer
**                 beginning pointer.
**------------------------------------------------------------------------------
*/
void buff_reseek()
{
    buff_cur = buff_beg;            /* Position buff_getc() to beginning */
}

/*------------------------------------------------------------------------------
** buff_empty() - Moves the buffer beginning pointer to the buffer position 
**                pointer.
**------------------------------------------------------------------------------
*/
void buff_empty()
{
    buff_beg = buff_cur;            /* Forget all characters before cur ptr */
}
