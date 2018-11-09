#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "o.h"


int linnum_compare(LINE_T *btreeline, LINE_T*line)
{
    if ( btreeline->hex_offset > line->hex_offset ) {
        return( LEFT );
    } else {
        if ( btreeline->hex_offset < line->hex_offset ) {
            return( RIGHT );
        } else {
            return( EQUAL );
        }
    }
}


void linnum( word length)
{
  char            *record;
  char            text[80];
  char            cksum;
  int             i;
  LINE_T          *line;

  get_byte();
  get_byte();
  length -= 3;
  record = o_malloc(length);
  get_str( length, record );
  i = 0;
  while (length) {
    line = (LINE_T*)o_malloc(sizeof(LINE_T));
    line->hex_offset = *(word*)&record[i+2];
    line = insert(line, line_tree, TC linnum_compare)->data;
    line->line_number = *(word*)&record[i];
    sprintf(text, "; %4d: %04X",line->line_number,line->hex_offset);
    out_line(text,"","","");
    out_newline();
    i+=4;
    length-=4;
  }
  out_newline();
  cksum = get_byte();
  cksum = cksum;
}
