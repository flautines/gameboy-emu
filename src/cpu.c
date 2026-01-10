// src/cpu.c
#include "gb.h"
#include <stdlib.h>

typedef struct {
    void (*func)(GameBoy* gb); // Puntero a la función que implementa la instrucción
    char* name;               // Nombre de la instrucción (para debugging)
    u8 cycles;                 // Número de M-Cycles que consume la instrucción
    u8 length;                 // Longitud en bytes de la instrucción
} Instruction;

void print_cpu_state(const Cpu* cpu);

// Declaraciones de funciones (prototipos)
void op_nop(GameBoy* gb);
void op_ld_r_r(GameBoy* gb);
void op_ld_r_d8(GameBoy* gb);
void op_ld_rr_d16(GameBoy* gb);

void op_halt(GameBoy* gb);

void op_inc_r(GameBoy* gb);
void op_dec_r(GameBoy* gb);

void op_add_a_r(GameBoy* gb);
void op_add_a_d8(GameBoy* gb);
void op_adc_a_r(GameBoy* gb);
void op_adc_a_d8(GameBoy* gb);
void op_sub_a_r(GameBoy* gb);
void op_sub_a_d8(GameBoy* gb);
void op_sbc_a_r(GameBoy* gb);
void op_sbc_a_d8(GameBoy* gb);
void op_cp_a_r(GameBoy* gb);
void op_cp_a_d8(GameBoy* gb);
void op_and_a_r(GameBoy* gb);
void op_and_a_d8(GameBoy* gb);
void op_xor_a_r(GameBoy* gb);
void op_xor_a_d8(GameBoy* gb);
void op_or_a_r(GameBoy* gb);
void op_or_a_d8(GameBoy* gb);

void op_ld_addr_rr_a(GameBoy* gb);
void op_ld_a_addr_rr(GameBoy* gb);
void op_inc_dec_rr(GameBoy* gb);
void op_push_rr(GameBoy* gb);
void op_pop_rr(GameBoy* gb);

void op_jp_nn(GameBoy* gb);
void op_jp_cc_nn(GameBoy* gb);
void op_jp_hl(GameBoy* gb);
void op_jr_e(GameBoy* gb);
void op_jr_cc_e(GameBoy* gb);
void op_call_nn(GameBoy* gb);
void op_call_cc_nn(GameBoy* gb);

