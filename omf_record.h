#ifndef OMF_RECORD_H
#define OMF_RECORD_H

                                    /*--- Expanded INTEL OMF record types --*/
#define RHEADR  0x6E                /*   R-Module Header Record             */
#define REGINT  0x70                /*   Register Initialization Record     */
#define REDATA  0x72                /*   Relocatable Enumerated Data Record */
#define RIDATA  0x74                /*   Relocatable Iterated Data Record   */
#define OVLDEF  0x76                /*   Overlay Definition Record          */
#define ENDREC  0x78                /*   End Record                         */
#define BLKREC  0x7A                /*   Block Definition Record            */
#define BKLEND  0x7C                /*   Block End Record                   */
#define DEBSYM  0x7E                /*   Debug Symbols Record               */
#define THEADR  0x80                /* x T-Module Header Record             */
#define LHEADR  0x82                /*   L-Module Header Record             */
#define PEDATA  0x84                /*   Physical Enumerated Data (?)       */
#define PIDATA  0x86                /*   Physical Iterated Data (?)         */
#define COMENT  0x88                /* x Comment Record                     */
#define MODEND  0x8A                /* x Module End Record                  */
#define MODENDL 0x8B                /* l Module End Record                  */
#define EXTDEF  0x8C                /* x External Names Definition Record   */
#define TYPDEF  0x8E                /*   Type Definitions Record            */
#define PUBDEF  0x90                /* x Public Names Definition Record     */
#define PUBDEFL 0x91                /* l Public Names Definition Record     */
#define LOCSYM  0x92                /*   Local Symbols Record               */
#define LINNUM  0x94                /*   Line Numbers Record                */
#define LNAMES  0x96                /* x List of Names Record               */
#define SEGDEF  0x98                /* x Segment Definition Record          */
#define SEGDEFL 0x99                /* l Segment Definition Record          */
#define GRPDEF  0x9A                /* x Group Definition Record            */
#define FIXUPP  0x9C                /* x Fix-Up Record                      */
#define FIXUPPL 0x9D                /* l Fix-Up Record                      */
#define LEDATA  0xA0                /* x Logical Enumerated Data            */
#define LEDATAL 0xA1                /* l Logical Enumerated Data            */
#define LIDATA  0xA2                /* x Logical Iterated Data              */
#define LIDATAL 0xA3                /* l Logical Iterated Data              */
#define LIBHED  0xA4                /*   Library Header Record              */
#define LIBNAM  0xA6                /*   Library Module Names Record        */
#define LIBLOC  0xA8                /*   Library Module Locations Record    */
#define LIBDIC  0xAA                /*   Library Dictionary Record          */
#define COMDEF  0xB0                /* m Communal Data Definition Record    */
#define LEXTDEF 0xB4                /* m Local External Definition          */
#define LPUBDEF 0xB6                /* m Local Public Definition            */
#define LPUBDF2 0xB7                /* m Local Public Definition (2nd case?)*/
#define LCOMDEF 0xB8                /* m Local Communal Data Definition     */
#define LIBHDR  0xF0                /* m Library Header Record              */
#define LIBEND  0xF1                /* m Library Trailer Record             */
                                    /* x = Intel OMF used by Microsoft      */
                                    /* m = Microsoft Additions to Intel OMF */
                                    /* l = Later extensions by Microsoft    */

// Records routine

void theadr(void);                         // ortheadr.c
void segdef(void);                         // orsegdef.c
void lnames(size_t length);                // orlnames.c
void grpdef(size_t length);                // orgrpdef.c
void coment(size_t length);                // orcoment.c
void linnum(size_t length);                // orlinnum.c
void comdef(size_t length, int scope);     // orcomdef.c
void pubdef(size_t length, int scope);     // orpubdef.c
void extdef(size_t length, int scope);     // orextdef.c
void ledata(size_t length, int extension); // orledata.c
void lidata(size_t length, int extension); // orlidata.c
void fixupp(size_t length, int extension); // orfixupp.c
void modend(size_t length, int extension); // ormodend.c

#endif // OMF_RECORD_H
