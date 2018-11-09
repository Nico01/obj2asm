#include <stdio.h>
#include <string.h>
#include "o.h"

static char *esc_0x0X[] = {
    "fchs"   , "fabs"   ,  ""      , "",            /* [0C:0] - [0C:3] */
    "ftst"   , "fxam"   ,  ""      , "",            /* [0C:4] - [0C:7] */
    "fld1"   , "fldl2t" , "fldl2e" , "fldpi",       /* [0D:0] - [0D:3] */
    "fldlg2" , "fldln2" , "fldz"   , "",            /* [0D:4] - [0D:7] */
    "f2xm1"  , "fyl2x"  , "fptan"  , "fpatan",      /* [0E:0] - [0E:3] */
    "fxtract", "fprem1" , "fdecstp", "fincstp",     /* [0E:4] - [0E:7] */
    "fprem"  , "fyl2xp1", "fsqrt"  , "fsincos",     /* [0F:0] - [0F:3] */
    "frndint", "fscale" , "fsin"   , "fcos"     };  /* [0F:4] - [0F:7] */

static char *esc_0x1C[] = {
    "fneni"  , "fndisi" , "fnclex" , "fninit",      /* [1C:0] - [1C:3] */
    ""       , ""       , ""       , ""         };  /* [1C:4] - [1C:7] */

void esc_special( opcode, operand, esc_byte, reg )
    char            *opcode;
    char            *operand;
    int             esc_byte;
    int             reg;
{
    int             element;

    strcpy( opcode, "" );
    strcpy( operand, "" );
    
    /*
    ** This routine is bad, but it would make no more sense if it listed
    ** out all of the possible esc_byte and register combinations!
    */
    if ( esc_byte == 0x0A ) {
        if ( reg == 0x00 ) {
            strcpy( opcode, "fnop" );
        }
        return;
    }
    if ( esc_byte == 0x0B ) {
        return;
    }
    if ( esc_byte >= 0x0C && esc_byte <= 0x0F ) {
        element = (esc_byte - 0x0C) * 8 + reg;
        strcpy( opcode, esc_0x0X[element] );
        return;
    }
    if ( esc_byte >= 0x10 && esc_byte <= 0x1F ) {
        if ( esc_byte == 0x15 && reg == 0x01 ) {
            strcpy( opcode, "fucompp" );
        }
        if ( esc_byte == 0x1C ) {
            strcpy( opcode, esc_0x1C[reg] );
        }
        return;
    }
    if ( esc_byte == 0x22 || esc_byte == 0x23 || esc_byte == 0x29 
         || (esc_byte >= 0x2C && esc_byte <= 0x2F) ) {
        return;
    }
    if ( esc_byte == 0x33 ) {
        if ( reg == 0x01 ) {
            strcpy( opcode, "fcompp" );
        }
        return;
    }   
    if ( esc_byte == 0x32 ) {
        return;
    }
    if ( esc_byte == 0x38 || esc_byte == 0x39
      || esc_byte == 0x3A || esc_byte == 0x3B ) {
        return;
    }
    if ( esc_byte == 0x3C ) {
        if ( reg == 0x00 ) {
            strcpy( opcode, "fstsw" );
            strcpy( operand, "ax" );
        }
        return;
    }
    if ( esc_byte == 0x3D || esc_byte == 0x3E || esc_byte == 0x3F ) {
        return;
    }
    element = esc_byte & 0x0F;
    if (    element == 0x02 || element == 0x03 || element == 0x08
         || element == 0x09 || element == 0x0A || element == 0x0B ) {
        sprintf( operand, "st(%d)", reg );
    } else {
        if ( esc_byte >= 0x10 ) {
            sprintf( operand, "st(%d),st", reg );
        } else {
            sprintf( operand, "st,st(%d)", reg );
        }
    }
    if (    esc_byte == 0x24 || esc_byte == 0x34
         || esc_byte == 0x26 || esc_byte == 0x36 ) {
        element++;
    }
    if (    esc_byte == 0x25 || esc_byte == 0x35
         || esc_byte == 0x27 || esc_byte == 0x37 ) {
        --element;
    }
    if ( esc_byte == 0x09 ) {
        strcpy( opcode, "fxch" );
    } else {
        strcpy( opcode, esc_inst[element] );
    }
    if ( esc_byte >= 0x30 ) {
        strcat( opcode, "p" );
    }
}

