// src/cpu.c
#include "gb.h"
#include <stdlib.h>

typedef struct {
    void (*func)(GameBoy* gb); // Puntero a la función que implementa la instrucción
    char* name;               // Nombre de la instrucción (para debugging)
    u8 cycles;                 // Número de M-Cycles que consume la instrucción
    u8 length;                 // Longitud en bytes de la instrucción
} Instruction;

// Declaraciones de funciones (prototipos)
void op_nop(GameBoy* gb);
void op_ld_r_r(GameBoy* gb);
void op_ld_r_d8(GameBoy* gb);
void op_ld_rr_d16(GameBoy* gb);
void op_halt(GameBoy* gb);
void op_inc_r(GameBoy* gb);
void op_dec_r(GameBoy* gb);
void op_add_addc(GameBoy* gb);
void op_sub_sbc_a_r(GameBoy* gb);
void op_and_a_r(GameBoy* gb);
void op_xor_a_r(GameBoy* gb);

// ... otras declaraciones de instrucciones ...

// Tabla de instrucciones (completa con todas las instrucciones)
Instruction instruction_set[256] = {
    [0x00] = { .func = op_nop, .name = "NOP", .cycles = 1, .length = 1 },
    [0x01] = { .func = op_ld_rr_d16, .name = "LD BC,d16", .cycles = 3, .length = 3 },
    [0x02] = { .func = NULL, .name = "LD (BC),A", .cycles = 2, .length = 1 },
    [0x03] = { .func = NULL, .name = "INC BC", .cycles = 2, .length = 1 },
    [0x04] = { .func = op_inc_r, .name = "INC B", .cycles = 1, .length = 1 },
    [0x05] = { .func = op_dec_r, .name = "DEC B", .cycles = 1, .length = 1 },
    [0x06] = { .func = op_ld_r_d8, .name = "LD B,d8", .cycles = 2, .length = 2 },
    [0x07] = { .func = NULL, .name = "RLCA", .cycles = 1, .length = 1 },
    [0x08] = { .func = NULL, .name = "LD (a16),SP", .cycles = 5, .length = 3 },
    [0x09] = { .func = NULL, .name = "ADD HL,BC", .cycles = 2, .length = 1 },
    [0x0A] = { .func = NULL, .name = "LD A,(BC)", .cycles = 2, .length = 1 },
    [0x0B] = { .func = NULL, .name = "DEC BC", .cycles = 2, .length = 1 },
    [0x0C] = { .func = op_inc_r, .name = "INC C", .cycles = 1, .length = 1 },
    [0x0D] = { .func = op_dec_r, .name = "DEC C", .cycles = 1, .length = 1 },
    [0x0E] = { .func = NULL, .name = "LD C,d8", .cycles = 2, .length = 2 },
    [0x0F] = { .func = NULL, .name = "RRCA", .cycles = 1, .length = 1 },
    [0x10] = { .func = NULL, .name = "STOP", .cycles = 2, .length = 2 },
    [0x11] = { .func = op_ld_rr_d16, .name = "LD DE,d16", .cycles = 3, .length = 3 },
    [0x12] = { .func = NULL, .name = "LD (DE),A", .cycles = 2, .length = 1 },
    [0x13] = { .func = NULL, .name = "INC DE", .cycles = 2, .length = 1 },
    [0x14] = { .func = op_inc_r, .name = "INC D", .cycles = 1, .length = 1 },
    [0x15] = { .func = op_dec_r, .name = "DEC D", .cycles = 1, .length = 1 },
    [0x16] = { .func = op_ld_r_d8, .name = "LD D,d8", .cycles = 2, .length = 2 },
    [0x17] = { .func = NULL, .name = "RLA", .cycles = 1, .length = 1 },
    [0x18] = { .func = NULL, .name = "JR r8", .cycles = 3, .length = 2 },
    [0x19] = { .func = NULL, .name = "ADD HL,DE", .cycles = 2, .length = 1 },
    [0x1A] = { .func = NULL, .name = "LD A,(DE)", .cycles = 2, .length = 1 },
    [0x1B] = { .func = NULL, .name = "DEC DE", .cycles = 2, .length = 1 },
    [0x1C] = { .func = op_inc_r, .name = "INC E", .cycles = 1, .length = 1 },
    [0x1D] = { .func= op_dec_r, .name = "DEC E", .cycles = 1, .length = 1 },
    [0x1E] = { .func= NULL, .name = "LD E,d8", .cycles = 2, .length = 2 },
    [0x1F] = { .func= NULL, .name = "RRA", .cycles = 1, .length = 1 },
    [0x20] = { .func = NULL, .name = "JR NZ,r8", .cycles = 3, .length = 2 },
    [0x21] = { .func = op_ld_rr_d16, .name = "LD HL,d16", .cycles = 3, .length = 3 },
    [0x22] = { .func = NULL, .name = "LD (HL+),A", .cycles = 2, .length = 1 },
    [0x23] = { .func = NULL, .name = "INC HL", .cycles = 2, .length = 1 },
    [0x24] = { .func = op_inc_r, .name = "INC H", .cycles = 1, .length = 1 },
    [0x25] = { .func = op_dec_r, .name = "DEC H", .cycles = 1, .length = 1 },
    [0x26] = { .func = op_ld_r_d8, .name = "LD H,d8", .cycles = 2, .length = 2 },
    [0x27] = { .func = NULL, .name = "DAA", .cycles = 1, .length = 1 },
    [0x28] = { .func = NULL, .name = "JR Z,r8", .cycles = 3, .length = 2 },
    [0x29] = { .func = NULL, .name = "ADD HL,HL", .cycles = 2, .length = 1 },
    [0x2A] = { .func = NULL, .name = "LD A,(HL+)", .cycles = 2, .length = 1 },
    [0x2B] = { .func = NULL, .name = "DEC HL", .cycles = 2, .length = 1 },
    [0x2C] = { .func = op_inc_r, .name = "INC L", .cycles = 1, .length = 1 },
    [0x2D] = { .func = op_dec_r, .name = "DEC L", .cycles = 1, .length = 1 },
    [0x2E] = { .func= NULL, .name= "LD L,d8", .cycles = 2, .length = 2 },
    [0x2F] = { .func= NULL, .name= "CPL", .cycles = 1, .length = 1 },
    [0x30] = { .func = NULL, .name = "JR NC,r8", .cycles = 3, .length = 2 },
    [0x31] = { .func = op_ld_rr_d16, .name = "LD SP,d16", .cycles = 3, .length = 3 },
    [0x32] = { .func = NULL, .name = "LD (HL-),A", .cycles = 2, .length = 1 },
    [0x33] = { .func = NULL, .name = "INC SP", .cycles = 2, .length = 1 },
    [0x34] = { .func = op_inc_r, .name = "INC (HL)", .cycles = 3, .length = 1 },
    [0x35] = { .func = op_dec_r, .name = "DEC (HL)", .cycles = 3, .length = 1 },
    [0x36] = { .func = op_ld_r_d8, .name = "LD (HL),d8", .cycles = 3, .length = 2 },
    [0x37] = { .func = NULL, .name = "SCF", .cycles = 1, .length = 1 },
    [0x38] = { .func = NULL, .name = "JR C,r8", .cycles = 3, .length = 2 },
    [0x39] = { .func = NULL, .name = "ADD HL,SP", .cycles = 2, .length = 1 },
    [0x3A] = { .func = NULL, .name = "LD A,(HL-)", .cycles = 2, .length = 1 },
    [0x3B] = { .func = NULL, .name = "DEC SP", .cycles = 2, .length = 1 },
    [0x3C] = { .func = op_inc_r, .name = "INC A", .cycles = 1, .length = 1 },
    [0x3D] = { .func= op_dec_r, .name= "DEC A", .cycles = 1, .length = 1 },
    [0x3E] = { .func= NULL, .name= "LD A,d8", .cycles = 2, .length = 2 },
    [0x3F] = { .func= NULL, .name= "CCF", .cycles = 1, .length = 1 },
    [0x40] = { .func = op_ld_r_r, .name = "LD B,B", .cycles = 1, .length = 1 },
    [0x41] = { .func = op_ld_r_r, .name = "LD B,C", .cycles = 1, .length = 1 },
    [0x42] = { .func = op_ld_r_r, .name = "LD B,D", .cycles = 1, .length = 1 },
    [0x43] = { .func = op_ld_r_r, .name = "LD B,E", .cycles = 1, .length = 1 },
    [0x44] = { .func = op_ld_r_r, .name = "LD B,H", .cycles = 1, .length = 1 },
    [0x45] = { .func = op_ld_r_r, .name = "LD B,L", .cycles = 1, .length = 1 },
    [0x46] = { .func = op_ld_r_r, .name = "LD B,(HL)", .cycles = 2, .length = 1 },
    [0x47] = { .func = op_ld_r_r, .name = "LD B,A", .cycles = 1, .length = 1 },
    [0x48] = { .func = op_ld_r_r, .name = "LD C,B", .cycles = 1, .length = 1 },
    [0x49] = { .func = op_ld_r_r, .name = "LD C,C", .cycles = 1, .length = 1 },
    [0x4A] = { .func = op_ld_r_r, .name = "LD C,D", .cycles = 1, .length = 1 },
    [0x4B] = { .func = op_ld_r_r, .name = "LD C,E", .cycles = 1, .length = 1 },
    [0x4C] = { .func = op_ld_r_r, .name = "LD C,H", .cycles = 1, .length = 1 },
    [0x4D] = { .func = op_ld_r_r, .name= "LD C,L", .cycles = 1, .length = 1 },
    [0x4E] = { .func = op_ld_r_r, .name= "LD C,(HL)", .cycles = 2, .length = 1 },
    [0x4F] = { .func = op_ld_r_r, .name= "LD C,A", .cycles = 1, .length = 1 },
    [0x50] = { .func = op_ld_r_r, .name = "LD D,B", .cycles = 1, .length = 1 },
    [0x51] = { .func = op_ld_r_r, .name = "LD D,C", .cycles = 1, .length = 1 },
    [0x52] = { .func = op_ld_r_r, .name = "LD D,D", .cycles = 1, .length = 1 },
    [0x53] = { .func = op_ld_r_r, .name = "LD D,E", .cycles = 1, .length = 1 },
    [0x54] = { .func = op_ld_r_r, .name = "LD D,H", .cycles = 1, .length = 1 },
    [0x55] = { .func = op_ld_r_r, .name = "LD D,L", .cycles = 1, .length = 1 },
    [0x56] = { .func = op_ld_r_r, .name = "LD D,(HL)", .cycles = 2, .length = 1 },
    [0x57] = { .func = op_ld_r_r, .name = "LD D,A", .cycles = 1, .length = 1 },
    [0x58] = { .func = op_ld_r_r, .name = "LD E,B", .cycles = 1, .length = 1 },
    [0x59] = { .func = op_ld_r_r, .name = "LD E,C", .cycles = 1, .length = 1 },
    [0x5A] = { .func = op_ld_r_r, .name = "LD E,D", .cycles = 1, .length = 1 },
    [0x5B] = { .func = op_ld_r_r, .name = "LD E,E", .cycles = 1, .length = 1 },
    [0x5C] = { .func = op_ld_r_r, .name = "LD E,H", .cycles = 1, .length = 1 },
    [0x5D] = { .func = op_ld_r_r, .name= "LD E,L", .cycles = 1, .length = 1 },
    [0x5E] = { .func = op_ld_r_r, .name= "LD E,(HL)", .cycles = 2, .length = 1 },
    [0x5F] = { .func = op_ld_r_r, .name= "LD E,A", .cycles = 1, .length = 1 },
    [0x60] = { .func = op_ld_r_r, .name = "LD H,B", .cycles = 1, .length = 1 },
    [0x61] = { .func = op_ld_r_r, .name = "LD H,C", .cycles = 1, .length = 1 },
    [0x62] = { .func = op_ld_r_r, .name = "LD H,D", .cycles = 1, .length = 1 },
    [0x63] = { .func = op_ld_r_r, .name = "LD H,E", .cycles = 1, .length = 1 },
    [0x64] = { .func = op_ld_r_r, .name = "LD H,H", .cycles = 1, .length = 1 },
    [0x65] = { .func = op_ld_r_r, .name = "LD H,L", .cycles = 1, .length = 1 },
    [0x66] = { .func = op_ld_r_r, .name = "LD H,(HL)", .cycles = 2, .length = 1 },
    [0x67] = { .func = op_ld_r_r, .name = "LD H,A", .cycles = 1, .length = 1 },
    [0x68] = { .func = op_ld_r_r, .name = "LD L,B", .cycles = 1, .length = 1 },
    [0x69] = { .func = op_ld_r_r, .name = "LD L,C", .cycles = 1, .length = 1 },
    [0x6A] = { .func = op_ld_r_r, .name = "LD L,D", .cycles = 1, .length = 1 },
    [0x6B] = { .func = op_ld_r_r, .name = "LD L,E", .cycles = 1, .length = 1 },
    [0x6C] = { .func = op_ld_r_r, .name = "LD L,H", .cycles = 1, .length = 1 },
    [0x6D] = { .func = op_ld_r_r, .name= "LD L,L", .cycles = 1, .length = 1 },
    [0x6E] = { .func = op_ld_r_r, .name= "LD L,(HL)", .cycles = 2, .length = 1 },
    [0x6F] = { .func = op_ld_r_r, .name= "LD L,A", .cycles = 1, .length = 1 },
    [0x70] = { .func = op_ld_r_r, .name = "LD (HL),B", .cycles = 2, .length = 1 },
    [0x71] = { .func = op_ld_r_r, .name = "LD (HL),C", .cycles = 2, .length = 1 },
    [0x72] = { .func = op_ld_r_r, .name = "LD (HL),D", .cycles = 2, .length = 1 },
    [0x73] = { .func = op_ld_r_r, .name = "LD (HL),E", .cycles = 2, .length = 1 },
    [0x74] = { .func = op_ld_r_r, .name = "LD (HL),H", .cycles = 2, .length = 1 },
    [0x75] = { .func = op_ld_r_r, .name = "LD (HL),L", .cycles = 2, .length = 1 },
    [0x76] = { .func = op_halt, .name = "HALT", .cycles = 1, .length = 1 },
    [0x77] = { .func = op_ld_r_r, .name = "LD (HL),A", .cycles = 2, .length = 1 },
    [0x78] = { .func = op_ld_r_r, .name = "LD A,B", .cycles = 1, .length = 1 },
    [0x79] = { .func = op_ld_r_r, .name = "LD A,C", .cycles = 1, .length = 1 },
    [0x7A] = { .func = op_ld_r_r, .name = "LD A,D", .cycles = 1, .length = 1 },
    [0x7B] = { .func = op_ld_r_r, .name = "LD A,E", .cycles = 1, .length = 1 },
    [0x7C] = { .func = op_ld_r_r, .name = "LD A,H", .cycles = 1, .length = 1 },
    [0x7D] = { .func = op_ld_r_r, .name= "LD A,L", .cycles = 1, .length = 1 },
    [0x7E] = { .func = op_ld_r_r, .name= "LD A,(HL)", .cycles = 2, .length = 1 },
    [0x7F] = { .func = op_ld_r_r, .name= "LD A,A", .cycles = 1, .length = 1 },
    [0x80] = { .func = op_add_addc, .name = "ADD A,B", .cycles = 1, .length = 1 },
    [0x81] = { .func = op_add_addc, .name = "ADD A,C", .cycles = 1, .length = 1 },
    [0x82] = { .func = op_add_addc, .name = "ADD A,D", .cycles = 1, .length = 1 },
    [0x83] = { .func = op_add_addc, .name = "ADD A,E", .cycles = 1, .length = 1 },
    [0x84] = { .func = op_add_addc, .name = "ADD A,H", .cycles = 1, .length = 1 },
    [0x85] = { .func = op_add_addc, .name = "ADD A,L", .cycles = 1, .length = 1 },
    [0x86] = { .func = op_add_addc, .name = "ADD A,(HL)", .cycles = 2, .length = 1 },
    [0x87] = { .func = op_add_addc, .name = "ADD A,A", .cycles = 1, .length = 1 },
    [0x88] = { .func = op_add_addc, .name = "ADC A,B", .cycles = 1, .length = 1 },
    [0x89] = { .func = op_add_addc, .name = "ADC A,C", .cycles = 1, .length = 1 },
    [0x8A] = { .func = op_add_addc, .name = "ADC A,D", .cycles = 1, .length = 1 },
    [0x8B] = { .func = op_add_addc, .name = "ADC A,E", .cycles = 1, .length = 1 },
    [0x8C] = { .func = op_add_addc, .name = "ADC A,H", .cycles = 1, .length = 1 },
    [0x8D] = { .func = op_add_addc, .name= "ADC A,L", .cycles= 1, .length= 1 },
    [0x8E] = { .func= op_add_addc, .name= "ADC A,(HL)",.cycles=2,.length=1},
    [0x8F] = { .func=op_add_addc, .name="ADC A,A",.cycles=1,.length=1},
    [0x90] = { .func = op_sub_sbc_a_r, .name = "SUB B", .cycles = 1, .length = 1 },
    [0x91] = { .func = op_sub_sbc_a_r, .name = "SUB C", .cycles = 1, .length = 1 },
    [0x92] = { .func = op_sub_sbc_a_r, .name = "SUB D", .cycles = 1, .length = 1 },
    [0x93] = { .func = op_sub_sbc_a_r, .name = "SUB E", .cycles = 1, .length = 1 },
    [0x94] = { .func = op_sub_sbc_a_r, .name = "SUB H", .cycles = 1, .length = 1 },
    [0x95] = { .func = op_sub_sbc_a_r, .name = "SUB L", .cycles = 1, .length = 1 },
    [0x96] = { .func = op_sub_sbc_a_r, .name = "SUB (HL)", .cycles = 2, .length = 1 },
    [0x97] = { .func = op_sub_sbc_a_r, .name = "SUB A", .cycles = 1, .length = 1 },
    [0x98] = { .func = op_sub_sbc_a_r, .name = "SBC A,B", .cycles = 1, .length = 1 },
    [0x99] = { .func = op_sub_sbc_a_r, .name = "SBC A,C", .cycles = 1, .length = 1 },
    [0x9A] = { .func = op_sub_sbc_a_r, .name = "SBC A,D", .cycles = 1, .length = 1 },
    [0x9B] = { .func = op_sub_sbc_a_r, .name = "SBC A,E", .cycles = 1, .length = 1 },
    [0x9C] = { .func = op_sub_sbc_a_r, .name = "SBC A,H", .cycles= 1, .length= 1 },
    [0x9D] = { .func = op_sub_sbc_a_r, .name = "SBC A,L", .cycles= 1, .length= 1 },
    [0x9E] = { .func = op_sub_sbc_a_r, .name = "SBC A,(HL)",.cycles=2,.length=1},
    [0x9F] = { .func = op_sub_sbc_a_r, .name = "SBC A,A",.cycles=1,.length=1},
    [0xA0] = { .func = op_and_a_r, .name = "AND B", .cycles = 1, .length = 1 },
    [0xA1] = { .func = op_and_a_r, .name = "AND C", .cycles = 1, .length = 1 },
    [0xA2] = { .func = op_and_a_r, .name = "AND D", .cycles = 1, .length = 1 },
    [0xA3] = { .func = op_and_a_r, .name = "AND E", .cycles = 1, .length = 1 },
    [0xA4] = { .func = op_and_a_r, .name = "AND H", .cycles = 1, .length = 1 },
    [0xA5] = { .func = op_and_a_r, .name = "AND L", .cycles = 1, .length = 1 },
    [0xA6] = { .func = op_and_a_r, .name = "AND (HL)", .cycles = 2, .length = 1 },
    [0xA7] = { .func = op_and_a_r, .name = "AND A", .cycles = 1, .length = 1 },
    [0xA8] = { .func = op_xor_a_r, .name = "XOR B", .cycles = 1, .length = 1 },
    [0xA9] = { .func = op_xor_a_r, .name = "XOR C", .cycles = 1, .length= 1 },
    [0xAA] = { .func = op_xor_a_r, .name = "XOR D", .cycles = 1, .length= 1 },
    [0xAB] = { .func = op_xor_a_r, .name = "XOR E", .cycles = 1, .length= 1 },
    [0xAC] = { .func = op_xor_a_r, .name = "XOR H", .cycles = 1, .length= 1 },
    [0xAD] = { .func = op_xor_a_r, .name= "XOR L", .cycles= 1, .length= 1 },
    [0xAE] = { .func = op_xor_a_r, .name= "XOR (HL)",.cycles=2,.length=1},
    [0xAF] = { .func = op_xor_a_r, .name = "XOR A", .cycles = 1, .length = 1 },
    [0xB0 ... 0xC5] = {.func=NULL,.name="UNIMPLEMENTED",.cycles=0,.length=1},
    [0xC6] = { .func = op_add_addc, .name = "ADD A,d8", .cycles = 2, .length = 2 },
    [0xC7 ... 0xCD] = {.func=NULL,.name="UNIMPLEMENTED",.cycles=0,.length=1},
    [0xCE] = { .func = op_add_addc, .name = "ADC A,d8", .cycles = 2, .length = 2 },
    [0xCF ... 0xE5] = {.func=NULL,.name="UNIMPLEMENTED",.cycles=0,.length=1},
    [0xE6] = { .func = op_and_a_r, .name = "AND d8", .cycles = 2, .length = 2 },
    [0xE7 ... 0xED] = {.func=NULL,.name="UNIMPLEMENTED",.cycles=0,.length=1},
    [0xEE] = { .func = op_xor_a_r, .name = "XOR d8", .cycles = 2, .length = 2 },
    [0xEF ... 0xFF] = {.func=NULL,.name="UNIMPLEMENTED",.cycles=0,.length=1},
    
};  