// Tabla de instrucciones (completa con todas las instrucciones)
Instruction instruction_set[256] = {
    [0x00] = { .func = op_nop, .name = "NOP", .cycles = 1, .length = 1 },
    [0x01] = { .func = op_ld_rr_d16, .name = "LD BC,d16", .cycles = 3, .length = 3 },
    [0x02] = { .func = op_ld_addr_rr_a, .name = "LD (BC),A", .cycles = 2, .length = 1 },
    [0x03] = { .func = op_inc_dec_rr, .name = "INC BC", .cycles = 2, .length = 1 },
    [0x04] = { .func = op_inc_r, .name = "INC B", .cycles = 1, .length = 1 },
    [0x05] = { .func = op_dec_r, .name = "DEC B", .cycles = 1, .length = 1 },
    [0x06] = { .func = op_ld_r_d8, .name = "LD B,d8", .cycles = 2, .length = 2 },
    [0x07] = { .func = NULL, .name = "RLCA", .cycles = 1, .length = 1 },
    [0x08] = { .func = NULL, .name = "LD (a16),SP", .cycles = 5, .length = 3 },
    [0x09] = { .func = NULL, .name = "ADD HL,BC", .cycles = 2, .length = 1 },
    [0x0A] = { .func = op_ld_a_addr_rr, .name = "LD A,(BC)", .cycles = 2, .length = 1 },
    [0x0B] = { .func = op_inc_dec_rr, .name = "DEC BC", .cycles = 2, .length = 1 },
    [0x0C] = { .func = op_inc_r, .name = "INC C", .cycles = 1, .length = 1 },
    [0x0D] = { .func = op_dec_r, .name = "DEC C", .cycles = 1, .length = 1 },
    [0x0E] = { .func = op_ld_r_d8, .name = "LD C,d8", .cycles = 2, .length = 2 },
    [0x0F] = { .func = NULL, .name = "RRCA", .cycles = 1, .length = 1 },
    [0x10] = { .func = NULL, .name = "STOP", .cycles = 2, .length = 2 },
    [0x11] = { .func = op_ld_rr_d16, .name = "LD DE,d16", .cycles = 3, .length = 3 },
    [0x12] = { .func = op_ld_addr_rr_a, .name = "LD (DE),A", .cycles = 2, .length = 1 },
    [0x13] = { .func = op_inc_dec_rr, .name = "INC DE", .cycles = 2, .length = 1 },
    [0x14] = { .func = op_inc_r, .name = "INC D", .cycles = 1, .length = 1 },
    [0x15] = { .func = op_dec_r, .name = "DEC D", .cycles = 1, .length = 1 },
    [0x16] = { .func = op_ld_r_d8, .name = "LD D,d8", .cycles = 2, .length = 2 },
    [0x17] = { .func = NULL, .name = "RLA", .cycles = 1, .length = 1 },
    [0x18] = { .func = op_jr_e, .name = "JR r8", .cycles = 3, .length = 2 },
    [0x19] = { .func = NULL, .name = "ADD HL,DE", .cycles = 2, .length = 1 },
    [0x1A] = { .func = op_ld_a_addr_rr, .name = "LD A,(DE)", .cycles = 2, .length = 1 },
    [0x1B] = { .func = op_inc_dec_rr, .name = "DEC DE", .cycles = 2, .length = 1 },
    [0x1C] = { .func = op_inc_r, .name = "INC E", .cycles = 1, .length = 1 },
    [0x1D] = { .func= op_dec_r, .name = "DEC E", .cycles = 1, .length = 1 },
    [0x1E] = { .func= op_ld_r_d8, .name = "LD E,d8", .cycles = 2, .length = 2 },
    [0x1F] = { .func= NULL, .name = "RRA", .cycles = 1, .length = 1 },
    [0x20] = { .func = op_jr_cc_e, .name = "JR NZ,r8", .cycles = 2, .length = 2 },
    [0x21] = { .func = op_ld_rr_d16, .name = "LD HL,d16", .cycles = 3, .length = 3 },
    [0x22] = { .func = op_ld_addr_rr_a, .name = "LD (HL+),A", .cycles = 2, .length = 1 },
    [0x23] = { .func = op_inc_dec_rr, .name = "INC HL", .cycles = 2, .length = 1 },
    [0x24] = { .func = op_inc_r, .name = "INC H", .cycles = 1, .length = 1 },
    [0x25] = { .func = op_dec_r, .name = "DEC H", .cycles = 1, .length = 1 },
    [0x26] = { .func = op_ld_r_d8, .name = "LD H,d8", .cycles = 2, .length = 2 },
    [0x27] = { .func = NULL, .name = "DAA", .cycles = 1, .length = 1 },
    [0x28] = { .func = op_jr_cc_e, .name = "JR Z,r8", .cycles = 2, .length = 2 },
    [0x29] = { .func = NULL, .name = "ADD HL,HL", .cycles = 2, .length = 1 },
    [0x2A] = { .func = op_ld_a_addr_rr, .name = "LD A,(HL+)", .cycles = 2, .length = 1 },
    [0x2B] = { .func = op_inc_dec_rr, .name = "DEC HL", .cycles = 2, .length = 1 },
    [0x2C] = { .func = op_inc_r, .name = "INC L", .cycles = 1, .length = 1 },
    [0x2D] = { .func = op_dec_r, .name = "DEC L", .cycles = 1, .length = 1 },
    [0x2E] = { .func= op_ld_r_d8, .name= "LD L,d8", .cycles = 2, .length = 2 },
    [0x2F] = { .func= NULL, .name= "CPL", .cycles = 1, .length = 1 },
    [0x30] = { .func = op_jr_cc_e, .name = "JR NC,r8", .cycles = 2, .length = 2 },
    [0x31] = { .func = op_ld_rr_d16, .name = "LD SP,d16", .cycles = 3, .length = 3 },
    [0x32] = { .func = op_ld_addr_rr_a, .name = "LD (HL-),A", .cycles = 2, .length = 1 },
    [0x33] = { .func = op_inc_dec_rr, .name = "INC SP", .cycles = 2, .length = 1 },
    [0x34] = { .func = op_inc_r, .name = "INC (HL)", .cycles = 3, .length = 1 },
    [0x35] = { .func = op_dec_r, .name = "DEC (HL)", .cycles = 3, .length = 1 },
    [0x36] = { .func = op_ld_r_d8, .name = "LD (HL),d8", .cycles = 3, .length = 2 },
    [0x37] = { .func = NULL, .name = "SCF", .cycles = 1, .length = 1 },
    [0x38] = { .func = op_jr_cc_e, .name = "JR C,r8", .cycles = 2, .length = 2 },
    [0x39] = { .func = NULL, .name = "ADD HL,SP", .cycles = 2, .length = 1 },
    [0x3A] = { .func = op_ld_a_addr_rr, .name = "LD A,(HL-)", .cycles = 2, .length = 1 },
    [0x3B] = { .func = op_inc_dec_rr, .name = "DEC SP", .cycles = 2, .length = 1 },
    [0x3C] = { .func = op_inc_r, .name = "INC A", .cycles = 1, .length = 1 },
    [0x3D] = { .func= op_dec_r, .name= "DEC A", .cycles = 1, .length = 1 },
    [0x3E] = { .func= op_ld_r_d8, .name= "LD A,d8", .cycles = 2, .length = 2 },
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
    [0x80] = { .func = op_add_a_r, .name = "ADD A,B", .cycles = 1, .length = 1 },
    [0x81] = { .func = op_add_a_r, .name = "ADD A,C", .cycles = 1, .length = 1 },
    [0x82] = { .func = op_add_a_r, .name = "ADD A,D", .cycles = 1, .length = 1 },
    [0x83] = { .func = op_add_a_r, .name = "ADD A,E", .cycles = 1, .length = 1 },
    [0x84] = { .func = op_add_a_r, .name = "ADD A,H", .cycles = 1, .length = 1 },
    [0x85] = { .func = op_add_a_r, .name = "ADD A,L", .cycles = 1, .length = 1 },
    [0x86] = { .func = op_add_a_r, .name = "ADD A,(HL)", .cycles = 2, .length = 1 },
    [0x87] = { .func = op_add_a_r, .name = "ADD A,A", .cycles = 1, .length = 1 },
    [0x88] = { .func = op_adc_a_r, .name = "ADC A,B", .cycles = 1, .length = 1 },
    [0x89] = { .func = op_adc_a_r, .name = "ADC A,C", .cycles = 1, .length = 1 },
    [0x8A] = { .func = op_adc_a_r, .name = "ADC A,D", .cycles = 1, .length = 1 },
    [0x8B] = { .func = op_adc_a_r, .name = "ADC A,E", .cycles = 1, .length = 1 },
    [0x8C] = { .func = op_adc_a_r, .name = "ADC A,H", .cycles = 1, .length = 1 },
    [0x8D] = { .func = op_adc_a_r, .name= "ADC A,L", .cycles= 1, .length= 1 },
    [0x8E] = { .func= op_adc_a_r, .name= "ADC A,(HL)",.cycles=2,.length=1 },
    [0x8F] = { .func= op_adc_a_r, .name= "ADC A,A", .cycles=1,.length=1 },
    [0x90] = { .func = op_sub_a_r, .name = "SUB B", .cycles = 1, .length = 1 },
    [0x91] = { .func = op_sub_a_r, .name = "SUB C", .cycles = 1, .length = 1 },
    [0x92] = { .func = op_sub_a_r, .name = "SUB D", .cycles = 1, .length = 1 },
    [0x93] = { .func = op_sub_a_r, .name = "SUB E", .cycles = 1, .length = 1 },
    [0x94] = { .func = op_sub_a_r, .name = "SUB H", .cycles = 1, .length = 1 },
    [0x95] = { .func = op_sub_a_r, .name = "SUB L", .cycles = 1, .length = 1 },
    [0x96] = { .func = op_sub_a_r, .name = "SUB (HL)", .cycles = 2, .length = 1 },
    [0x97] = { .func = op_sub_a_r, .name = "SUB A", .cycles = 1, .length = 1 },
    [0x98] = { .func = op_sbc_a_r, .name = "SBC A,B", .cycles = 1, .length = 1 },
    [0x99] = { .func = op_sbc_a_r, .name = "SBC A,C", .cycles = 1, .length = 1 },
    [0x9A] = { .func = op_sbc_a_r, .name = "SBC A,D", .cycles = 1, .length = 1 },
    [0x9B] = { .func = op_sbc_a_r, .name = "SBC A,E", .cycles = 1, .length = 1 },
    [0x9C] = { .func = op_sbc_a_r, .name = "SBC A,H", .cycles= 1, .length= 1 },
    [0x9D] = { .func = op_sbc_a_r, .name = "SBC A,L", .cycles= 1, .length= 1 },
    [0x9E] = { .func = op_sbc_a_r, .name = "SBC A,(HL)",.cycles=2,.length=1},
    [0x9F] = { .func = op_sbc_a_r, .name = "SBC A,A",.cycles=1,.length=1},
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
    [0xB0] = { .func = op_or_a_r, .name = "OR B", .cycles = 1, .length = 1 },
    [0xB1] = { .func = op_or_a_r, .name = "OR C", .cycles = 1, .length = 1 },
    [0xB2] = { .func = op_or_a_r, .name = "OR D", .cycles = 1, .length = 1 },
    [0xB3] = { .func = op_or_a_r, .name = "OR E", .cycles = 1, .length = 1 },
    [0xB4] = { .func = op_or_a_r, .name = "OR H", .cycles = 1, .length = 1 },
    [0xB5] = { .func = op_or_a_r, .name = "OR L", .cycles = 1, .length = 1 },
    [0xB6] = { .func = op_or_a_r, .name = "OR (HL)", .cycles = 2, .length = 1 },
    [0xB7] = { .func = op_or_a_r, .name = "OR A", .cycles = 1, .length = 1 },
    [0xB8] = { .func = op_cp_a_r, .name = "CP B", .cycles = 1, .length = 1 },
    [0xB9] = { .func = op_cp_a_r, .name = "CP C", .cycles = 1, .length = 1 },
    [0xBA] = { .func = op_cp_a_r, .name = "CP D", .cycles = 1, .length = 1 },
    [0xBB] = { .func = op_cp_a_r, .name = "CP E", .cycles = 1, .length = 1 },
    [0xBC] = { .func = op_cp_a_r, .name = "CP H", .cycles = 1, .length = 1 },
    [0xBD] = { .func = op_cp_a_r, .name = "CP L", .cycles = 1, .length = 1 },
    [0xBE] = { .func = op_cp_a_r, .name = "CP (HL)", .cycles = 2, .length = 1 },
    [0xBF] = { .func = op_cp_a_r, .name = "CP A", .cycles = 1, .length = 1 },
    [0xC0] = { .func = op_ret_cc, .name = "RET NZ", .cycles = 2, .length = 1 },
    [0xC1] = { .func = op_pop_rr, .name = "POP BC", .cycles = 3, .length = 1 },
    [0xC2] = { .func = op_jp_cc_nn, .name = "JP NZ,a16", .cycles = 3, .length = 3 },
    [0xC3] = { .func = op_jp_nn, .name = "JP a16", .cycles = 4, .length = 3 },
    [0xC4] = { .func = op_call_cc_nn, .name = "CALL NZ,a16", .cycles = 3, .length = 3 },
    [0xC5] = { .func = op_push_rr, .name = "PUSH BC", .cycles = 4, .length = 1 },
    [0xC6] = { .func = op_add_a_d8, .name = "ADD A,d8", .cycles = 2, .length = 2 },
    [0xC7] = { .func = NULL, .name = "RST 00H", .cycles = 4, .length = 1 },
    [0xC8] = { .func = op_ret_cc, .name = "RET Z", .cycles = 2, .length = 1 },
    [0xC9] = { .func = op_ret, .name = "RET", .cycles = 4, .length = 1 },
    [0xCA] = { .func = op_jp_cc_nn, .name = "JP Z,a16", .cycles = 3, .length = 3 },
    [0xCB] = { .func = NULL, .name = "PREFIX CB", .cycles = 0, .length = 1 },
    [0xCC] = { .func = op_call_cc_nn, .name = "CALL Z,a16", .cycles = 3, .length = 3 },
    [0xCD] = { .func = op_call_nn, .name = "CALL a16", .cycles = 6, .length = 3 },
    [0xCE] = { .func = op_adc_a_d8, .name = "ADC A,d8", .cycles = 2, .length = 2 },
    [0xCF] = { .func = NULL, .name = "RST 08H", .cycles = 4, .length = 1 },
    [0xD0] = { .func = op_ret_cc, .name = "RET NC", .cycles = 2, .length = 1 },
    [0xD1] = { .func = op_pop_rr, .name = "POP DE", .cycles = 3, .length = 1 },
    [0xD2] = { .func = op_jp_cc_nn, .name = "JP NC,a16", .cycles = 3, .length = 3 },
    [0xD3] = { .func = NULL, .name = "!!INVALID OPCODE!!", .cycles = 0, .length = 1 },
    [0xD4] = { .func = op_call_cc_nn, .name = "CALL NC,a16", .cycles = 3, .length = 3 },
    [0xD5] = { .func = op_push_rr, .name = "PUSH DE", .cycles = 4, .length = 1 },
    [0xD6] = { .func = op_sub_a_d8, .name = "SUB d8", .cycles = 2, .length = 2 },
    [0xD7] = { .func = NULL, .name = "RST 10H", .cycles = 4, .length = 1 },
    [0xD8] = { .func = op_ret_cc, .name = "RET C", .cycles = 2, .length = 1 },
    [0xD9] = { .func = op_reti, .name = "RETI", .cycles = 4, .length = 1 },
    [0xDA] = { .func = op_jp_cc_nn, .name = "JP C,a16", .cycles = 3, .length = 3 },
    [0xDB] = { .func = NULL, .name = "!!INVALID OPCODE!!", .cycles = 0, .length = 1 },
    [0xDC] = { .func = op_call_cc_nn, .name = "CALL C,a16", .cycles = 3, .length = 3 },
    [0xDD] = { .func = NULL, .name = "!!INVALID OPCODE!!", .cycles = 0, .length = 1 },
    [0xDE] = { .func = op_sbc_a_d8, .name = "SBC A,d8", .cycles = 2, .length = 2 },
    [0xDF] = { .func = NULL, .name = "RST 18H", .cycles = 4, .length = 1 },
    [0xE0] = { .func = NULL, .name = "LDH,(a8),A", .cycles = 3, .length = 2 },
    [0xE1] = { .func = op_pop_rr, .name = "POP HL", .cycles = 3, .length = 1 },
    [0xE2] = { .func = NULL, .name = "LDH (C),A", .cycles = 2, .length = 1 },
    [0xE3] = { .func = NULL, .name = "!!INVALID OPCODE!!", .cycles = 0, .length = 1 },
    [0xE4] = { .func = NULL, .name = "!!INVALID OPCODE!!", .cycles = 0, .length = 1 },
    [0xE5] = { .func = op_push_rr, .name = "PUSH HL", .cycles = 4, .length = 1 },
    [0xE6] = { .func = op_and_a_d8, .name = "AND d8", .cycles = 2, .length = 2 },
    [0xE7] = { .func = NULL, .name = "RST 20H", .cycles = 4, .length = 1 },
    [0xE8] = { .func = NULL, .name = "ADD SP,r8", .cycles = 4, .length = 2 },
    [0xE9] = { .func = op_jp_hl, .name = "JP HL", .cycles = 1, .length = 1 },
    [0xEA] = { .func = NULL, .name = "LD (a16),A", .cycles = 4, .length = 3 },
    [0xEB] = { .func = NULL, .name = "!!INVALID OPCODE!!", .cycles = 0, .length = 1 },
    [0xEC] = { .func = NULL, .name = "!!INVALID OPCODE!!", .cycles = 0, .length = 1 },
    [0xED] = { .func = NULL, .name = "!!INVALID OPCODE!!", .cycles = 0, .length = 1 },
    [0xEE] = { .func = op_xor_a_d8, .name = "XOR d8", .cycles = 2, .length = 2 },
    [0xEF] = { .func = NULL, .name = "RST 28H", .cycles = 4, .length = 1 },
    [0xF0] = { .func = NULL, .name = "LDH A,(a8)", .cycles = 3, .length = 2 },
    [0xF1] = { .func = op_pop_rr, .name = "POP AF", .cycles = 3, .length = 1 },
    [0xF2] = { .func = NULL, .name = "LDH A,(C)", .cycles = 2, .length = 1 },
    [0xF3] = { .func = NULL, .name = "DI", .cycles = 1, .length = 1 },
    [0xF4] = { .func = NULL, .name = "!!INVALID OPCODE!!", .cycles = 0, .length = 1 },
    [0xF5] = { .func = op_push_rr, .name = "PUSH AF", .cycles = 4, .length = 1 },
    [0xF6] = { .func = op_or_a_d8, .name = "OR d8", .cycles = 2, .length = 2 },
    [0xF7] = { .func = NULL, .name = "RST 30H", .cycles = 4, .length = 1 },
    [0xF8] = { .func = NULL, .name = "LD HL,SP+r8", .cycles = 3, .length = 2 },
    [0xF9] = { .func = NULL, .name = "LD SP,HL", .cycles = 2, .length = 1 },
    [0xFA] = { .func = NULL, .name = "LD A,(a16)", .cycles = 4, .length = 3 },
    [0xFB] = { .func = NULL, .name = "EI", .cycles = 1, .length = 1 },
    [0xFC] = { .func = NULL, .name = "!!INVALID OPCODE!!", .cycles = 0, .length = 1 },
    [0xFD] = { .func = NULL, .name = "!!INVALID OPCODE!!", .cycles = 0, .length = 1 },
    [0xFE] = { .func = op_cp_a_d8, .name = "CP d8", .cycles = 2, .length = 2 },
    [0xFF] = { .func = NULL, .name = "RST 38H", .cycles = 4, .length = 1 },
    
};

