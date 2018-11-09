#include <stdio.h>
#include "o.h"

INST_T  instr[] = {
    mod_reg,        0,  "add",          /* ADD     mem/reg,reg (byte) [00] */
    mod_reg,        1,  "add",          /* ADD     mem/reg,reg (word) [01] */
    mod_reg,        2,  "add",          /* ADD     reg,mem/reg (byte) [02] */
    mod_reg,        3,  "add",          /* ADD     reg,mem/reg (word) [03] */
    two_byte,      AL,  "add",          /* ADD     AL,kk              [04] */
    three_byte,    AX,  "add",          /* ADD     AX,jjkk            [05] */
    one_byte,      ES,  "push",         /* PUSH    ES                 [06] */
    one_byte,      ES,  "pop",          /* POP     ES                 [07] */
    mod_reg,        0,  "or",           /* OR      mem/reg,reg (byte) [08] */
    mod_reg,        1,  "or",           /* OR      mem/reg,reg (word) [09] */
    mod_reg,        2,  "or",           /* OR      reg,mem/reg (byte) [0A] */
    mod_reg,        3,  "or",           /* OR      reg,mem/reg (word) [0B] */
    two_byte,      AL,  "or",           /* OR      AL,kk              [0C] */
    three_byte,    AX,  "or",           /* OR      AX,jjkk            [0D] */
    one_byte,      CS,  "push",         /* PUSH    CS                 [0E] */
    extra,          0,  "",             /* not used                   [0F] */
    mod_reg,        0,  "adc",          /* ADC     mem/reg,reg (byte) [10] */
    mod_reg,        1,  "adc",          /* ADC     mem/reg,reg (word) [11] */
    mod_reg,        2,  "adc",          /* ADC     reg,mem/reg (byte) [12] */
    mod_reg,        3,  "adc",          /* ADC     reg,mem/reg (word) [13] */
    two_byte,      AL,  "adc",          /* ADC     AL,kk              [14] */
    three_byte,    AX,  "adc",          /* ADC     AX,jjkk            [15] */
    one_byte,      SS,  "push",         /* PUSH    SS                 [16] */
    one_byte,      SS,  "pop",          /* POP     SS                 [17] */
    mod_reg,        0,  "sbb",          /* SBB     mem/reg,reg (byte) [18] */
    mod_reg,        1,  "sbb",          /* SBB     mem/reg,reg (word) [19] */
    mod_reg,        2,  "sbb",          /* SBB     reg,mem/reg (byte) [1A] */
    mod_reg,        3,  "sbb",          /* SBB     reg,mem/reg (word) [1B] */
    two_byte,      AL,  "sbb",          /* SBB     AL,kk              [1C] */
    three_byte,    AX,  "sbb",          /* SBB     AX,jjkk            [1D] */
    one_byte,      DS,  "push",         /* PUSH    DS                 [1E] */
    one_byte,      DS,  "pop",          /* POP     DS                 [1F] */
    mod_reg,        0,  "and",          /* AND     mem/reg,reg (byte) [20] */
    mod_reg,        1,  "and",          /* AND     mem/reg,reg (word) [21] */
    mod_reg,        2,  "and",          /* AND     reg,mem/reg (byte) [22] */
    mod_reg,        3,  "and",          /* AND     reg,mem/reg (word) [23] */
    two_byte,      AL,  "and",          /* AND     AL,kk              [24] */
    three_byte,    AX,  "and",          /* AND     AX,jjkk            [25] */
    seg_over,       0,  "",             /* SEGMENT OVERIDE "ES"       [26] */
    one_byte,    NREG,  "daa",          /* DAA                        [27] */
    mod_reg,        0,  "sub",          /* SUB     mem/reg,reg (byte) [28] */
    mod_reg,        1,  "sub",          /* SUB     mem/reg,reg (word) [29] */
    mod_reg,        2,  "sub",          /* SUB     reg,mem/reg (byte) [2A] */
    mod_reg,        3,  "sub",          /* SUB     reg,mem/reg (word) [2B] */
    two_byte,      AL,  "sub",          /* SUB     AL,kk              [2C] */
    three_byte,    AX,  "sub",          /* SUB     AX,jjkk            [2D] */
    seg_over,       0,  "",             /* SEGMENT OVERIDE "CS"       [2E] */
    one_byte,    NREG,  "das",          /* DAS                        [2F] */
    mod_reg,        0,  "xor",          /* XOR     mem/reg,reg (byte) [30] */
    mod_reg,        1,  "xor",          /* XOR     mem/reg,reg (word) [31] */
    mod_reg,        2,  "xor",          /* XOR     reg,mem/reg (byte) [32] */
    mod_reg,        3,  "xor",          /* XOR     reg,mem/reg (word) [33] */
    two_byte,      AL,  "xor",          /* XOR     AL,kk              [34] */
    three_byte,    AX,  "xor",          /* XOR     AX,jjkk            [35] */
    seg_over,       0,  "",             /* SEGMENT OVERIDE "SS"       [36] */
    one_byte,    NREG,  "aaa",          /* AAA                        [37] */
    mod_reg,        0,  "cmp",          /* CMP     mem/reg,reg (byte) [38] */
    mod_reg,        1,  "cmp",          /* CMP     mem/reg,reg (word) [39] */
    mod_reg,        2,  "cmp",          /* CMP     reg,mem/reg (byte) [3A] */
    mod_reg,        3,  "cmp",          /* CMP     reg,mem/reg (word) [3B] */
    two_byte,      AL,  "cmp",          /* CMP     AL,kk              [3C] */
    three_byte,    AX,  "cmp",          /* CMP     AX,jjkk            [3D] */
    seg_over,       0,  "",             /* SEGMENT OVERIDE "DS"       [3E] */
    one_byte,    NREG,  "aas",          /* AAS                        [3F] */
    one_byte,      AX,  "inc",          /* INC     AX                 [40] */
    one_byte,      CX,  "inc",          /* INC     CX                 [41] */
    one_byte,      DX,  "inc",          /* INC     DX                 [42] */
    one_byte,      BX,  "inc",          /* INC     BX                 [43] */
    one_byte,      SP,  "inc",          /* INC     SP                 [44] */
    one_byte,      BP,  "inc",          /* INC     BP                 [45] */
    one_byte,      SI,  "inc",          /* INC     SI                 [46] */
    one_byte,      DI,  "inc",          /* INC     DI                 [47] */
    one_byte,      AX,  "dec",          /* DEC     AX                 [48] */
    one_byte,      CX,  "dec",          /* DEC     CX                 [49] */
    one_byte,      DX,  "dec",          /* DEC     DX                 [4A] */
    one_byte,      BX,  "dec",          /* DEC     BX                 [4B] */
    one_byte,      SP,  "dec",          /* DEC     SP                 [4C] */
    one_byte,      BP,  "dec",          /* DEC     BP                 [4D] */
    one_byte,      SI,  "dec",          /* DEC     SI                 [4E] */
    one_byte,      DI,  "dec",          /* DEC     DI                 [4F] */
    one_byte,      AX,  "push",         /* PUSH    AX                 [50] */
    one_byte,      CX,  "push",         /* PUSH    CX                 [51] */
    one_byte,      DX,  "push",         /* PUSH    DX                 [52] */
    one_byte,      BX,  "push",         /* PUSH    BX                 [53] */
    one_byte,      SP,  "push",         /* PUSH    SP                 [54] */
    one_byte,      BP,  "push",         /* PUSH    BP                 [55] */
    one_byte,      SI,  "push",         /* PUSH    SI                 [56] */
    one_byte,      DI,  "push",         /* PUSH    DI                 [57] */
    one_byte,      AX,  "pop",          /* POP     AX                 [58] */
    one_byte,      CX,  "pop",          /* POP     CX                 [59] */
    one_byte,      DX,  "pop",          /* POP     DX                 [5A] */
    one_byte,      BX,  "pop",          /* POP     BX                 [5B] */
    one_byte,      SP,  "pop",          /* POP     SP                 [5C] */
    one_byte,      BP,  "pop",          /* POP     BP                 [5D] */
    one_byte,      SI,  "pop",          /* POP     SI                 [5E] */
    one_byte,      DI,  "pop",          /* POP     DI                 [5F] */
    one_byte,    NREG,  "pusha",        /* PUSHA (PUSHAD on 386)      [60] */
    one_byte,    NREG,  "popa",         /* POPA (POPAD on 386)        [61] */
    mod_reg,       17,  "bound",        /* BOUND                      [62] */
    mod_reg,        1,  "arpl",         /* ARPL                       [63] */
    seg_over,       0,  "",             /* SEGMENT OVERIDE "FS" on 386[64] */
    seg_over,       0,  "",             /* SEGMENT OVERIDE "GS" on 386[65] */
    opsize_over,    0,  "",             /* OPERAND SIZE OVERIDE on 386[66] */
    adrsize_over,   0,  "",             /* ADDRESS SIZE OVERIDE on 386[67] */
    three_byte,  NREG,  "push",         /* PUSH (immediate word)      [68] */
    mod_reg,       24,  "imul",         /* IMUL reg,mem/reg,imm. word [69] */
    two_ubyte,      0,  "push\t",       /* PUSH (immed. signed byte)  [6A] */
    mod_reg,       25,  "imul",         /* IMUL reg,mem/reg,imm. byte [6B] */
    one_byte,    NREG,  "insb",         /* INS     BYTE               [6C] */
    one_byte,    NREG,  "insw",         /* INS     WORD               [6D] */
    one_byte,    NREG,  "outsb",        /* OUTS    BYTE               [6E] */
    one_byte,    NREG,  "outsw",        /* OUTS    WORD               [6F] */
    disp8,          0,  "jo",           /* JO      <Label>            [70] */
    disp8,          0,  "jno",          /* JNO     <Label>            [71] */
    disp8,          0,  "jc",           /* JC      <Label>            [72] */
    disp8,          0,  "jnc",          /* JNC     <Label>            [73] */
    disp8,          0,  "jz",           /* JZ      <Label>            [74] */
    disp8,          0,  "jnz",          /* JNZ     <Label>            [75] */
    disp8,          0,  "jbe",          /* JBE     <Label>            [76] */
    disp8,          0,  "jnbe",         /* JNBE    <Label>            [77] */
    disp8,          0,  "js",           /* JS      <Label>            [78] */
    disp8,          0,  "jns",          /* JNS     <Label>            [79] */
    disp8,          0,  "jp",           /* JP      <Label>            [7A] */
    disp8,          0,  "jnp",          /* JNP     <Label>            [7B] */
    disp8,          0,  "jl",           /* JL      <Label>            [7C] */
    disp8,          0,  "jnl",          /* JNL     <Label>            [7D] */
    disp8,          0,  "jng",          /* JG      <Label>            [7E] */
    disp8,          0,  "jg",           /* JNG     <Label>            [7F] */
    mod_reg,        4,  "",             /* Special mem/reg,kk         [80] */
    mod_reg,        5,  "",             /* Special mem/reg,jjkk       [81] */
    stub,           0,  "",             /* not used                   [82] */
    mod_reg,        6,  "",             /* Special mem/reg,kk (word)  [83] */
    mod_reg,        2,  "test",         /* TEST    mem/reg,reg (byte) [84] */
    mod_reg,        3,  "test",         /* TEST    mem/reg,reg (word) [85] */
    mod_reg,        2,  "xchg",         /* XCHG    reg,mem/reg (byte) [86] */
    mod_reg,        3,  "xchg",         /* XCHG    reg,mem/reg (word) [87] */
    mod_reg,        0,  "mov",          /* MOV     mem/reg,reg (byte) [88] */
    mod_reg,        1,  "mov",          /* MOV     mem/reg,reg (word) [89] */
    mod_reg,        2,  "mov",          /* MOV     reg,mem/reg (byte) [8A] */
    mod_reg,        3,  "mov",          /* MOV     reg,mem/reg (word) [8B] */
    mod_reg,        7,  "mov",          /* MOV     mem/reg,segreg     [8C] */
    mod_reg,        9,  "lea",          /* LEA     reg,mem/reg        [8D] */
    mod_reg,        8,  "mov",          /* MOV     segreg,mem/reg     [8E] */
    mod_reg,       10,  "",             /* POP     mem/reg (Special)  [8F] */
    one_byte,    NREG,  "nop",          /* NOP                        [90] */
    one_a,         CX,  "xchg",         /* XCHG    AX,CX              [91] */
    one_a,         DX,  "xchg",         /* XCHG    AX,DX              [92] */
    one_a,         BX,  "xchg",         /* XCHG    AX,BX              [93] */
    one_a,         SP,  "xchg",         /* XCHG    AX,SP              [94] */
    one_a,         BP,  "xchg",         /* XCHG    AX,BP              [95] */
    one_a,         SI,  "xchg",         /* XCHG    AX,SI              [96] */
    one_a,         DI,  "xchg",         /* XCHG    AX,DI              [97] */
    one_byte,    NREG,  "cbw",          /* CBW                        [98] */
    one_byte,    NREG,  "cwd",          /* CWD                        [99] */
    five_byte,      0,  "call",         /* CALL    <Label>            [9A] */
    wait,           0,  "wait",         /* WAIT                       [9B] */
    one_byte,    NREG,  "pushf",        /* PUSHF                      [9C] */
    one_byte,    NREG,  "popf",         /* POPF                       [9D] */
    one_byte,    NREG,  "sahf",         /* SAHF                       [9E] */
    one_byte,    NREG,  "lahf",         /* LAHF                       [9F] */
    three_a,        0,  "mov",          /* MOV     AL,addr            [A0] */
    three_a,        0,  "mov",          /* MOV     AX,addr            [A1] */
    three_a,        0,  "mov",          /* MOV     addr,AL            [A2] */
    three_a,        0,  "mov",          /* MOV     addr,AX            [A3] */
    string_byte,    0,  "movs",         /* MOVS    BYTE               [A4] */
    string_byte,    0,  "movs",         /* MOVS    WORD               [A5] */
    string_byte,    0,  "cmps",         /* CMPS    BYTE               [A6] */
    string_byte,    0,  "cmps",         /* CMPS    WORD               [A7] */
    two_byte,      AL,  "test",         /* TEST    AL,addr            [A8] */
    three_byte,    AX,  "test",         /* TEST    AX,addr            [A9] */
    string_byte,    0,  "stos",         /* STOS    BYTE               [AA] */
    string_byte,    0,  "stos",         /* STOS    WORD               [AB] */
    string_byte,    0,  "lods",         /* LODS    BYTE               [AC] */
    string_byte,    0,  "lods",         /* LODS    WORD               [AD] */
    string_byte,    0,  "scas",         /* SCAS    BYTE               [AE] */
    string_byte,    0,  "scas",         /* SCAS    WORD               [AF] */
    two_byte,      AL,  "mov",          /* MOV     AL,kk              [B0] */
    two_byte,      CL,  "mov",          /* MOV     CL,kk              [B1] */
    two_byte,      DL,  "mov",          /* MOV     DL,kk              [B2] */
    two_byte,      BL,  "mov",          /* MOV     BL,kk              [B3] */
    two_byte,      AH,  "mov",          /* MOV     AH,kk              [B4] */
    two_byte,      CH,  "mov",          /* MOV     CH,kk              [B5] */
    two_byte,      DH,  "mov",          /* MOV     DH,kk              [B6] */
    two_byte,      BH,  "mov",          /* MOV     BH,kk              [B7] */
    three_byte,    AX,  "mov",          /* MOV     AX,kk              [B8] */
    three_byte,    CX,  "mov",          /* MOV     CX,kk              [B9] */
    three_byte,    DX,  "mov",          /* MOV     DX,kk              [BA] */
    three_byte,    BX,  "mov",          /* MOV     BX,kk              [BB] */
    three_byte,    SP,  "mov",          /* MOV     SP,kk              [BC] */
    three_byte,    BP,  "mov",          /* MOV     BP,kk              [BD] */
    three_byte,    SI,  "mov",          /* MOV     SI,kk              [BE] */
    three_byte,    DI,  "mov",          /* MOV     DI,kk              [BF] */
    mod_reg,       11,  "",             /* Special mem/reg,kk  (byte) [C0] */
    mod_reg,       12,  "",             /* Special mem/reg,kk  (word) [C1] */
    three_byte,  NREG,  "ret",          /* RET     jjkk               [C2] */
    one_byte,    NREG,  "ret",          /* RET                        [C3] */
    mod_reg,       17,  "les",          /* LES     reg,mem            [C4] */
    mod_reg,       17,  "lds",          /* LDS     reg,mem            [C5] */
    mod_reg,       18,  "",             /* MOV     mem,kk   (Special) [C6] */
    mod_reg,       19,  "",             /* MOV     mem,jjkk (Special) [C7] */
    enter,          0,  "enter",        /* ENTER   im. word,im. byte  [C8] */
    one_byte,    NREG,  "leave",        /* LEAVE                      [C9] */
    three_byte,  NREG,  "retf",         /* RETF    jjkk               [CA] */
    one_byte,    NREG,  "retf",         /* RETF                       [CB] */
    one_byte,    NREG,  "int\t3",       /* INT     3                  [CC] */
    two_byte,    NREG,  "int",          /* INT     type               [CD] */
    one_byte,    NREG,  "into",         /* INTO                       [CE] */
    one_byte,    NREG,  "iret",         /* IRET                       [CF] */
    mod_reg,       13,  "",             /* Special mem/reg,1  (byte)  [D0] */
    mod_reg,       14,  "",             /* Special mem/reg,1  (word)  [D1] */
    mod_reg,       15,  "",             /* Special mem/reg,CL (byte)  [D2] */
    mod_reg,       16,  "",             /* Special mem/reg,CL (word)  [D3] */
    two_bcd,        0,  "aam",          /* AAM                        [D4] */
    two_bcd,        0,  "aad",          /* AAD                        [D5] */
    stub,           0,  "",             /* not used                   [D6] */
    one_byte,    NREG,  "xlat",         /* XLAT                       [D7] */
    esc,            0,  "esc",          /* ESC  (Special)             [D8] */
    esc,            0,  "esc",          /* ESC  (Special)             [D9] */
    esc,            0,  "esc",          /* ESC  (Special)             [DA] */
    esc,            0,  "esc",          /* ESC  (Special)             [DB] */
    esc,            0,  "esc",          /* ESC  (Special)             [DC] */
    esc,            0,  "esc",          /* ESC  (Special)             [DD] */
    esc,            0,  "esc",          /* ESC  (Special)             [DE] */
    esc,            0,  "esc",          /* ESC  (Special)             [DF] */
    disp8,          0,  "loopnz",       /* LOOPNZ                     [E0] */
    disp8,          0,  "loopz",        /* LOOPZ                      [E1] */
    disp8,          0,  "loop",         /* LOOP                       [E2] */
    disp8,          0,  "jcxz",         /* JCXZ                       [E3] */
    two_byte,      AL,  "in",           /* IN      AL,kk              [E4] */
    two_byte,      AX,  "in",           /* IN      AX,kk              [E5] */
    two_a,          0,  "out",          /* OUT     kk,AL              [E6] */
    two_a,          0,  "out",          /* OUT     kk,AX              [E7] */
    disp16,         0,  "call",         /* CALL    <Label>            [E8] */
    disp16,         0,  "jmp",          /* JMP     <Label>            [E9] */
    five_byte,      0,  "jmp",          /* JMP     <Label>            [EA] */
    disp8,          0,  "jmp",          /* JMP     SHORT <Label>      [EB] */
    in_out,        DX,  "in",           /* IN      AL,DX              [EC] */
    in_out,        DX,  "in",           /* IN      AX,DX              [ED] */
    in_out,        DX,  "out",          /* OUT     DX,AL              [EE] */
    in_out,        DX,  "out",          /* OUT     DX,AX              [EF] */
    prefix,         0,  "lock",         /* LOCK                       [F0] */
    stub,           0,  "",             /* not used                   [F1] */
    prefix,         0,  "repnz",        /* REPNZ                      [F2] */
    prefix,         0,  "repz",         /* REPZ                       [F3] */
    one_byte,       0,  "hlt",          /* HLT                        [F4] */
    one_byte,       0,  "cmc",          /* CMC                        [F5] */
    mod_reg,       20,  "",             /* Special (byte) (Group 3)   [F6] */
    mod_reg,       21,  "",             /* Special (word) (Group 3)   [F7] */
    one_byte,       0,  "clc",          /* CLC                        [F8] */
    one_byte,       0,  "stc",          /* STC                        [F9] */
    one_byte,       0,  "cli",          /* CLI                        [FA] */
    one_byte,       0,  "sti",          /* STI                        [FB] */
    one_byte,       0,  "cld",          /* CLD                        [FC] */
    one_byte,       0,  "std",          /* STD                        [FD] */
    mod_reg,       22,  "",             /* Special (Group 4)          [FE] */
    mod_reg,       23,  ""         };   /* Special (Group 4)          [FF] */

