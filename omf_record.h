#ifndef OMF_RECORD_H
#define OMF_RECORD_H

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