// Función auxiliar que muestra el estado de la CPU
void print_cpu_state(const Cpu* cpu) {
    printf("AF 0x%02X%02X ", cpu->A, cpu->F);
    printf("BC 0x%02X%02X ", cpu->B, cpu->C);
    printf("DE 0x%02X%02X ", cpu->D, cpu->E);
    printf("HL 0x%02X%02X ", cpu->H, cpu->L);

    printf("SP: 0x%04X ", cpu->SP);
    printf("PC: 0x%04X ", cpu->PC);

    // Mostrar flags de Z, N, H, C por separado
    printf("%c %c %c %c\t",
           (cpu->F & FLAG_Z) ? 'Z' : '.',
           (cpu->F & FLAG_N) ? 'N' : '.',
           (cpu->F & FLAG_H) ? 'H' : '.',
           (cpu->F & FLAG_C) ? 'C' : '.');

    printf("IME: %d ", cpu->ime);
    printf("Halted: %d\n", cpu->halted);
}

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
    // 1. Obtenemos el PC actual y el opcode
    u8 opcode = bus_read(gb, gb->cpu.PC);

    // 2. Buscamos la instrución en la tabla
    const Instruction* instr = &instruction_set[opcode];

    // 3. Avanzamos PC (consumimos el opcode)
    gb->cpu.PC++; 

    // 4. Inicializamos los cilos con el valor BASE de la tabla
    // Si la instrucción es condicional, la función sumará el extra de cilos
    // a esta variable.
    gb->cpu.cycles = instr->cycles;

    // 5. Ejecutamos la instrucción
    if (instr->func) {
        // Debug: Imprimir la instrucción que se va a ejecutar
        printf("0x%04X: %s (0x%02X)\n", gb->cpu.PC, instr->name, opcode);
        instr->func(gb);
        // Debug: Imprimir el estado de la CPU después de la instrucción
        print_cpu_state(&gb->cpu);
    } else {
        // Instrucción no implementada
        printf("Instrucción no implementada: %s (0x%02X) en PC:0x%04X\n", instr->name, opcode, gb->cpu.PC);
        exit(1);
    }

    // 6. Devolvemos el total acumulado para este paso
    return gb->cpu.cycles;
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