// Inicializa la CPU a su estado por defecto
void cpu_init(Cpu* cpu) {
    // Estado post-boot ROM
    cpu->PC = 0x0100; // Punto de entrada de los cartuchos
    cpu->SP = 0xFFFE; // Puntero de pila inicial

    // Valores iniciales de los registros tras el boot ROM
    cpu->A = 0x01;
    cpu->F = 0xB0;
    cpu->B = 0x00;
    cpu->C = 0x13;
    cpu->D = 0x00;
    cpu->E = 0xD8;
    cpu->H = 0x01;
    cpu->L = 0x4D;

    // Estado interno
    cpu->ime = false;    // Interrupts deshabilitados
    cpu->halted = false; // No en modo halt
}

// Retorna el número de M-Cycles consumidos por la instrucción ejecutada
int cpu_step(GameBoy* gb) {
    if (gb->cpu.halted) {
        // En modo halt, no se ejecutan instrucciones
        return 1; // Consume 1 M-Cycle
    }

    // 1. FETCH: Leer el opcode en la dirección PC
    u8 opcode = bus_read(gb, gb->cpu.PC);
    const Instruction* instr = &instruction_set[opcode];

    if (instr->func) {
        // Debug: Imprimir la instrucción que se va a ejecutar
        printf("Ejecutando: %s (0x%02X) en PC:0x%04X\n", instr->name, opcode, gb->cpu.PC);
        // Ejecutar la función asociada a la instrucción
        gb->cpu.PC++; // Avanzamos PC antes de ejecutar la instrucción
        instr->func(gb);
    } else {
        // Instrucción no implementada
        printf("Instrucción no implementada: %s (0x%02X) en PC:0x%04X\n", instr->name, opcode, gb->cpu.PC);
        exit(1);
    }

    // Retornar el número de M-Cycles consumidos
    return instr->cycles;
}

