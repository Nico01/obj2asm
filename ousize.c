#include <stdio.h>
#include <string.h>
#include "o.h"

/*
** Local Variables
*/

int size_to_type( size )
    int     size;
{
    int     result;

    switch( size ) {
        case  1: result = BYTE_PTR;     break;
        case  2: result = WORD_PTR;     break;
        case  4: result = DWORD_PTR;    break;
        case  6: result = FWORD_PTR;    break;
        case  8: result = QWORD_PTR;    break;
        case 10: result = TBYTE_PTR;    break;
        default: result = UNKNOWN;      break;
    }
    return( result );
}

int reg_size_to_type( reg_size )
    int reg_size;
{
    int     result;

    switch( reg_size ) {
        case 0:     result = BYTE_PTR;      break;
        case 1:     result = WORD_PTR;      break;
        case 2:     result = DWORD_PTR;     break;
        case 3:     result = FWORD_PTR;     break;
        default:    result = UNKNOWN;       break;
    }

    return( result );
}

int type_to_size( type )
    int     type;
{
    int     result;

    switch( type ) {
        case  BYTE_PTR:     result =  1;    break;
        case  WORD_PTR:     result =  2;    break;
        case  DWORD_PTR:    result =  4;    break;
        case  FWORD_PTR:    result =  6;    break;
        case  QWORD_PTR:    result =  8;    break;
        case  TBYTE_PTR:    result = 10;    break;
        default:            result =  0;    break;
    }
    return( result );
}

char *type_to_text( type )
    int     type;
{
    char    *result;

    switch( type ) {
        case BYTE_PTR:      result = "byte ptr ";   break;
        case WORD_PTR:      result = "word ptr ";   break;
        case DWORD_PTR:     result = "dword ptr ";  break;
        case FWORD_PTR:     result = "fword ptr ";  break;
        case QWORD_PTR:     result = "qword ptr ";  break;
        case TBYTE_PTR:     result = "tbyte ptr ";  break;
        default:            result = "";            break;
    }

    return( result );
}


char *size_to_opcode( size, times )
    int     size;
    int     *times;
{
    char    *result;

    *times = 1;

    switch( size ) {
        case 0:     result = "*ERROR*"; *times = 0; break;
        case 1:     result = "db";                  break;
        case 2:     result = "dw";                  break;
        case 4:     result = "dd";                  break;
        case 6:     result = "df";                  break;
        case 8:     result = "dq";                  break;
        case 10:    result = "dt";                  break;
        default:    result = "db";  *times = size;  break;
    }
    return( result );
}