// Helper: Lee un valor de 16 bits del par de registros dado
// index: 0=BC, 1=DE, 2=HL, 3=SP
u16 read_register_pair(Cpu* cpu, int index) {
    switch (index) {
        case 0: // BC
            return ((u16)cpu->B << 8) | cpu->C;
        case 1: // DE
            return ((u16)cpu->D << 8) | cpu->E;
        case 2: // HL
            return ((u16)cpu->H << 8) | cpu->L;
        case 3: // SP
            return cpu->SP;
    }
    return 0; // Indicador de error
}

// Función NOP (No Operation)
void op_nop(GameBoy* gb) {
    // No hace nada
    // Evitar warnings de variables no usadas
    (void)gb;
}

// ---------------- LD r, r --------------------------------
// Para las cargas de 8 bits (LD r, r), el opcode sigue este patrón binario:
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
    // 1. Recuperamos opcode
    u8 opcode = bus_read(gb, gb->cpu.PC - 1);

    // 2. Extraemos índices (LD dst, src)
    // Patrón: 01 ddd sss
    u8 dst_idx = (opcode >> 3) & 0x07;  // Bits 5-3 son DESTINO
    u8 src_idx = opcode & 0x07;         // Bits 2-0 son ORIGEN

    // --- FASE 1: LECTURA DEL VALOR (Origen) ---
    u8 val;
    if (src_idx == 6) {
        // Origen es memoria (HL)
        u16 addr = get_hl(&gb->cpu);
        val = bus_read(gb, addr);
    }
    else {
        // Origen es registro
        val = *get_register_ptr(&gb->cpu, src_idx);
    }

    // --- FASE 2: ESCRITURA DEL VALOR (Destino) ---
    if (dst_idx == 6) {
        // Destino es memoria (HL)
        u16 addr = get_hl(&gb->cpu);
        bus_write(gb, addr, val);
    }
    else {
        // Destino es registro
        u8* dest_ptr = get_register_ptr(&gb->cpu, dst_idx);
        *dest_ptr = val;
    }
}

// ---------------- LD r, d8 -------------------------------
void op_ld_r_d8(GameBoy* gb) {
    // 1. Recuperamos el opcode 
    u8 opcode = bus_read(gb, gb->cpu.PC - 1);

    // 2. Extraemos destino (Bits 3-5)
    // Patrón: 00 rrr 110
    u8 reg_idx = (opcode >> 3) & 0x07;

    // 3. Leemos el valor inmediato (d8)
    u8 val = bus_read(gb, gb->cpu.PC);
    gb->cpu.PC++; // ¡Buena práctica! Avanzar el PC justo después de leer

    // 4. Escribimos el valor inmediato en  destino
    if (reg_idx == 6) { 
        // Escribir en (HL) 
        u16 addr = get_hl(&gb->cpu);
        bus_write(gb, addr, val);
    }
    else {
        // Escribir en registro
        u8* reg = get_register_ptr(&gb->cpu, reg_idx);

        // Un assert o check simple por seguridad
        if (reg) {
            *reg = val;
        }
    }
}