// Helper para obtener punteros a registros según el índice (0-7)
u8* get_register_ptr(Cpu* cpu, int index) {
    switch (index) {
        case 0: return &cpu->B;
        case 1: return &cpu->C;
        case 2: return &cpu->D;
        case 3: return &cpu->E;
        case 4: return &cpu->H;
        case 5: return &cpu->L;
        case 6: return NULL; // No hay registro HL
        case 7: return &cpu->A;
    }
    return NULL; // Indicador de error
}

// Helper: Escribe un valor de 16 bits en el par de registros dado
// index: 0=BC, 1=DE, 2=HL, 3=SP
void write_register_pair(Cpu* cpu, int index, u16 value) {
    switch (index) {
        case 0: // BC
            cpu->B = (value >> 8) & 0xFF;
            cpu->C = value & 0xFF;
            break;
        case 1: // DE
            cpu->D = (value >> 8) & 0xFF;
            cpu->E = value & 0xFF;
            break;
        case 2: // HL
            cpu->H = (value >> 8) & 0xFF;
            cpu->L = value & 0xFF;
            break;
        case 3: // SP
            cpu->SP = value;
            break;
    }
}

// Función NOP (No Operation)
void op_nop(GameBoy* gb) {
    // No hace nada
    // Evitar warnings de variables no usadas
    (void)gb;
}

