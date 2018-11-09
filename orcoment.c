#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "o.h"
                                        /*----- Comment Record Types ------*/
#define MSLANG  0                       /* 00h - MS Language Name          */
#define MSDOSV  156                     /* BCh - MS DOS Level Number (?)   */
#define MSMODL  157                     /* BDh - MS Memory Model (+opts)   */
#define MSDSEG  158                     /* BEh - MS Forced 'DOSSEG' switch */
#define MSILIB  159                     /* A0h - MS INCLUDELIB directive   */
#define MSEXTN  161                     /* A1h - MS Extensions Enabled     */
#define UNKNWN  162                     /* A2h - (?)                       */
#define MSLNAM  163                     /* A3h - MS Library Module Name    */
#define PATIME  221                     /* DDh - Phoenix Time Stamp        */
#define PACMNT  255                     /* FFh - Phoenix Comment           */
#define TCXSYMTYPIX 0xe0
#define TCPUBSYMTYP 0xe1
#define TCSTRUCT 0xe2
#define TCTYPDEF 0xe3
#define TCENUM   0xe4
#define TCBEGSCP 0xe5
#define TCLOCDEF 0xE6
#define TCENDSCP 0xe7
#define TCSOURCE 0xe8
#define TCDEPFIL 0xe9
#define TCXLATOR 0xea
#define TCMANGLE 0xf8

static char *models[] = {
  "tiny","small","medium","compact","large","huge"
};

typedef struct {
  word seconds:5;
  word minute:6;
  word hour:5;
} TIME;


typedef struct {
  word day:5;
  word month:4;
  word year:7;
} DATE;