// Cargas de 16 bits
// ----------------- LD rr, d16 ----------------------------
void op_ld_rr_d16(GameBoy* gb) {
    // 1. Decodificación (PC apunta al byte siguiente al opcode)
    u8 opcode = bus_read(gb, gb->cpu.PC - 1);

    // Bits 4-5: Índice del par (0=BC, 1=DE, 2=HL, 3=SP)
    int reg_pair_index = (opcode >> 4) & 0x03;

    // 2. Lectura de 16 bits
    u16 value = bus_read16(gb, gb->cpu.PC);

    // 3. Escritura en registro
    write_register_pair(&gb->cpu, reg_pair_index, value);

    // 4. Avance del PC
    gb->cpu.PC += 2;
}

// LD (rr), A: Almacena el valor de A en la dirección apuntada por el par de registros rr
void op_ld_addr_rr_a(GameBoy* gb) {
    // 1. Recuperamos el opcode (PC ya avanzó en el bucle principal)
    u8 opcode = bus_read(gb, gb->cpu.PC - 1);

    // 2. Extraemos el índice del par de registros
    // Opcode: 00 rr 0010
    int reg_pair_index = (opcode >> 4) & 0x03; // Bits 5-4
    
    u16 addr = 0x0000;
    // 3. Obtenemos la dirección desde el par de registros
    // Caso especial 0x22 y 0x32 (LD (HL+), A y LD (HL-), A)
    if (reg_pair_index == 3) {
        addr = get_hl(&gb->cpu);
    }
    else {
        addr = read_register_pair(&gb->cpu, reg_pair_index);
    }
    
    // 4. Escribimos el valor de A en la dirección obtenida
    bus_write(gb, addr, gb->cpu.A);    
    
    // 5. Manejo de los casos especiales de auto-incremento/decremento
    if (reg_pair_index == 2) {
        // LD (HL+), A
        addr++;
        write_register_pair(&gb->cpu, 2, addr);
    }
    else if (reg_pair_index == 3) {
        // LD (HL-), A
        addr--;
        write_register_pair(&gb->cpu, 2, addr);
    }
    
}

// LD A, (rr): Carga en A el valor desde la dirección apuntada por el par de registros rr
void op_ld_a_addr_rr(GameBoy* gb) {
    // 1. Recuperamos el opcode (PC ya avanzó en el bucle principal)
    u8 opcode = bus_read(gb, gb->cpu.PC - 1);
    
    // 2. Extraemos el índice del par de registros
    // Opcode: 00 rr 1010
    int reg_pair_index = (opcode >> 4) & 0x03; // Bits 5-4

    u16 addr = 0x0000;
    // 3. Obtenemos la dirección desde el par de registros
    // Caso especial 0x3A, reg_pair_index == 3 devolvería SP, pero queremos HL
    if (reg_pair_index == 3) {
        addr = get_hl(&gb->cpu);
    }
    else {
        addr = read_register_pair(&gb->cpu, reg_pair_index);
    }

    // 4. Leemos el valor desde la dirección obtenida y lo cargamos en A
    gb->cpu.A = bus_read(gb, addr);

    // 5. Manejo de los casos especiales de auto-incremento/decremento
    if (reg_pair_index == 2) {
        // LD A, (HL+)
        addr++;
        write_register_pair(&gb->cpu, 2, addr);
    }
    else if (reg_pair_index == 3) {
        // LD A, (HL-)
        addr--;
        write_register_pair(&gb->cpu, 2, addr);
    }
}

void op_halt(GameBoy* gb) {
    gb->cpu.halted = true;
}

// -------------------------- INC r -------------------------
void op_inc_r(GameBoy* gb) {
    // 1. Decodificar registro (3-5)
    u8 opcode = bus_read(gb, gb->cpu.PC - 1);
    u8 reg_idx = (opcode >> 3) & 0x07;

    // 2. Obtener el valor actual
    u8 val;
    u8* reg_ptr = NULL;

    if (reg_idx == 6) { // INC (HL)
        u16 addr = (gb->cpu.H << 8)| gb->cpu.L;
        val = bus_read(gb, addr);
    }
    else {
        reg_ptr = get_register_ptr(&gb->cpu, reg_idx);
        val = *reg_ptr;
    }

    // 3. Calcular resultado
    u8 result = val + 1;

    // 4. GESTION DE FLAGS
    // Mantenemos Carry y borramos el resto de flags
    gb->cpu.F = (gb->cpu.F & FLAG_C);

    // Flag Z
    gb->cpu.F |= CHECK_ZERO(result);

    // Flag N: Siempre 0 en INC
    // (No hace falta hacer nada para este flag)

    // Flag H: Half Carry
    // Ocurre si pasamos de xxx1111 a xx10000.
    // Es decir, si los 4 bits bajos vaían 15 (0xF)
    gb->cpu.F |= ((val & 0x0F) == 0x0F) ? FLAG_H : 0;

    // 5. Escribir el resultado
    if (reg_idx == 6) {
        u16 addr = get_hl(&gb->cpu);
        bus_write(gb, addr, result);
    }
    else {
        *reg_ptr = result;
    }
}

// -------------------------- DEC r -------------------------
void op_dec_r(GameBoy* gb) {
    // 1. Decodificamos el registro objetivo (Bits 3-5)
    // Formato del opcode: 00 rrr 101
    u8 opcode = bus_read(gb, gb->cpu.PC - 1);
    int reg_idx = (opcode >> 3) & 0x07;
    
    // 2. Obtenemos el valor actual (target)
    u8 val;
    u8* reg_ptr = NULL;
    
    if (reg_idx == 6) {
        // Caso especial (HL): Leemos de memoria
        u16 addr = get_hl(&gb->cpu);
        val = bus_read(gb, addr);
    }
    else {
        // Caso registro directo
        reg_ptr = get_register_ptr(&gb->cpu, reg_idx);
        val = *reg_ptr;
    }

    // 3. Calculamos el resultado
    u8 result = val - 1;

    // 4. GESTIÓN DE FLAGS (¡Cuidado aquí!)
    // Conservamos el flag de C, borramos los otros 3
    gb->cpu.F = gb->cpu.F & FLAG_C;
    
    // Flag Z: Si el resultado es 0
    gb->cpu.F |= CHECK_ZERO(result);

    // Flag N: Siempre 1 (es una resta)
    gb->cpu.F |= FLAG_N;

    // Flag H: Half Carry
    gb->cpu.F |= ((val & 0x0F) == 0) ? FLAG_H : 0;
    
    // 5. Escribimos el resultado
    if (reg_idx == 6) {
        u16 addr = get_hl(&gb->cpu);
        bus_write(gb, addr, result);
    }
    else {
        *reg_ptr = result;
    }
}