// Función UNIVERSAL para LD r, r'
// Para las cargas de 8 bits (LD r, r'), el opcode sigue este patrón binario:
// 01 ddd sss
//    ddd: destino (registro o (HL))
//    sss: fuente (registro o (HL))
//
// Los registros se mapean así:
//    000 - B
//    001 - C
//    010 - D
//    011 - E
//    100 - H
//    101 - L
//    110 - (HL)  <- Indica acceso a memoria en la dirección apuntada
//    111 - A
void op_ld_r_r(GameBoy* gb) {
    // 1. Recuperamos el opcode actual
    // Como PC ya avanzó en el bucle principal, el opcode está en PC - 1 
    u8 opcode = bus_read(gb, gb->cpu.PC - 1);
    
    // 2. Extraemos los índices de los registros fuente y destino
    int src_index = (opcode >> 3) & 0x07; // Bits 5-3
    int dst_index = opcode & 0x07;        // Bits 2-0

    // 3. Obtenemos punteros a los registros o manejamos (HL)
    u8* src_ptr = get_register_ptr(&gb->cpu, src_index);
    u8* dst_ptr = get_register_ptr(&gb->cpu, dst_index);

    // 4. Realizamos la operación de carga
    if (src_ptr && dst_ptr) {
        // Ambos son registros
        *src_ptr = *dst_ptr;
    } 
    else if (src_ptr && dst_index == 6) {
        // r2 es (HL)
        u16 hl_addr = (gb->cpu.H << 8) | gb->cpu.L;
        *src_ptr = bus_read(gb, hl_addr);
    } 
    else if (src_index == 6 && dst_ptr) {
        // r1 es (HL)
        u16 hl_addr = (gb->cpu.H << 8) | gb->cpu.L;
        bus_write(gb, hl_addr, *dst_ptr);
    }

    // Caso no manejado (debería ser imposible llegar aquí)
    printf("Error en LD r, r con opcode: 0x%02X\n", opcode);
    exit(1);
}