char *esc_inst[] = {
    "fadd",         /* FADD (DWord Ptr)   [00] */
    "fmul",         /* FMUL (DWord Ptr)   [01] */
    "fcom",         /* FCOM (DWord Ptr)   [02] */
    "fcomp",        /* FCOMP (DWord Ptr)  [03] */
    "fsub",         /* FSUB (DWord Ptr)   [04] */
    "fsubr",        /* FSUBR (DWord Ptr)  [05] */
    "fdiv",         /* FDIV (DWord Ptr)   [06] */
    "fdivr",        /* FDIVR (DWord Ptr)  [07] */
    "fld",          /* FLD (DWord Ptr)    [08] */
    "",             /* ESC 09h            [09] */
    "fst",          /* FST (DWord Ptr)    [0A] */
    "fstp",         /* FSTP (DWord Ptr)   [0B] */
    "fldenv",       /* FLDENV             [0C] */
    "fldcw",        /* FLDCW              [0D] */
    "fnstenv",      /* FSTENV             [0E] */
    "fnstcw",       /* FSTCW              [0F] */
    "fiadd",        /* FIADD (DWord Ptr)  [10] */
    "fimul",        /* FIMUL (DWord Ptr)  [11] */
    "ficom",        /* FICOM (DWord Ptr)  [12] */
    "ficomp",       /* FICOMP (DWord Ptr) [13] */
    "fisub",        /* FISUB (DWord Ptr)  [14] */
    "fisubr",       /* FISUBR (DWord Ptr) [15] */
    "fidiv",        /* FIDIV (DWord Ptr)  [16] */
    "fidivr",       /* FIDIVR (DWord Ptr) [17] */
    "fild",         /* FILD (DWord Ptr)   [18] */
    "",             /* ESC 19h            [19] */
    "fist",         /* FIST (DWord Ptr)   [1A] */
    "fistp",        /* FISTP (DWord Ptr)  [1B] */
    "",             /* ESC 1Ch            [1C] */
    "fld",          /* FLD (TByte Ptr)    [1D] */
    "",             /* ESC 1Eh            [1E] */
    "fstp",         /* FSTP (TByte Ptr)   [1F] */
    "fadd",         /* FADD (QWord Ptr)   [20] */
    "fmul",         /* FMUL (QWord Ptr)   [21] */
    "fcom",         /* FCOM (QWord Ptr)   [22] */
    "fcomp",        /* FCOMP (QWord Ptr)  [23] */
    "fsub",         /* FSUB (QWord Ptr)   [24] */
    "fsubr",        /* FSUBR (QWord Ptr)  [25] */
    "fdiv",         /* FDIV (QWord Ptr)   [26] */
    "fdivr",        /* FDIVR (QWord Ptr)  [27] */
    "fld",          /* FLD (QWord Ptr)    [28] */
    "",             /* ESC 29h            [29] */
    "fst",          /* FST (QWord Ptr)    [2A] */
    "fstp",         /* FSTP (QWord Ptr)   [2B] */
    "frstor",       /* FRSTOR             [2C] */
    "",             /* ESC 2Dh            [2D] */
    "fnsave",       /* FSAVE              [2E] */
    "fnstsw",       /* FSTSW              [2F] */
    "fiadd",        /* FIADD (Word Ptr)   [30] */
    "fimul",        /* FIMUL (Word Ptr)   [31] */
    "ficom",        /* FICOM (Word Ptr)   [32] */
    "ficomp",       /* FICOMP (Word Ptr)  [33] */
    "fisub",        /* FISUB (Word Ptr)   [34] */
    "fisubr",       /* FISUBR (Word Ptr)  [35] */
    "fidiv",        /* FIDIV (Word Ptr)   [36] */
    "fidivr",       /* FIDIVR (Word Ptr)  [37] */
    "fild",         /* FILD (Word Ptr)    [38] */
    "",             /* ESC 39h            [39] */
    "fist",         /* FIST (Word Ptr)    [3A] */
    "fistp",        /* FISTP (Word Ptr)   [3B] */
    "fbld",         /* FBLD (TByte Ptr)   [3C] */
    "fild",         /* FILD (QWord Ptr)   [3D] */
    "fbstp",        /* FBSTP (QWord Ptr)  [3E] */
    "fistp",   };   /* FISTP (QWord Ptr)  [3F] */