// INC y DEC de pares de registros de 16 bits
void op_inc_dec_rr(GameBoy* gb) {
    // 1. Recuperamos el opcode (PC ya avanzó en el bucle principal)
    u8 opcode = bus_read(gb, gb->cpu.PC - 1);

    // 2. Extraemos el índice del par de registros
    // Formato del opcode: 
    //   INC 00 rr 0011 / DEC 00 rr 1011
    int reg_pair_index = (opcode >> 4) & 0x03; // Bits 5-4
    u16 value = read_register_pair(&gb->cpu, reg_pair_index);
    int8_t delta = (opcode & 0x08) ? -1 : 1; // DEC si bit 3 es 1, INC si es 0
    value += delta;
    write_register_pair(&gb->cpu, reg_pair_index, value);

    // INC/DEC de pares de registros no afecta flags
}

// Helper interno: Calcula flags para ADD y ADC
// Sirve para ADD (carry_in=0) y ADC (carry_in=flag_C)
static void set_add_adc_flags(GameBoy* gb, u8 val, u8 carry_in) {
    u8 a = gb->cpu.A;
    // Usamos int (o u16) para capturar el resultado completo (más de 255)
    int result = a + val + carry_in;

    // Lipiamos flags (ADD/ADC siempre ponen N a 0)
    gb->cpu.F = 0;

    // 1. Zero Flag (Z)
    gb->cpu.F |= CHECK_ZERO(result);

    // 2. Substract Flag (N)
    // Siempre es 0 en sumas. (Ya lo hicimos al limpiar F)

    // 3. Half Carry (H)
    gb->cpu.F |= CHECK_HALF_CARRY_ADD(a, val, carry_in);

    // 4. Carry Flag
    gb->cpu.F |= CHECK_CARRY_ADD(result);
}

// ------------------------ ADD ----------------------

// ADD A, r (Opcodes 0x80 - 0x87)
void op_add_a_r(GameBoy* gb) {
    // 1. Decodificar el registro origen
    u8 opcode = bus_read(gb, gb->cpu.PC - 1);
    u8 reg_idx = opcode & 0x07; // Bits 0-2
    u8 val;

    if (reg_idx == 6) { // Caso (HL)
        u16 addr = get_hl(&gb->cpu);
        val = bus_read(gb, addr);
    }
    else {
        val = *get_register_ptr(&gb->cpu, reg_idx);
    }

    // 2. Calcular flags (Carry in es 0)
    set_add_adc_flags(gb, val, 0);

    // 3. Ejecutar suma
    gb->cpu.A += val;
}

// ADD A, d8 (Opcode 0xC6)
void op_add_a_d8(GameBoy* gb) {
    // 1. Leer inmediato
    u8 val = bus_read(gb, gb->cpu.PC);
    gb->cpu.PC++; // Avanzar PC por el dato leído

    // 2. Calcular flags
    set_add_adc_flags(gb, val, 0);

    // 3. Ejecutar suma
    gb->cpu.A += val;
}

// ------------------------ ADC ----------------------

// ADC A, r (Opcodes 0x88 - 0x8F)
void op_adc_a_r(GameBoy* gb) {
    u8 opcode = bus_read(gb, gb->cpu.PC - 1);
    u8 reg_idx = opcode & 0x07;
    u8 val;

    if (reg_idx == 6) {
        u16 addr = get_hl(&gb->cpu);
        val = bus_read(gb, addr);
    }
    else {
        val = *get_register_ptr(&gb->cpu, reg_idx);
    }

    // Extraer Carry actual (0 o 1)
    u8 carry = (gb->cpu.F & FLAG_C) ? 1 : 0;

    // Calcular flags CON carry
    set_add_adc_flags(gb, val, carry);

    // Ejecutar suma completa
    gb->cpu.A += val + carry;
 }

 // ADC A, d8 (Opcode CE)
 void op_adc_a_d8(GameBoy* gb) {
    u8 val = bus_read(gb, gb->cpu.PC - 1);
    gb->cpu.PC++;

    u8 carry = (gb->cpu.F & FLAG_C) ? 1 : 0;

    set_add_adc_flags(gb, val, carry);

    gb->cpu.A += val + carry;
 }

// Helper interno: Calcula y actualiza flags para una resta (A - val - carry)
// Sirve para SUB (carry_in=0), CP (carry_in=0) y SBC (carry_in=flag_C)
static void set_sub_sbc_flags(GameBoy* gb, u8 val, u8 carry_in) {
    u8 a = gb->cpu.A;
    // Usamos int para capturar resultados negativos sin overflow
    int result = a - val - carry_in;

    gb->cpu.F = FLAG_N; // N siempre es 1 en restas

    // 1. Zero Flag
    gb->cpu.F |= CHECK_ZERO(result);
    
    // 2. Half Carry (H)
    gb->cpu.F |= CHECK_HALF_CARRY_SUB(gb->cpu.A, result, carry_in);

    // 3. Carry Flag (C)
    gb->cpu.F |= CHECK_CARRY_SUB(result);
}

// ------------------------ SUB ----------------------
// SUB A, r (Opcodes 0x90 - 0x97)
void op_sub_a_r(GameBoy* gb) {
    // 1. Decodificar registro origen
    u8 opcode = bus_read(gb, gb->cpu.PC - 1);
    u8 reg_idx = opcode & 0x07; // Bits 2-0
    u8 val;

    if (reg_idx == 6) { // Caso (HL)
        u16 addr = get_hl(&gb->cpu);
        val = bus_read(gb, addr);
    }
    else {
        val = *get_register_ptr(&gb->cpu, reg_idx);
    }

    // 2. Calcular flags (Carry in es 0 para SUB)
    set_sub_sbc_flags(gb, val, 0);

    // 3. Guardar resultado
    gb->cpu.A -= val;
}

// SUB A, d8 (Opcode 0xD6)
void op_sub_a_d8(GameBoy* gb) {
    // 1. Leer inmediato
    u8 val = bus_read(gb, gb->cpu.PC);
    gb->cpu.PC++; // Avanzamos PC por el dato leído

    // 2. Calcular flags
    set_sub_sbc_flags(gb, val, 0);

    // 3. Guardar resultado
    gb->cpu.A -= val;
}

// ------------------------ SBC -----------------------

// SBC A, r (Opcodes 0x90 - 0x9F)
void op_sbc_a_r(GameBoy* gb) {
    u8 opcode = bus_read(gb, gb->cpu.PC - 1);
    u8 reg_idx = opcode & 0x07;
    u8 val;

    if (reg_idx == 6) {
        u16 addr = get_hl(&gb->cpu);
        val = bus_read(gb, addr);
    }
    else {
        val = *get_register_ptr(&gb->cpu, reg_idx);
    }

    // EXTRAEMOS EL CARRY ACTUAL (0 o 1)
    u8 carry = gb->cpu.F ? 1 : 0;

    set_sub_sbc_flags(gb, val, carry);

    // Actualizamos el registro A con la resta con acarreo
    gb->cpu.A = gb->cpu.A - val - carry;
}

// SBC A, d8 (Opcode 0xDE)
void op_sbc_a_d8(GameBoy* gb) {
    u8 val = bus_read(gb, gb->cpu.PC);
    gb->cpu.PC++;

    u8 carry = gb->cpu.F ? 1 : 0;

    set_sub_sbc_flags(gb, val, carry);
    gb->cpu.A = gb->cpu.A - val - carry;
}