// LD r, n: Load immediate 8-bit value into register r
void op_ld_r_d8(GameBoy* gb) {
    // 1. Recuperamos el opcode (PC ya avanzó en el bucle principal)
    u8 opcode = bus_read(gb, gb->cpu.PC - 1);

    // 2. Extraemos el índice del registro destino
    // Formato del opcode: 00 ddd110
    int reg_index = (opcode >> 3) & 0x07; // Bits 5-3

    // 3. Leemos el valor inmediato de 8 bits desde la memoria
    u8 immediate_value = bus_read(gb, gb->cpu.PC);

    // 4. Escribimos el valor inmediato en el registro destino
    u8* reg_ptr = get_register_ptr(&gb->cpu, reg_index);
    if (reg_ptr && reg_index != 6) {
        *reg_ptr = immediate_value;
    } 
    else if (reg_index == 6) {
        // Escribimos en (HL)
        u16 hl_addr = (gb->cpu.H << 8) | gb->cpu.L;
        bus_write(gb, hl_addr, immediate_value);
    } 
    else {
        printf("Error en LD r, n con opcode: 0x%02X\n", opcode);
        exit(1);
    }
    // 5. Avanzamos el PC en 1 byte adicional (ya que leímos n)
    gb->cpu.PC += 1;
}