static char *months[] = {
  "(nul)",
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

static void sprint_date(char*string,char*datestr)
{
  TIME time;
  DATE date;

  time = *(TIME*)datestr;
  date = *(DATE*)&datestr[2];
  sprintf(string, "%02d:%02d:%02d on %.3s %d, %04d", time.hour, time.minute, time.seconds,
                months[date.month], date.day, date.year+1980);
}


int scope_compare(SCOPE_T *btreescope, SCOPE_T *scope)
{
  if ( btreescope->hex_offset > scope->hex_offset ) {
      return( LEFT );
  } else {
      if ( btreescope->hex_offset < scope->hex_offset ) {
          return( RIGHT );
      } else {
          return( EQUAL );
      }
  }
}



void coment( word length)
{
  char            junk[3];
  char            *comment;
  char            text[80];
  char            temp[80];
  int             type;
  int             class;
  char            cksum;
  uchar           *prt_char;
  int             i;
  int             w;
  char            *p;
  LOCAL_VAR       *local_var;
  static word     scope_type = VT_VAR;
  static NODE_T **ppScope_tree;
  static SCOPE_T *pScope;
  static SCOPE_T **ppScope;
  static SCOPE_T *arg_scope;
  static SCOPE_T *loc_scope;

  type = get_byte();
  class = get_byte();
  length -= 3;
  if (length)
    comment = o_malloc(length);
  else
    comment = junk;
  get_str( length, comment);
  switch( class ) {
  case MSLANG:
       out_line( "", "; Compiler:", comment, "" );
       break;
  case MSMODL:
       processor_type_comment_occurred = TRUE;
       switch( comment[0] ) {
       case '3':
            out_line( "", ".386p", "", "" );
            out_line( "", ".387", "", "" );
            processor_mode = 386;
            break;
       case '2':
            out_line( "", ".286p", "", "" );
            out_line( "", ".287", "", "" );
            processor_mode = 286;
            break;
       case '1':
            out_line( "", ".186", "", "" );
            out_line( "", ".187", "", "" );
            processor_mode = 186;
            break;
       case '0':
            out_line( "", ".8086", "", "" );
            out_line( "", ".8087", "", "" );
            processor_mode = 8086;
            break;
       default:
            fmt_error( "Unknown Processor Type" );
            break;
       }
       switch( comment[1] ) {
       case 'c':
            out_line( "", "; Compact Memory Model", "", "" );
            break;
       case 's':
            out_line( "", "; Small Memory Model", "", "" );
            break;
       case 'm':
            out_line( "", "; Medium Memory Model", "", "" );
            break;
       case 'l':
            out_line( "", "; Large Memory Model", "", "" );
            break;
       case 'h':
            out_line( "", "; Huge Memory Model", "", "" );
            break;
       default:
            fmt_error( "Unknown Model" );
            break;
       }
       if ( comment[2] == 'O' )
           out_line( "", "; Optimizations Enabled", "", "" );
       break;
  case MSDSEG:
       out_line( "", "; Force DOSSEG linker option", "", "" );
       break;
  case MSILIB:
       out_line( "", "includelib", comment, "" );
       break;
  case MSEXTN:
       if (length) {
         if ( !strcmp( comment, "\001CV" ) ) {
             out_line( "", "; CodeView Enabled", "", "" );
         } else {
             fmt_error( "Unknown CodeView Option" );
         }
       }
       break;
  case UNKNWN:
       out_line( "", "; Linker - Pass two marker","","");
       break;
  case PATIME:
       if ( comment[0] != 16 )
         fmt_error( "Unknown Phoenix Time Stamp Prefix" );
       out_line( "", "; Time: ", &comment[1], "" );
       break;
  case PACMNT:
       out_line( "", ";", comment, "" );
       break;

/*
   Extensions to COMENT record defined by Borland.

   Note that these have been deduced using TDUMP (and lots of examples)
   and consequently may be completely wrong.
                                                                   [rh]
*/

  case TCXSYMTYPIX:
       sprintf(text, "; External symbol type index %02X", *comment, comment[1]&1);
       out_line(text,"","","");
       break;

  case TCPUBSYMTYP:
       sprintf(text, "; Public symbol type %02X, function return offset %02X", *comment, comment[1]&1);
       out_line(text,"","","");
       break;

  case TCSTRUCT:
       out_line("; typedef struct:","","","");
       i = 1;
       while (i<length) {
         out_newline();
         sprintf(text, "; \"%.*s\"",comment[i],&comment[i+1]);
         i+=comment[i]+1;
         sprintf(temp, "type %02X",comment[i]); /* ;* */
         i+=2;
         out_line(text,"","",temp);
       }
       break;

  case TCDEPFIL:
       if (length<=3) {
         out_line("; End of dependency list","","","");
       } else {
         sprintf(text,"%.*s",(int)comment[4],&comment[5]);
         out_line("; Dependent file: ",text,"","");
       }
       break;

  case TCENDSCP:
       pScope = o_malloc(sizeof(SCOPE_T));
       pScope->hex_offset = *(int*)comment;
       pScope = insert(pScope, end_scope_tree,TC scope_compare)->data;
       sprintf(text, "; End of scope at offset %04X",*(int*)comment);
       out_line(text,"","","");
       break;

  case TCBEGSCP:
       if (scope_type==VT_ARG)
         scope_type = VT_VAR;
       else
         scope_type = VT_ARG;
       if (scope_type==VT_ARG) {
         ppScope_tree = &arg_scope_tree;
         ppScope      = &arg_scope;
       } else {
         ppScope_tree = &loc_scope_tree;
         ppScope      = &loc_scope;
       }
       *ppScope = o_malloc(sizeof(SCOPE_T));
       (*ppScope)->hex_offset = *(int*)&comment[1];
       *ppScope = insert(*ppScope,*ppScope_tree, TC scope_compare)->data;
       sprintf(text,"; Begin scope segment %04X, offset %04X",(int)*comment,*(int*)&comment[1]);
       out_line(text,"","","");
       break;

  case TCLOCDEF:
       out_line("; Local definitions:","","","");
       i = 0;
       local_var = (*ppScope)->head = o_malloc(sizeof(LOCAL_VAR));
       while (i+1<length) {
         if (i!=0)
           local_var=local_var->next = o_malloc(sizeof(LOCAL_VAR));
         out_newline();
         sprintf(text,"; \"%.*s\"",comment[i],&comment[i+1]);
         sprintf(local_var->vname,"%.*s",comment[i],&comment[i+1]);
         i+=comment[i]+1;
         sprintf(temp,", type %02X, class %02X",comment[i],comment[i+1]&7);
         strcat(text,temp);
         *temp = 0;
         i++;
         local_var->class = comment[i]&7;
         switch (comment[i]&7) {
         case 7:
              sprintf(temp, " (instance of typed variable)");
              local_var->bInfo1 = comment[i+1];
              i++;
              break;
         case 6:                           /* local typedef ? */
              sprintf(temp, " (local typedef)");
              local_var->bInfo1 = comment[i+1];
              i++;
              break;
         case 4:                          /* register variable */
              i++;
              switch (comment[i]) {
              case 0x06: p = "si"; break;
              case 0x07: p = "di"; break;
              default  : p = "[unknown]"; break;
              }
              local_var->bInfo1 = comment[i];
              sprintf(temp," variable in register %s",p);
              i++;
              local_var->bInfo2 = comment[i];
              break;
         case 2:                        /* variable */
              local_var->bInfo1 = comment[i]; /* argument or local */
              sprintf(temp," (var) %s stored in ",comment[i]&8?"argument":"local");
              strcat(text,temp);
              switch (comment[i++]&7) {
              case 2 :                          /* stored on stack */
                   local_var->bInfo2 = (comment[i-1]&7)==2; /* stored on stack */
                   local_var->wInfo1 = *(int*)&comment[i]; /* address */
                   w = *(int*)&comment[i++];
                   if (w<0)
                     sprintf(temp,"[bp-%04X]",-w);
                   else
                     sprintf(temp,"[bp+%04X]",w);
                   break;
              default :
                   strcpy(temp,"[unknown]");
                   break;
              }
              i++;
              break;
         case 0:                  /* static function */
              sprintf(temp, " (static func) index %04X, word %04X", *(int*)&comment[i+1],*(int*)&comment[i+3]);
              i+=5;
         }
         strcat(text,temp);
         out_line(text,"","","");
       }
       break;

  case TCTYPDEF:
       sprintf(text, "; Type definition: index %02X ",*(int*)&comment[0]);
       i = 1;
       if (comment[1]) {
         sprintf(temp, "\"%.*s\", ",comment[1],&comment[2]); /* name */
         strcat(text,temp);
       }
       i+=comment[i]+1;
       sprintf(temp, "size %04X, TID %02X",*(int*)&comment[i],comment[i+2]);
       strcat(text,temp);
       out_line(text,"","","");
       break;

  case TCSOURCE :
       if (!*comment) {
         i = 1;
         sprintf(text, "; Source file %.*s",comment[1],&comment[2]);
         i+=comment[1]+1;
         sprint_date(temp,&comment[i]);
         out_line(text,temp,"","");
       } else {
         out_line("; Source file","","","");
       }
       break;

  case TCXLATOR :
       strcpy(text, "; Compiler: ");
       switch (*comment) {
       case 0x01: strcat(text,"'C'");       break;
       case 0x04: strcat(text,"Assembler"); break;
       default :  strcat(text,"[unknown]"); break;
       }
       sprintf(temp, " using %s model with underscores %s",
               models[comment[1]&7],comment[1]&8?"on":"off");
       strcat(text,temp);
       out_line(text,"","","");
       break;

  case TCENUM :
       i = 1;
       out_line("; ENUM member list","","","");
       while (i<length) {
         sprintf(text,"; %.*s",comment[i],&comment[i+1]);
         i+=comment[i]+1;
         sprintf(temp, " = %04X",*(word*)&comment[i]);
         i+=3;
         strcat(text,temp);
         out_line(text,"","","");
         out_newline();
       }
       break;

  case TCMANGLE:
       sprintf(temp, "; Mangled name \"%.*s\"",*comment, &comment[1]);
       out_line(temp,"","","");
       break;

  default:
       text[0] = '\0';
       prt_char = comment;
       while ( length ) {
           if ( isprint(*prt_char) ) {
               sprintf( temp, "%c", *prt_char );
           } else {
               sprintf( temp, "[%02X]", *prt_char );
           }
           strcat( text, temp );
           prt_char++;
           --length;
       }
       sprintf( temp, "; Unknown COMENT Record (Class %d): '%s'", class, text );
       out_line("", temp, "", "" );
       break;
  }
  out_newline();
  cksum = get_byte();
  cksum = cksum;
  type = type;
  if (comment!=junk)
    free(comment);
}