// ------------------------- CP -----------------------

// CP A, r (Opcodes 0xB8 - 0xBF)
void op_cp_a_r(GameBoy* gb) {
    u8 opcode = bus_read(gb, gb->cpu.PC - 1);
    u8 reg_idx = opcode & 0x07;
    u8 val;

    if (reg_idx == 6) {
        u16 addr = get_hl(&gb->cpu);
        val = bus_read(gb, addr);
    }
    else {
        val = *get_register_ptr(&gb->cpu, reg_idx);
    }

    // Solo calculamos flags, NO modificamos A
    set_sub_sbc_flags(gb, val, 0);
}

// CP A, d8 (Opcode 0xFE)
void op_cp_a_d8(GameBoy* gb) {
    u8 val = bus_read(gb, gb->cpu.PC);
    gb->cpu.PC++;

    set_sub_sbc_flags(gb, val, 0);
}

// Helper para AND, OR, XOR
// h_flag: 1 para AND, 0 para OR/XOR
static void set_logic_op_flags(GameBoy* gb, bool h_flag) {
    gb->cpu.F = 0; // N=0, C=0 siempre en estas ops

    // Flag Z: Se calcula sobre el registro A (que ya tiene el resultado)
    gb->cpu.F |= CHECK_ZERO(gb->cpu.A);

    // Flag H: Depende de la instrucción
    gb->cpu.F |= h_flag;
}

// ------------------------- AND ------------------------
void op_and_a_r(GameBoy* gb) {
    // 1. Fetch
    u8 opcode = bus_read(gb, gb->cpu.PC - 1);
    u8 reg_idx = opcode & 0x07;
    u8 val;

    if (reg_idx == 6) {
        u16 addr = get_hl(&gb->cpu);
        val = bus_read(gb, addr);
    }
    else {
        val = *get_register_ptr(&gb->cpu, reg_idx);
    }

    // 2. Operación
    gb->cpu.A &= val;

    // 3. Flags (AND pone H a 1)
    set_logic_op_flags(gb, FLAG_H);
}

void op_and_a_d8(GameBoy* gb) {
    u8 val = bus_read(gb, gb->cpu.PC);
    gb->cpu.PC++;

    gb->cpu.A &= val;
    set_logic_op_flags(gb, FLAG_H);
}

// ------------------------- OR ------------------------
void op_or_a_r(GameBoy* gb) {
    // 1. Fetch
    u8 opcode = bus_read(gb, gb->cpu.PC - 1);
    u8 reg_idx = opcode & 0x07;
    u8 val;

    if (reg_idx == 6) {
        u16 addr = get_hl(&gb->cpu);
        val = bus_read(gb, addr);
    }
    else {
        val = *get_register_ptr(&gb->cpu, reg_idx);
    }

    // 2. Operación
    gb->cpu.A |= val;

    // 3. Flags (AND pone H a 1)
    set_logic_op_flags(gb, 0);
}

void op_or_a_d8(GameBoy* gb) {
    u8 val = bus_read(gb, gb->cpu.PC);
    gb->cpu.PC++;

    gb->cpu.A |= val;
    set_logic_op_flags(gb, 0);
}

// ------------------------- XOR ------------------------
void op_xor_a_r(GameBoy* gb) {
    // 1. Fetch
    u8 opcode = bus_read(gb, gb->cpu.PC - 1);
    u8 reg_idx = opcode & 0x07;
    u8 val;

    if (reg_idx == 6) {
        u16 addr = get_hl(&gb->cpu);
        val = bus_read(gb, addr);
    }
    else {
        val = *get_register_ptr(&gb->cpu, reg_idx);
    }

    // 2. Operación
    gb->cpu.A ^= val;

    // 3. Flags (AND pone H a 1)
    set_logic_op_flags(gb, 0);
}

void op_xor_a_d8(GameBoy* gb) {
    u8 val = bus_read(gb, gb->cpu.PC);
    gb->cpu.PC++;

    gb->cpu.A ^= val;
    set_logic_op_flags(gb, 0);
}

// INC rr: Incrementa par de registros
void op_inc_rr(GameBoy* gb) {
    // Implementación de la instrucción INC rr
    // 1. Recuperamos el opcode (PC ya avanzó en el bucle principal)
    u8 opcode = bus_read(gb, gb->cpu.PC - 1);

    // 2. Extraemos el índice del par de registros
    // Formato del opcode: 00 rr 0011
    int reg_pair_index = (opcode >> 4) & 0x03; // Bits 5-4
    u16 value = (read_register_pair(&gb->cpu, reg_pair_index) + 1) & 0xFFFF;

    // 3. Incrementamos el valor del par de registros
    write_register_pair(&gb->cpu, reg_pair_index, value);

    // INC rr no afecta a los flags
}

// --------------------- PUSH rr -------------------------
void op_push_rr(GameBoy* gb) {
    // 1. Recuperamos el opcode (PC ya avanzó en el bucle principal)
    u8 opcode = bus_read(gb, gb->cpu.PC - 1);

    // Bits 4-5 determinan el par: 00=BC, 01=DE, 10=HL, 11=AF
    int reg_pair_idx = (opcode >> 4) & 0x03;

    u16 value;

    // 2. Obtener el valor del registro (Manejando el caso especial AF)
    if (reg_pair_idx == 3) {
        // Caso especial: PUSH AF
        // Combinamos A (Alto) y F (Bajo)
        value = (gb->cpu.A << 8) | gb->cpu.F; 
    }
    else {
        // Casos normales: BC, DE, HL
        value = read_register_pair(&gb->cpu, reg_pair_idx);
    }
    
    // 3. Escribir en la Pila (PUSH)
    // Orden: Primero HIGH byte, luego LOW byte. SP decrementa ANTES de escribir
    
    // Paso 1: Byte Alto
    gb->cpu.SP--;
    bus_write(gb, gb->cpu.SP, (value >> 8) & 0xFF);

    // Paso 2: Byte Bajo
    gb->cpu.SP--;
    bus_write(gb, gb->cpu.SP, value & 0xFF);
}

// --------------------- POP -----------------------------
void op_pop_rr(GameBoy* gb) {
    // 1. Decodificar
    u8 opcode = bus_read(gb, gb->cpu.PC - 1);
    int reg_pair_idx = (opcode >> 4) & 0x03;
    
    // 2. Leer de la Pila (POP)
    // Orden inverso a PUSH: Primero LOW byte, luego HIGH byte.

    // Paso 1: Byte Bajo
    u8 lo = bus_read(gb, gb->cpu.SP);
    gb->cpu.SP++;
    
    // Paso 2: Byte Alto
    u8 hi = bus_read(gb, gb->cpu.SP);
    gb->cpu.SP++;

    u16 value = (hi << 8)| lo;

    // 3. Escribir en el registro destino
    if (reg_pair_idx == 3) {
        // --- CASO ESPECIAL: POP AF ---
        gb->cpu.A = (value >> 8) & 0xFF; // Byte Alto -> A

        // ¡CRÍTICO! El registro F tiene los 4 bits bajos SIEMPRE a 0.
        gb->cpu.F = value & 0xF0;
    }
    else {
        // --- CASO NORMAL: BC, DE, HL ---
        write_register_pair(&gb->cpu, reg_pair_idx, value);
    }
}