// Cargas de 16 bits (LD rr, d16)
void op_ld_rr_d16(GameBoy* gb) {
    // 1. Recuperamos el opcode (PC ya avanzó en el bucle principal)
    u8 opcode = bus_read(gb, gb->cpu.PC - 1);

    // 2. Extraemos el índice del par de registros
    // Opcode: 00 rr 0001
    int reg_pair_index = (opcode >> 4) & 0x03; // Bits 5-4

    // 3. Leemos el valor inmediato de 16 bits desde la memoria (d16)
    // GameBoy es Little Endian, así que leemos el byte bajo primero y luego el alto
    u8 low_byte = bus_read(gb, gb->cpu.PC);
    u8 high_byte = bus_read(gb, gb->cpu.PC + 1);
    u16 value = (high_byte << 8) | low_byte;

    // 4. Escribimos el valor en el par de registros correspondiente
    write_register_pair(&gb->cpu, reg_pair_index, value);

    // 5. Avanzamos el PC en 2 bytes adicionales (ya que leímos d16)
    gb->cpu.PC += 2;
} 

void op_halt(GameBoy* gb) {
    gb->cpu.halted = true;
}

// INC r: Incrementa registro
void op_inc_r(GameBoy* gb) {
    // Implementación de la instrucción INC r
    // 1. Recuperamos el opcode (PC ya avanzó en el bucle principal)

    u8 opcode = bus_read(gb, gb->cpu.PC - 1);

    // 2. Extraemos el índice del registro
    // Formato del opcode: 00 ddd100
    int reg_index = (opcode >> 3) & 0x07; // Bits 5-3
    u8* reg_ptr = get_register_ptr(&gb->cpu, reg_index);
    u8 value = 0;
    
    // 3. Incrementamos el valor del registro
    if (reg_ptr && reg_index != 6) {
        (*reg_ptr)++;
        value = *reg_ptr;
    }
    else if (reg_index == 6) {
        // Incrementamos el valor en (HL)
        u16 hl_addr = (gb->cpu.H << 8) | gb->cpu.L;
        value = bus_read(gb, hl_addr);
        value++;
        bus_write(gb, hl_addr, value);
    }
    else {
        printf("Error en INC r con opcode: 0x%02X\n", opcode);
        exit(1);
    }
    
    // 4. Actualizamos los flags
    // Conservamos el flag C, borramos los otros 3
    gb->cpu.F = gb->cpu.F & CPU_F_C;
    
    // Actualizamos el flag Z
    gb->cpu.F |= CHECK_ZERO(value);

    // INC pone N a 0
    gb->cpu.F &= ~CPU_F_N;

    // Actualizamos el flag H
    gb->cpu.F |= CHECK_HALF_CARRY_ADD(value, 1);
}