INST_T  ex_instr[] = {
    mod_reg,       26,  "",             /* Spec. (word) (grp 6)   [0F] [00] */
    mod_reg,       27,  "",             /* Spec. (dword) (grp 7)  [0F] [01] */
    mod_reg,       28,  "lar",          /* LAR                    [0F] [02] */
    mod_reg,       28,  "lsl",          /* LSL                    [0F] [03] */
    stub,           0,  "",             /* not used               [0F] [04] */
    stub,           0,  "",             /* not used               [0F] [05] */
    one_byte,    NREG,  "clts",         /* not used               [0F] [06] */
    stub,           0,  "",             /* not used               [0F] [07] */
    one_byte,    NREG,  "",             /* INVD (486 only)        [0F] [08] */
    one_byte,    NREG,  "",             /* WBINVD (486 only)      [0F] [09] */
    stub,           0,  "",             /* not used               [0F] [0A] */
    stub,           0,  "",             /* not used               [0F] [0B] */
    stub,           0,  "",             /* not used               [0F] [0C] */
    stub,           0,  "",             /* not used               [0F] [0D] */
    stub,           0,  "",             /* not used               [0F] [0E] */
    stub,           0,  "",             /* not used               [0F] [0F] */
    stub,           0,  "",             /* not used               [0F] [10] */
    stub,           0,  "",             /* not used               [0F] [11] */
    stub,           0,  "",             /* not used               [0F] [12] */
    stub,           0,  "",             /* not used               [0F] [13] */
    stub,           0,  "",             /* not used               [0F] [14] */
    stub,           0,  "",             /* not used               [0F] [15] */
    stub,           0,  "",             /* not used               [0F] [16] */
    stub,           0,  "",             /* not used               [0F] [17] */
    stub,           0,  "",             /* not used               [0F] [18] */
    stub,           0,  "",             /* not used               [0F] [19] */
    stub,           0,  "",             /* not used               [0F] [1A] */
    stub,           0,  "",             /* not used               [0F] [1B] */
    stub,           0,  "",             /* not used               [0F] [1C] */
    stub,           0,  "",             /* not used               [0F] [1D] */
    stub,           0,  "",             /* not used               [0F] [1E] */
    stub,           0,  "",             /* not used               [0F] [1F] */
    mod_reg,       31,  "mov",          /* MOV      r32,CRx       [0F] [20] */
    mod_reg,       32,  "mov",          /* MOV      CRx,r32       [0F] [21] */
    mod_reg,       33,  "mov",          /* MOV      r32,DRx       [0F] [22] */
    mod_reg,       34,  "mov",          /* MOV      DRx,r32       [0F] [23] */
    mod_reg,       35,  "mov",          /* MOV      r32,TRx       [0F] [24] */
    stub,           0,  "",             /* not used               [0F] [25] */
    mod_reg,       36,  "mov",          /* MOV      TRx,r32       [0F] [26] */
    stub,           0,  "",             /* not used               [0F] [27] */
    stub,           0,  "",             /* not used               [0F] [28] */
    stub,           0,  "",             /* not used               [0F] [29] */
    stub,           0,  "",             /* not used               [0F] [2A] */
    stub,           0,  "",             /* not used               [0F] [2B] */
    stub,           0,  "",             /* not used               [0F] [2C] */
    stub,           0,  "",             /* not used               [0F] [2D] */
    stub,           0,  "",             /* not used               [0F] [2E] */
    stub,           0,  "",             /* not used               [0F] [2F] */
    stub,           0,  "",             /* not used               [0F] [30] */
    stub,           0,  "",             /* not used               [0F] [31] */
    stub,           0,  "",             /* not used               [0F] [32] */
    stub,           0,  "",             /* not used               [0F] [33] */
    stub,           0,  "",             /* not used               [0F] [34] */
    stub,           0,  "",             /* not used               [0F] [05] */
    stub,           0,  "",             /* not used               [0F] [36] */
    stub,           0,  "",             /* not used               [0F] [37] */
    stub,           0,  "",             /* not used               [0F] [38] */
    stub,           0,  "",             /* not used               [0F] [39] */
    stub,           0,  "",             /* not used               [0F] [3A] */
    stub,           0,  "",             /* not used               [0F] [3B] */
    stub,           0,  "",             /* not used               [0F] [3C] */
    stub,           0,  "",             /* not used               [0F] [3D] */
    stub,           0,  "",             /* not used               [0F] [3E] */
    stub,           0,  "",             /* not used               [0F] [3F] */
    stub,           0,  "",             /* not used               [0F] [40] */
    stub,           0,  "",             /* not used               [0F] [41] */
    stub,           0,  "",             /* not used               [0F] [42] */
    stub,           0,  "",             /* not used               [0F] [43] */
    stub,           0,  "",             /* not used               [0F] [44] */
    stub,           0,  "",             /* not used               [0F] [45] */
    stub,           0,  "",             /* not used               [0F] [46] */
    stub,           0,  "",             /* not used               [0F] [47] */
    stub,           0,  "",             /* not used               [0F] [48] */
    stub,           0,  "",             /* not used               [0F] [49] */
    stub,           0,  "",             /* not used               [0F] [4A] */
    stub,           0,  "",             /* not used               [0F] [4B] */
    stub,           0,  "",             /* not used               [0F] [4C] */
    stub,           0,  "",             /* not used               [0F] [4D] */
    stub,           0,  "",             /* not used               [0F] [4E] */
    stub,           0,  "",             /* not used               [0F] [4F] */
    stub,           0,  "",             /* not used               [0F] [50] */
    stub,           0,  "",             /* not used               [0F] [51] */
    stub,           0,  "",             /* not used               [0F] [52] */
    stub,           0,  "",             /* not used               [0F] [53] */
    stub,           0,  "",             /* not used               [0F] [54] */
    stub,           0,  "",             /* not used               [0F] [55] */
    stub,           0,  "",             /* not used               [0F] [56] */
    stub,           0,  "",             /* not used               [0F] [57] */
    stub,           0,  "",             /* not used               [0F] [58] */
    stub,           0,  "",             /* not used               [0F] [59] */
    stub,           0,  "",             /* not used               [0F] [5A] */
    stub,           0,  "",             /* not used               [0F] [5B] */
    stub,           0,  "",             /* not used               [0F] [5C] */
    stub,           0,  "",             /* not used               [0F] [5D] */
    stub,           0,  "",             /* not used               [0F] [5E] */
    stub,           0,  "",             /* not used               [0F] [5F] */
    stub,           0,  "",             /* not used               [0F] [60] */
    stub,           0,  "",             /* not used               [0F] [61] */
    stub,           0,  "",             /* not used               [0F] [62] */
    stub,           0,  "",             /* not used               [0F] [63] */
    stub,           0,  "",             /* not used               [0F] [64] */
    stub,           0,  "",             /* not used               [0F] [65] */
    stub,           0,  "",             /* not used               [0F] [66] */
    stub,           0,  "",             /* not used               [0F] [67] */
    stub,           0,  "",             /* not used               [0F] [68] */
    stub,           0,  "",             /* not used               [0F] [69] */
    stub,           0,  "",             /* not used               [0F] [6A] */
    stub,           0,  "",             /* not used               [0F] [6B] */
    stub,           0,  "",             /* not used               [0F] [6C] */
    stub,           0,  "",             /* not used               [0F] [6D] */
    stub,           0,  "",             /* not used               [0F] [6E] */
    stub,           0,  "",             /* not used               [0F] [6F] */
    stub,           0,  "",             /* not used               [0F] [70] */
    stub,           0,  "",             /* not used               [0F] [71] */
    stub,           0,  "",             /* not used               [0F] [72] */
    stub,           0,  "",             /* not used               [0F] [73] */
    stub,           0,  "",             /* not used               [0F] [74] */
    stub,           0,  "",             /* not used               [0F] [75] */
    stub,           0,  "",             /* not used               [0F] [76] */
    stub,           0,  "",             /* not used               [0F] [77] */
    stub,           0,  "",             /* not used               [0F] [78] */
    stub,           0,  "",             /* not used               [0F] [79] */
    stub,           0,  "",             /* not used               [0F] [7A] */
    stub,           0,  "",             /* not used               [0F] [7B] */
    stub,           0,  "",             /* not used               [0F] [7C] */
    stub,           0,  "",             /* not used               [0F] [7D] */
    stub,           0,  "",             /* not used               [0F] [7E] */
    stub,           0,  "",             /* not used               [0F] [7F] */
    disp16,         0,  "jo",           /* JO       <Label>       [0F] [80] */
    disp16,         0,  "jno",          /* JNO      <Label>       [0F] [81] */
    disp16,         0,  "jb",           /* JB       <Label>       [0F] [82] */
    disp16,         0,  "jnb",          /* JNB      <Label>       [0F] [83] */
    disp16,         0,  "jz",           /* JO       <Label>       [0F] [84] */
    disp16,         0,  "jnz",          /* JNO      <Label>       [0F] [85] */
    disp16,         0,  "jbe",          /* JBE      <Label>       [0F] [86] */
    disp16,         0,  "jnbe",         /* JNBE     <Label>       [0F] [87] */
    disp16,         0,  "js",           /* JS       <Label>       [0F] [88] */
    disp16,         0,  "jns",          /* JNS      <Label>       [0F] [89] */
    disp16,         0,  "jp",           /* JP       <Label>       [0F] [8A] */
    disp16,         0,  "jnp",          /* JNP      <Label>       [0F] [8B] */
    disp16,         0,  "jl",           /* JL       <Label>       [0F] [8C] */
    disp16,         0,  "jnl",          /* JNL      <Label>       [0F] [8D] */
    disp16,         0,  "jle",          /* JLE      <Label>       [0F] [8E] */
    disp16,         0,  "jnle",         /* JNLE     <Label>       [0F] [8F] */
    mod_reg,       30,  "seto",         /* SETO     mem/reg       [0F] [90] */
    mod_reg,       30,  "setno",        /* SETNO    mem/reg       [0F] [91] */
    mod_reg,       30,  "setb",         /* SETB     mem/reg       [0F] [92] */
    mod_reg,       30,  "setnb",        /* SETNB    mem/reg       [0F] [93] */
    mod_reg,       30,  "setz",         /* SETZ     mem/reg       [0F] [94] */
    mod_reg,       30,  "setnz",        /* SETNZ    mem/reg       [0F] [95] */
    mod_reg,       30,  "setbe",        /* SETNE    mem/reg       [0F] [96] */
    mod_reg,       30,  "setnbe",       /* SETNBE   mem/reg       [0F] [97] */
    mod_reg,       30,  "sets",         /* SETS     mem/reg       [0F] [98] */
    mod_reg,       30,  "setns",        /* SETNS    mem/reg       [0F] [99] */
    mod_reg,       30,  "setp",         /* SETP     mem/reg       [0F] [9A] */
    mod_reg,       30,  "setnp",        /* SETNP    mem/reg       [0F] [9B] */
    mod_reg,       30,  "setl",         /* SETL     mem/reg       [0F] [9C] */
    mod_reg,       30,  "setnl",        /* SETNL    mem/reg       [0F] [9D] */
    mod_reg,       30,  "setle",        /* SETLE    mem/reg       [0F] [9E] */
    mod_reg,       30,  "setnle",       /* SETNLE   mem/reg       [0F] [9F] */
    one_byte,      FS,  "push",         /* PUSH FS                [0F] [A0] */
    one_byte,      FS,  "pop",          /* POP FS                 [0F] [A1] */
    stub,           0,  "",             /* not used               [0F] [A2] */
    mod_reg,        1,  "bt",           /* BT       mem/reg,reg   [0F] [A3] */
    mod_reg,       39,  "shld",         /* SHLD     mem,reg,immb  [0F] [A4] */
    mod_reg,       40,  "shld",         /* SHLD     mem,reg,cl    [0F] [A5] */
    mod_reg,        0,  "",             /* CMPXCHG                [0F] [A6] */
    mod_reg,        1,  "",             /* CMPXCHG                [0F] [A7] */
    one_byte,      GS,  "push",         /* PUSH GS                [0F] [A8] */
    one_byte,      GS,  "pop",          /* POP GS                 [0F] [A9] */
    stub,           0,  "",             /* not used               [0F] [AA] */
    mod_reg,        1,  "bts",          /* BTS      mem/reg,reg   [0F] [AB] */
    mod_reg,       39,  "shrd",         /* SHRD     mem,reg,immb  [0F] [AC] */
    mod_reg,       40,  "shrd",         /* SHRD     mem,reg,cl    [0F] [AD] */
    stub,           0,  "",             /* not used               [0F] [AE] */
    mod_reg,        3,  "imul",         /* IMUL     reg,mem/reg   [0F] [AF] */
    stub,           0,  "",             /* not used               [0F] [B0] */
    stub,           0,  "",             /* not used               [0F] [B1] */
    mod_reg,       17,  "lss",          /* LSS      reg,mem       [0F] [B2] */
    mod_reg,        1,  "btr",          /* BTR      mem/reg,reg   [0F] [B3] */
    mod_reg,       17,  "lfs",          /* LFS      reg,mem       [0F] [B4] */
    mod_reg,       17,  "lgs",          /* LGS      reg,mem       [0F] [B5] */
    mod_reg,       37,  "movzx",        /* MOVZX    reg,mem (b)   [0F] [B6] */
    mod_reg,       38,  "movzx",        /* MOVZX    reg,mem (w)   [0F] [B7] */
    stub,           0,  "",             /* not used               [0F] [B8] */
    stub,           0,  "",             /* not used               [0F] [B9] */
    mod_reg,       29,  "",             /* Spec. (byte) (Group 8) [0F] [BA] */
    mod_reg,        1,  "btc",          /* BTC      mem/reg,reg   [0F] [BB] */
    mod_reg,        3,  "bsf",          /* BSF      reg,mem/reg   [0F] [BC] */
    mod_reg,        3,  "bsr",          /* BSR      reg,mem/reg   [0F] [BD] */
    mod_reg,       37,  "movsx",        /* MOVSX    reg,mem (b)   [0F] [BE] */
    mod_reg,       38,  "movsx",        /* MOVSX    reg,mem (w)   [0F] [BF] */
    mod_reg,        0,  "",             /* XADD (486 only)        [0F] [C0] */
    mod_reg,        1,  "",             /* XADD (486 only)        [0F] [C1] */
    stub,           0,  "",             /* not used               [0F] [C2] */
    stub,           0,  "",             /* not used               [0F] [C3] */
    stub,           0,  "",             /* not used               [0F] [C4] */
    stub,           0,  "",             /* not used               [0F] [C5] */
    stub,           0,  "",             /* not used               [0F] [C6] */
    stub,           0,  "",             /* not used               [0F] [C7] */
    one_byte,      AX,  "",             /* BSWAP AX (486 only)    [0F] [C8] */
    one_byte,      CX,  "",             /* BSWAP CX (486 only)    [0F] [C9] */
    one_byte,      DX,  "",             /* BSWAP DX (486 only)    [0F] [CA] */
    one_byte,      BX,  "",             /* BSWAP BX (486 only)    [0F] [CB] */
    one_byte,      SP,  "",             /* BSWAP SP (486 only)    [0F] [CC] */
    one_byte,      BP,  "",             /* BSWAP BP (486 only)    [0F] [CD] */
    one_byte,      SI,  "",             /* BSWAP SI (486 only)    [0F] [CE] */
    one_byte,      DI,  "",             /* BSWAP DI (486 only)    [0F] [CF] */
    stub,           0,  "",             /* not used               [0F] [D0] */
    stub,           0,  "",             /* not used               [0F] [D1] */
    stub,           0,  "",             /* not used               [0F] [D2] */
    stub,           0,  "",             /* not used               [0F] [D3] */
    stub,           0,  "",             /* not used               [0F] [D4] */
    stub,           0,  "",             /* not used               [0F] [D5] */
    stub,           0,  "",             /* not used               [0F] [D6] */
    stub,           0,  "",             /* not used               [0F] [D7] */
    stub,           0,  "",             /* not used               [0F] [D8] */
    stub,           0,  "",             /* not used               [0F] [D9] */
    stub,           0,  "",             /* not used               [0F] [DA] */
    stub,           0,  "",             /* not used               [0F] [DB] */
    stub,           0,  "",             /* not used               [0F] [DC] */
    stub,           0,  "",             /* not used               [0F] [DD] */
    stub,           0,  "",             /* not used               [0F] [DE] */
    stub,           0,  "",             /* not used               [0F] [DF] */
    stub,           0,  "",             /* not used               [0F] [E0] */
    stub,           0,  "",             /* not used               [0F] [E1] */
    stub,           0,  "",             /* not used               [0F] [E2] */
    stub,           0,  "",             /* not used               [0F] [E3] */
    stub,           0,  "",             /* not used               [0F] [E4] */
    stub,           0,  "",             /* not used               [0F] [E5] */
    stub,           0,  "",             /* not used               [0F] [E6] */
    stub,           0,  "",             /* not used               [0F] [E7] */
    stub,           0,  "",             /* not used               [0F] [E8] */
    stub,           0,  "",             /* not used               [0F] [E9] */
    stub,           0,  "",             /* not used               [0F] [EA] */
    stub,           0,  "",             /* not used               [0F] [EB] */
    stub,           0,  "",             /* not used               [0F] [EC] */
    stub,           0,  "",             /* not used               [0F] [ED] */
    stub,           0,  "",             /* not used               [0F] [EE] */
    stub,           0,  "",             /* not used               [0F] [EF] */
    stub,           0,  "",             /* not used               [0F] [F0] */
    stub,           0,  "",             /* not used               [0F] [F1] */
    stub,           0,  "",             /* not used               [0F] [F2] */
    stub,           0,  "",             /* not used               [0F] [F3] */
    stub,           0,  "",             /* not used               [0F] [F4] */
    stub,           0,  "",             /* not used               [0F] [F5] */
    stub,           0,  "",             /* not used               [0F] [F6] */
    stub,           0,  "",             /* not used               [0F] [F7] */
    stub,           0,  "",             /* not used               [0F] [F8] */
    stub,           0,  "",             /* not used               [0F] [F9] */
    stub,           0,  "",             /* not used               [0F] [FA] */
    stub,           0,  "",             /* not used               [0F] [FB] */
    stub,           0,  "",             /* not used               [0F] [FC] */
    stub,           0,  "",             /* not used               [0F] [FD] */
    stub,           0,  "",             /* not used               [0F] [FE] */
    stub,           0,  "",             /* not used               [0F] [FF] */
};