// ---------------------- JP -----------------------------
// Las instrucciones condicionales (JP NZ, CALL Z, etc.) 
// usan siempre los bits 3 y 4 del opcode para indicar la condición
// 00: NZ (Not Zero)
// 01: Z  (Zero)
// 10: NC (Not Carry)
// 11: C  (Carry)

// Helper interno: Devuelve true si la condición se cumple
bool check_condition(GameBoy* gb, int condition_code) {
    u8 f = gb->cpu.F;
    switch (condition_code) {
        case 0: return !(f & FLAG_Z); // NZ (Not Zero)
        case 1: return (f & FLAG_Z);  // Z  (Zero)
        case 2: return !(f & FLAG_C); // NC (Not Carry)
        case 3: return (f & FLAG_C);  // C  (Carry)
    }
    return false;
}

// ------------------- JP nn (Incondicional) -----------------
void op_jp_nn(GameBoy* gb) {
    // 1. Leemos la dirección destino
    u16 target_addr = bus_read16(gb, gb->cpu.PC);

    // 2. Saltamos (sobreescribimos PC)
    gb->cpu.PC = target_addr;
}

// ---------------- JP cc, nn (Condicional) ------------------
void op_jp_cc_nn(GameBoy* gb) {
    u8 opcode = bus_read(gb, gb->cpu.PC);
    int cond = (opcode >> 3) && 0x03; // Bits 3-4

    // Siempre leemos los argumentos para avanzar el PC correctamente
    // si la condición NO se cumple.
    u16 target_addr = bus_read16(gb, gb->cpu.PC);
    gb->cpu.PC += 2; // Avanzamos por defecto (como si no saltáramos)

    if (check_condition(gb, cond)) {
        gb->cpu.PC = target_addr; // Si se cumple, saltamos
        gb->cpu.cycles += 1; // Coste extra si se toma el salto
    }
}

// ------------------- JP (HL) -> Opcode 0xE9 --------------------
// ¡CUIDADO! No lee memoria, salta a la dirección que conitene HL.
void op_jp_hl(GameBoy* gb) {
    u16 hl = get_hl(&gb->cpu);
    gb->cpu.PC = hl;
}

// ------------------------- JR ----------------------------------
// Usa un desplazamiento con signo de 8 bits (int8_t).
// Rango del salto: -128 a +127 bytes.

// Helper genérico para JR
void op_jr_common(GameBoy* gb, bool jump_taken) {
    // 1. Leemos el offset como SIGNED int8
    // Es vital el cast a (int8_t) para que C entienda que 0xFF es -1.
    int8_t offset = (int8_t)bus_read(gb, gb->cpu.PC);
    gb->cpu.PC++; // Consumimos el byte del offset

    if (jump_taken) {
        gb->cpu.PC += offset; // Suma con signo (puede restar)
    }
}

// --------------------- JR e (Incondicional) -----------------------
void op_jr_e(GameBoy* gb) {
    op_jr_common(gb, true);
}

// -------------------- JR cc, e (Condicional) -----------------------
// Opcodes 0x20, 0x28, 0x30, 0x38
void op_jr_cc_e(GameBoy* gb) {
    u8 opcode = bus_read(gb, gb->cpu.PC - 1);
    int cond = (opcode >> 3) & 0x03;

    bool jump_taken = check_condition(gb, cond); 
    op_jr_common(gb, jump_taken);

    if (jump_taken) gb->cpu.cycles += 1;
}

// ------------------------- CALL ----------------------------------

// Helper para guardar el PC actual en la pila
void push_pc(GameBoy* gb) {
    u16 return_addr = gb->cpu.PC;

    gb->cpu.SP--;
    u8 hi = (return_addr >> 8) & 0xFF;
    bus_write(gb, gb->cpu.SP, hi);

    u8 lo = (return_addr & 0xFF);
    bus_write(gb, gb->cpu.SP, lo);
}

// ------------------- CALL nn (Incondicional ) ---------------------
// Opcode 0xCD
void op_call_nn(GameBoy* gb) {
    u16 target_addr = bus_read16(gb, gb->cpu.PC);
    gb->cpu.PC += 2; // El PC ahora apunta a la instrucción SIGUIENTE (Retorno)

    push_pc(gb);    // Guardamos esta dirección de retorno
    gb->cpu.PC = target_addr; // Saltamos
}

// ------------------- CALL cc, nn (Condicional) ---------------------
// Opcodes 0cC4, 0xCC, 0xD4, 0xDC
void op_call_cc_nn(GameBoy* gb) {
    u8 opcode = bus_read(gb, gb->cpu.PC - 1);
    int cond = (opcode >> 3) & 0x03;

    u16 target_adddr = bus_read16(gb, gb->cpu.PC);
    gb->cpu.PC += 2; // Preparamos el PC para continuar si NO saltamos

    if (check_condition(gb, cond)) {
        push_pc(gb); // Solo hacemos PUSH si la condición se cumple
        gb->cpu.PC = target_adddr;
        gb->cpu.cycles += 3; // Coste extra si se toma el salto
    }
}

// ------------------------- RET ----------------------------------
// Helper para restaurar el PC de la pila

// RET 
// Opcode: 0xC9
void op_ret(GameBoy* gb) {
    // Byte bajo primero, byte alto después
    u8 lo = bus_read(gb, gb->cpu.SP);
    gb->cpu.SP++;

    u8 hi = bus_read(gb, gb->cpu.SP);
    gb->cpu.SP++;

    // 2. Actualizar PC
    gb->cpu.PC = (hi << 8) | lo;
}

// RET cc (Condicional)
// Opcodes: 0xC0, 0xC8, 0xD0, 0xD8
void op_ret_cc(GameBoy* gb) {
    // 1. Decodificamos la condición (bits 3 y 4)
    u8 opcode = bus_read(gb, gb->cpu.PC - 1);
    int cond = (opcode >> 3) & 0x03;

    // 2. Verificamos si se cumple
    if (check_condition(gb, cond)) {
        // Hacemos exactamente RET
        op_ret(gb);

        // SUMAMOS LA PENALIZACIÓN
        // Ciclos totales necesarios: 5.
        // Base en tabla: 2.
        // Extra a sumar: 3.
        gb->cpu.cycles += 3;
    }
    
    // Si la condición es falsa, no hacemos nada.
    // El PC simplemente sigue en la instrucción siguiente al RET.
}

// RETI (Return from interrupt)
// Opcode 0xD9
void op_reti(GameBoy* gb) {
    // 1. Exactamente igual que RET
    op_ret(gb);

    // 2. Habilitar Interrupciones Maestras
    gb->cpu.ime = true;
}