// DEC r: Decrementa registro
void op_dec_r(GameBoy* gb) {
    // Implementación de la instrucción DEC r
    // 1. Recuperamos el opcode (PC ya avanzó en el bucle principal)

    u8 opcode = bus_read(gb, gb->cpu.PC - 1);

    // 2. Extraemos el índice del registro
    // Formato del opcode: 00 ddd101
    int reg_index = (opcode >> 3) & 0x07; // Bits 5-3
    u8* reg_ptr = get_register_ptr(&gb->cpu, reg_index);
    u8 value = 0;
    
    // 3. Decrementamos el valor del registro
    if (reg_ptr && reg_index != 6) {
        (*reg_ptr)--;
        value = *reg_ptr;
    }
    else if (reg_index == 6) {
        // Decrementamos el valor en (HL)
        u16 hl_addr = (gb->cpu.H << 8) | gb->cpu.L;
        value = bus_read(gb, hl_addr);
        value--;
        bus_write(gb, hl_addr, value);
    }
    else {
        printf("Error en DEC r con opcode: 0x%02X\n", opcode);
        exit(1);
    }
    
    // 4. Actualizamos los flags
    // Conservamos el flag C, borramos los otros 3
    gb->cpu.F = gb->cpu.F & CPU_F_C;
    
    // Actualizamos el flag Z
    gb->cpu.F |= CHECK_ZERO(value);

    // DEC pone N a 1
    gb->cpu.F |= CPU_F_N;

    // Actualizamos el flag H
    gb->cpu.F |= CHECK_HALF_CARRY_SUB(value, 1);
}

// ADD A, r: Suma registro a A
// ADC A, r (suma con carry) si el bit 3 del opcode está seteado
// ADD A, d8 si el opcode es 0xC6
// ADDC A, d8 si el opcode es 0xCE
void op_add_addc(GameBoy* gb) {
    // 1. Recuperamos el opcode (PC ya avanzó en el bucle principal
    u8 opcode = bus_read(gb, gb->cpu.PC - 1);

    // 2. Extraemos el índice del registro fuente
    // Formato del opcode: 10000sss
    int src_index = opcode & 0x07; // Bits 2-0
    u8* src_ptr = get_register_ptr(&gb->cpu, src_index);

    // 3. Realizamos la suma
    u8 value = 0;
    if (opcode == 0xC6 || opcode == 0xCE) {
        // ADD A, d8
        value = bus_read(gb, gb->cpu.PC);
        gb->cpu.PC += 1; // Avanzamos PC por el byte inmediato
    }
    else if (src_ptr && src_index != 6) {
        value = *src_ptr;
    }
    else if (src_index == 6) {
        // ADD A, (HL)
        u16 hl_addr = (gb->cpu.H << 8) | gb->cpu.L;
        value = bus_read(gb, hl_addr);
    }
    
    else {
        printf("Error en ADD A, r con opcode: 0x%02X\n", opcode);
        exit(1);
    }

    if (opcode & 0x08) {
        // ADDC: sumar con carry
        value += (gb->cpu.F & CPU_F_C) ? 1 : 0;
    }
    u16 result = gb->cpu.A + value;
    
    // 4. Actualizamos los flags
    gb->cpu.F = 0;
    gb->cpu.F |= CHECK_ZERO(result);
    gb->cpu.F |= CHECK_HALF_CARRY_ADD(gb->cpu.A, value);
    gb->cpu.F |= CHECK_CARRY_ADD(gb->cpu.A, value);

    // 5. Almacenamos el resultado en A
    gb->cpu.A = (u8)(result & 0xFF);
}   

// SUB A, r: Resta registro a A
// También realiza SBC A, r (resta con carry) si el bit 3 del opcode está seteado
void op_sub_sbc_a_r(GameBoy* gb) {
    // 1. Recuperamos el opcode (PC ya avanzó en el bucle principal
    u8 opcode = bus_read(gb, gb->cpu.PC - 1);

    // 2. Extraemos el índice del registro fuente
    // Formato del opcode: 10010sss
    int src_index = opcode & 0x07; // Bits 2-0
    u8* src_ptr = get_register_ptr(&gb->cpu, src_index);

    // 3. Realizamos la resta
    u8 value = 0;
    if (src_ptr && src_index != 6) {
        value = *src_ptr;
    }
    else if (src_index == 6) {
        // Fuente es (HL)
        u16 hl_addr = (gb->cpu.H << 8) | gb->cpu.L;
        value = bus_read(gb, hl_addr);
    }
    else {
        printf("Error en SUB A, r con opcode: 0x%02X\n", opcode);
        exit(1);
    }
    if (opcode & 0x08) {
        // SBC: restar con carry
        value += (gb->cpu.F & CPU_F_C) ? 1 : 0;
    }
    u16 result = gb->cpu.A - value;

    // 4. Actualizamos los flags
    gb->cpu.F = 0;
    gb->cpu.F |= CHECK_ZERO(result);
    gb->cpu.F |= CHECK_HALF_CARRY_SUB(gb->cpu.A, value);
    gb->cpu.F |= CHECK_CARRY_SUB(gb->cpu.A, value);
    gb->cpu.F |= CPU_F_N; // Resta pone N a 1

    // 5. Almacenamos el resultado en A
    gb->cpu.A = (u8)(result & 0xFF);
}  

// AND A, r: AND registro con A
void op_and_a_r(GameBoy* gb) {
    // 1. Recuperamos el opcode (PC ya avanzó en el bucle principal)
    u8 opcode = bus_read(gb, gb->cpu.PC - 1);

    // 2. Extraemos el índice del registro fuente
    // Formato del opcode: 10100sss
    int src_index = opcode & 0x07; // Bits 2-0
    u8* src_ptr = get_register_ptr(&gb->cpu, src_index);

    // 3. Obtenemos el segundo operando
    u8 value = 0;
    if (opcode == 0xE6) {
        // AND A, d8
        value = bus_read(gb, gb->cpu.PC);
        gb->cpu.PC += 1; // Avanzamos PC por el byte inmediato
    }
    else if (src_ptr && src_index != 6) {
        value = *src_ptr;
    }
    else if (src_index == 6) {
        // AND A, (HL)
        u16 hl_addr = (gb->cpu.H << 8) | gb->cpu.L;
        value = bus_read(gb, hl_addr);
        printf("HL Address: 0x%04X, Value: 0x%02X\n", hl_addr, value);
    }
    else {
        printf("Error en AND A, r con opcode: 0x%02X\n", opcode);
        exit(1);
    }

    // 4 Realizamos la operación AND
    gb->cpu.A &= value;

    // 5. Actualizamos los flags
    gb->cpu.F = CPU_F_H; // AND siempre pone H a 1
    gb->cpu.F |= CHECK_ZERO(gb->cpu.A);
}

// XOR A, r: XOR registro con A
// XOR A, d8 si el opcode es 0xEE
// XOR A, (HL) si el registro fuente es 110
void op_xor_a_r(GameBoy* gb) {
    // 1. Recuperamos el opcode (PC ya avanzó en el bucle principal)
    u8 opcode = bus_read(gb, gb->cpu.PC - 1);

    // 2. Extraemos el índice del registro fuente
    // Formato del opcode: 10101sss
    int src_index = opcode & 0x07; // Bits 2-0
    u8* src_ptr = get_register_ptr(&gb->cpu, src_index);

    // 3. Obtenemos el segundo operando
    u8 value = 0;
    if (opcode == 0xEE) {
        // XOR A, d8
        value = bus_read(gb, gb->cpu.PC);
        gb->cpu.PC += 1; // Avanzamos PC por el byte inmediato
    }
    else if (src_ptr && src_index != 6) {
        value = *src_ptr;
    }
    else if (src_index == 6) {
        // XOR A, (HL)
        u16 hl_addr = (gb->cpu.H << 8) | gb->cpu.L;
        value = bus_read(gb, hl_addr);
    }
    else {
        printf("Error en XOR A, r con opcode: 0x%02X\n", opcode);
        exit(1);
    }

    // 4 Realizamos la operación XOR
    gb->cpu.A ^= value;

    // 5. Actualizamos los flags
    gb->cpu.F = 0; // XOR borra todos los flags
    gb->cpu.F |= CHECK_ZERO(gb->cpu.A);
}