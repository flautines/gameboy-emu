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

// Tabla de instrucciones (completa con todas las instrucciones)
Instruction instruction_set[256] = {
    [0x00] = { .func = op_nop, .name = "NOP", .cycles = 1, .length = 1 },
    [0x01] = { .func = op_ld_rr_d16, .name = "LD BC,d16", .cycles = 3, .length = 3 },
    [0x02] = { .func = op_ld_addr_rr_a, .name = "LD (BC),A", .cycles = 2, .length = 1 },
    [0x03] = { .func = op_inc_rr, .name = "INC BC", .cycles = 2, .length = 1 },
    [0x04] = { .func = op_inc_r, .name = "INC B", .cycles = 1, .length = 1 },
    [0x05] = { .func = op_dec_r, .name = "DEC B", .cycles = 1, .length = 1 },
    [0x06] = { .func = op_ld_r_d8, .name = "LD B,d8", .cycles = 2, .length = 2 },
    [0x07] = { .func = op_rlca, .name = "RLCA", .cycles = 1, .length = 1 },
    [0x08] = { .func = op_ld_a16_sp, .name = "LD (a16),SP", .cycles = 5, .length = 3 },
    [0x09] = { .func = op_add_hl_rr, .name = "ADD HL,BC", .cycles = 2, .length = 1 },
    [0x0A] = { .func = op_ld_a_addr_rr, .name = "LD A,(BC)", .cycles = 2, .length = 1 },
    [0x0B] = { .func = op_dec_rr, .name = "DEC BC", .cycles = 2, .length = 1 },
    [0x0C] = { .func = op_inc_r, .name = "INC C", .cycles = 1, .length = 1 },
    [0x0D] = { .func = op_dec_r, .name = "DEC C", .cycles = 1, .length = 1 },
    [0x0E] = { .func = op_ld_r_d8, .name = "LD C,d8", .cycles = 2, .length = 2 },
    [0x0F] = { .func = op_rrca, .name = "RRCA", .cycles = 1, .length = 1 },
    [0x10] = { .func = op_stop, .name = "STOP", .cycles = 2, .length = 2 },
    [0x11] = { .func = op_ld_rr_d16, .name = "LD DE,d16", .cycles = 3, .length = 3 },
    [0x12] = { .func = op_ld_addr_rr_a, .name = "LD (DE),A", .cycles = 2, .length = 1 },
    [0x13] = { .func = op_inc_rr, .name = "INC DE", .cycles = 2, .length = 1 },
    [0x14] = { .func = op_inc_r, .name = "INC D", .cycles = 1, .length = 1 },
    [0x15] = { .func = op_dec_r, .name = "DEC D", .cycles = 1, .length = 1 },
    [0x16] = { .func = op_ld_r_d8, .name = "LD D,d8", .cycles = 2, .length = 2 },
    [0x17] = { .func = op_rla, .name = "RLA", .cycles = 1, .length = 1 },
    [0x18] = { .func = op_jr_e, .name = "JR r8", .cycles = 3, .length = 2 },
    [0x19] = { .func = op_add_hl_rr, .name = "ADD HL,DE", .cycles = 2, .length = 1 },
    [0x1A] = { .func = op_ld_a_addr_rr, .name = "LD A,(DE)", .cycles = 2, .length = 1 },
    [0x1B] = { .func = op_dec_rr, .name = "DEC DE", .cycles = 2, .length = 1 },
    [0x1C] = { .func = op_inc_r, .name = "INC E", .cycles = 1, .length = 1 },
    [0x1D] = { .func= op_dec_r, .name = "DEC E", .cycles = 1, .length = 1 },
    [0x1E] = { .func= op_ld_r_d8, .name = "LD E,d8", .cycles = 2, .length = 2 },
    [0x1F] = { .func= op_rra, .name = "RRA", .cycles = 1, .length = 1 },
    [0x20] = { .func = op_jr_cc_e, .name = "JR NZ,r8", .cycles = 2, .length = 2 },
    [0x21] = { .func = op_ld_rr_d16, .name = "LD HL,d16", .cycles = 3, .length = 3 },
    [0x22] = { .func = op_ld_addr_rr_a, .name = "LD (HL+),A", .cycles = 2, .length = 1 },
    [0x23] = { .func = op_inc_rr, .name = "INC HL", .cycles = 2, .length = 1 },
    [0x24] = { .func = op_inc_r, .name = "INC H", .cycles = 1, .length = 1 },
    [0x25] = { .func = op_dec_r, .name = "DEC H", .cycles = 1, .length = 1 },
    [0x26] = { .func = op_ld_r_d8, .name = "LD H,d8", .cycles = 2, .length = 2 },
    [0x27] = { .func = op_daa, .name = "DAA", .cycles = 1, .length = 1 },
    [0x28] = { .func = op_jr_cc_e, .name = "JR Z,r8", .cycles = 2, .length = 2 },
    [0x29] = { .func = op_add_hl_rr, .name = "ADD HL,HL", .cycles = 2, .length = 1 },
    [0x2A] = { .func = op_ld_a_addr_rr, .name = "LD A,(HL+)", .cycles = 2, .length = 1 },
    [0x2B] = { .func = op_dec_rr, .name = "DEC HL", .cycles = 2, .length = 1 },
    [0x2C] = { .func = op_inc_r, .name = "INC L", .cycles = 1, .length = 1 },
    [0x2D] = { .func = op_dec_r, .name = "DEC L", .cycles = 1, .length = 1 },
    [0x2E] = { .func= op_ld_r_d8, .name= "LD L,d8", .cycles = 2, .length = 2 },
    [0x2F] = { .func= op_cpl, .name= "CPL", .cycles = 1, .length = 1 },
    [0x30] = { .func = op_jr_cc_e, .name = "JR NC,r8", .cycles = 2, .length = 2 },
    [0x31] = { .func = op_ld_rr_d16, .name = "LD SP,d16", .cycles = 3, .length = 3 },
    [0x32] = { .func = op_ld_addr_rr_a, .name = "LD (HL-),A", .cycles = 2, .length = 1 },
    [0x33] = { .func = op_inc_rr, .name = "INC SP", .cycles = 2, .length = 1 },
    [0x34] = { .func = op_inc_r, .name = "INC (HL)", .cycles = 3, .length = 1 },
    [0x35] = { .func = op_dec_r, .name = "DEC (HL)", .cycles = 3, .length = 1 },
    [0x36] = { .func = op_ld_r_d8, .name = "LD (HL),d8", .cycles = 3, .length = 2 },
    [0x37] = { .func = op_scf, .name = "SCF", .cycles = 1, .length = 1 },
    [0x38] = { .func = op_jr_cc_e, .name = "JR C,r8", .cycles = 2, .length = 2 },
    [0x39] = { .func = op_add_hl_rr, .name = "ADD HL,SP", .cycles = 2, .length = 1 },
    [0x3A] = { .func = op_ld_a_addr_rr, .name = "LD A,(HL-)", .cycles = 2, .length = 1 },
    [0x3B] = { .func = op_dec_rr, .name = "DEC SP", .cycles = 2, .length = 1 },
    [0x3C] = { .func = op_inc_r, .name = "INC A", .cycles = 1, .length = 1 },
    [0x3D] = { .func= op_dec_r, .name= "DEC A", .cycles = 1, .length = 1 },
    [0x3E] = { .func= op_ld_r_d8, .name= "LD A,d8", .cycles = 2, .length = 2 },
    [0x3F] = { .func= op_ccf, .name= "CCF", .cycles = 1, .length = 1 },
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
    [0xC7] = { .func = op_rst, .name = "RST 00H", .cycles = 4, .length = 1 },
    [0xC8] = { .func = op_ret_cc, .name = "RET Z", .cycles = 2, .length = 1 },
    [0xC9] = { .func = op_ret, .name = "RET", .cycles = 4, .length = 1 },
    [0xCA] = { .func = op_jp_cc_nn, .name = "JP Z,a16", .cycles = 3, .length = 3 },
    [0xCB] = { .func = NULL, .name = "PREFIX CB", .cycles = 0, .length = 1 },
    [0xCC] = { .func = op_call_cc_nn, .name = "CALL Z,a16", .cycles = 3, .length = 3 },
    [0xCD] = { .func = op_call_nn, .name = "CALL a16", .cycles = 6, .length = 3 },
    [0xCE] = { .func = op_adc_a_d8, .name = "ADC A,d8", .cycles = 2, .length = 2 },
    [0xCF] = { .func = op_rst, .name = "RST 08H", .cycles = 4, .length = 1 },
    [0xD0] = { .func = op_ret_cc, .name = "RET NC", .cycles = 2, .length = 1 },
    [0xD1] = { .func = op_pop_rr, .name = "POP DE", .cycles = 3, .length = 1 },
    [0xD2] = { .func = op_jp_cc_nn, .name = "JP NC,a16", .cycles = 3, .length = 3 },
    [0xD3] = { .func = NULL, .name = "!!INVALID OPCODE!!", .cycles = 0, .length = 1 },
    [0xD4] = { .func = op_call_cc_nn, .name = "CALL NC,a16", .cycles = 3, .length = 3 },
    [0xD5] = { .func = op_push_rr, .name = "PUSH DE", .cycles = 4, .length = 1 },
    [0xD6] = { .func = op_sub_a_d8, .name = "SUB d8", .cycles = 2, .length = 2 },
    [0xD7] = { .func = op_rst, .name = "RST 10H", .cycles = 4, .length = 1 },
    [0xD8] = { .func = op_ret_cc, .name = "RET C", .cycles = 2, .length = 1 },
    [0xD9] = { .func = op_reti, .name = "RETI", .cycles = 4, .length = 1 },
    [0xDA] = { .func = op_jp_cc_nn, .name = "JP C,a16", .cycles = 3, .length = 3 },
    [0xDB] = { .func = NULL, .name = "!!INVALID OPCODE!!", .cycles = 0, .length = 1 },
    [0xDC] = { .func = op_call_cc_nn, .name = "CALL C,a16", .cycles = 3, .length = 3 },
    [0xDD] = { .func = NULL, .name = "!!INVALID OPCODE!!", .cycles = 0, .length = 1 },
    [0xDE] = { .func = op_sbc_a_d8, .name = "SBC A,d8", .cycles = 2, .length = 2 },
    [0xDF] = { .func = op_rst, .name = "RST 18H", .cycles = 4, .length = 1 },
    [0xE0] = { .func = op_ldh_a8_a, .name = "LDH,(a8),A", .cycles = 3, .length = 2 },
    [0xE1] = { .func = op_pop_rr, .name = "POP HL", .cycles = 3, .length = 1 },
    [0xE2] = { .func = op_ldh_c_a, .name = "LDH (C),A", .cycles = 2, .length = 1 },
    [0xE3] = { .func = NULL, .name = "!!INVALID OPCODE!!", .cycles = 0, .length = 1 },
    [0xE4] = { .func = NULL, .name = "!!INVALID OPCODE!!", .cycles = 0, .length = 1 },
    [0xE5] = { .func = op_push_rr, .name = "PUSH HL", .cycles = 4, .length = 1 },
    [0xE6] = { .func = op_and_a_d8, .name = "AND d8", .cycles = 2, .length = 2 },
    [0xE7] = { .func = op_rst, .name = "RST 20H", .cycles = 4, .length = 1 },
    [0xE8] = { .func = op_add_sp_r8, .name = "ADD SP,r8", .cycles = 4, .length = 2 },
    [0xE9] = { .func = op_jp_hl, .name = "JP HL", .cycles = 1, .length = 1 },
    [0xEA] = { .func = op_ld_a16_a, .name = "LD (a16),A", .cycles = 4, .length = 3 },
    [0xEB] = { .func = NULL, .name = "!!INVALID OPCODE!!", .cycles = 0, .length = 1 },
    [0xEC] = { .func = NULL, .name = "!!INVALID OPCODE!!", .cycles = 0, .length = 1 },
    [0xED] = { .func = NULL, .name = "!!INVALID OPCODE!!", .cycles = 0, .length = 1 },
    [0xEE] = { .func = op_xor_a_d8, .name = "XOR d8", .cycles = 2, .length = 2 },
    [0xEF] = { .func = op_rst, .name = "RST 28H", .cycles = 4, .length = 1 },
    [0xF0] = { .func = op_ldh_a_a8, .name = "LDH A,(a8)", .cycles = 3, .length = 2 },
    [0xF1] = { .func = op_pop_rr, .name = "POP AF", .cycles = 3, .length = 1 },
    [0xF2] = { .func = op_ldh_a_c, .name = "LDH A,(C)", .cycles = 2, .length = 1 },
    [0xF3] = { .func = op_di, .name = "DI", .cycles = 1, .length = 1 },
    [0xF4] = { .func = NULL, .name = "!!INVALID OPCODE!!", .cycles = 0, .length = 1 },
    [0xF5] = { .func = op_push_rr, .name = "PUSH AF", .cycles = 4, .length = 1 },
    [0xF6] = { .func = op_or_a_d8, .name = "OR d8", .cycles = 2, .length = 2 },
    [0xF7] = { .func = op_rst, .name = "RST 30H", .cycles = 4, .length = 1 },
    [0xF8] = { .func = op_ld_hl_sp_r8, .name = "LD HL,SP+r8", .cycles = 3, .length = 2 },
    [0xF9] = { .func = op_ld_sp_hl, .name = "LD SP,HL", .cycles = 2, .length = 1 },
    [0xFA] = { .func = op_ld_a_addr, .name = "LD A,(a16)", .cycles = 4, .length = 3 },
    [0xFB] = { .func = op_ei, .name = "EI", .cycles = 1, .length = 1 },
    [0xFC] = { .func = NULL, .name = "!!INVALID OPCODE!!", .cycles = 0, .length = 1 },
    [0xFD] = { .func = NULL, .name = "!!INVALID OPCODE!!", .cycles = 0, .length = 1 },
    [0xFE] = { .func = op_cp_a_d8, .name = "CP d8", .cycles = 2, .length = 2 },
    [0xFF] = { .func = op_rst, .name = "RST 38H", .cycles = 4, .length = 1 },
    
};

// Helpers para leer/escribir pares de registros
// Nota: GameBoy es Little Endian, pero los registros pares se leen High-Low.
// BC -> B es el byte alto, C es el byte bajo.
static inline u16 get_bc(Cpu* cpu) { return (cpu->b << 8) | cpu->c; }
static inline void set_bc(Cpu* cpu, u16 value) { cpu->b = (value >> 8) & 0xFF; cpu->c = value & 0xFF; }
static inline u16 get_de(Cpu* cpu) { return (cpu->d << 8) | cpu->e; }
static inline void set_de(Cpu* cpu, u16 value) { cpu->d = (value >> 8) & 0xFF; cpu->e = value & 0xFF; }
static inline u16 get_hl(Cpu* cpu) { return (cpu->h << 8) | cpu->l; }
static inline void set_hl(Cpu* cpu, u16 value) { cpu->h = (value >> 8) & 0xFF; cpu->l = value & 0xFF; }

// Función auxiliar que muestra el estado de la CPU
void print_cpu_state(const Cpu* cpu) {
    printf("AF 0x%02X%02X ", cpu->a, cpu->f);
    printf("BC 0x%02X%02X ", cpu->b, cpu->c);
    printf("DE 0x%02X%02X ", cpu->d, cpu->e);
    printf("HL 0x%02X%02X ", cpu->h, cpu->l);

    printf("SP: 0x%04X ", cpu->sp);
    printf("PC: 0x%04X ", cpu->pc);

    // Mostrar flags de Z, N, H, C por separado
    printf("%c %c %c %c\t",
           (cpu->f & FLAG_Z) ? 'Z' : '.',
           (cpu->f & FLAG_N) ? 'N' : '.',
           (cpu->f & FLAG_H) ? 'H' : '.',
           (cpu->f & FLAG_C) ? 'C' : '.');

    printf("IME: %d ", cpu->ime);
    printf("Halted: %d\n", cpu->halted);
}

// Inicializa la CPU a su estado por defecto
void cpu_init(Cpu* cpu) {
    // Estado post-boot ROM
    cpu->pc = 0x0100; // Punto de entrada de los cartuchos
    cpu->sp = 0xFFFE; // Puntero de pila inicial

    // Valores iniciales de los registros tras el boot ROM
    cpu->a = 0x01;
    cpu->f = 0xB0;
    cpu->b = 0x00;
    cpu->c = 0x13;
    cpu->d = 0x00;
    cpu->e = 0xD8;
    cpu->h = 0x01;
    cpu->l = 0x4D;

    // Estado interno
    cpu->ime = false;    // Interrupts deshabilitados
    cpu->halted = false; // No en modo halt
}

// Retorna el número de M-Cycles consumidos por la instrucción ejecutada
int cpu_step(GameBoy* gb) {
    // --- MODO STOP (Hibernación) ---
    if (gb->cpu.stopped) {
        // En hardware real, nada avanza.
        // No devolvemos ciclos (o devolvemos 0), por lo que Timers, Audio
        // y Video NO deben recibir actualizaciones de tiempo.
        return 0;
    }

    // Si estamos en HALT, no ejecutamos nada, solo consumimos tiempo
    if (gb->cpu.halted) {
        // CPU dormida, consume 1 M-Cycle por paso
        return 1;
    }

    // --- MANEJO DEL HALT BUG ---
    u16 pc_actual = gb->cpu.pc;
 
    // Obtenemos el PC actual y el opcode
    u8 opcode = bus_read(gb, pc_actual);

    // Si el bug ocurrió en la instrucción anterior:
    if (gb->cpu.halt_bug) {
        // NO incrementamos el PC.
        // El opcode se ejecuta, pero PC sigue apuntando al mismo sitio.
        // Esto causará que el siguiente byte se lea "dos veces" (o corrupto).
        gb->cpu.halt_bug = false;
    } 
    else {
        // Comportamiento normal: PC avanza
        gb->cpu.pc++; 
    }

    // Buscamos la instrución en la tabla
    const Instruction* instr = &instruction_set[opcode];

    // Inicializamos los cilos con el valor BASE de la tabla
    // Si la instrucción es condicional, la función sumará el extra de cilos
    // a esta variable.
    gb->cpu.cycles = instr->cycles;

    // 6. Ejecutamos la instrucción
    if (instr->func) {
        // Debug: Imprimir la instrucción que se va a ejecutar
        printf("0x%04X: %s (0x%02X)\n", gb->cpu.pc, instr->name, opcode);
        instr->func(gb);
        // Debug: Imprimir el estado de la CPU después de la instrucción
        print_cpu_state(&gb->cpu);
    } else {
        // Instrucción no implementada
        printf("Instrucción no implementada: %s (0x%02X) en PC:0x%04X\n", instr->name, opcode, gb->cpu.pc);
        exit(1);
    }

    // 6. Devolvemos el total acumulado para este paso
    return gb->cpu.cycles;
}

// Los dispositivos usarán esta función para solicitar una interrupción
void cpu_request_interrupt(GameBoy* gb, u8 type) {
    // Activamos el bit correspondiente en IF
    gb->cpu.if_reg |= type;

    // Si la CPU estaba dormida (HALT), ¡despierta!
    // Nota: Esto es independiente de IME o IE. Un HALT siempre se rompe
    // si ocurre una interrupción, aunque luego no se ejecute si está bloqueada.
    gb->cpu.halted = false;
}

// TODO: Esta función todavía no gestiona toda la lógica de interrupciones
// solo hace las acciones para gestionar el flag halted.
void handle_interrupts(GameBoy* gb) {
    // Si hay alguna interrupción pendiente (IF) y habilitada (IE)
    if (gb->cpu.ie & gb->cpu.if_reg & 0x1F) {

        // ¡DESPIERTA!
        // Esto saca a la CPU del bucle de HALT
        gb->cpu.halted = false;

        // Solo saltamos al vector (0x40, 0x48...) si IME está activo
        if (gb->cpu.ime) {
            // TODO: ... lógica de saltar al vector de interrupción ...
        }
    }
}

// Helper para obtener punteros a registros según el índice (0-7)
u8* get_register_ptr(Cpu* cpu, int index) {
    switch (index) {
        case 0: return &cpu->b;
        case 1: return &cpu->c;
        case 2: return &cpu->d;
        case 3: return &cpu->e;
        case 4: return &cpu->h;
        case 5: return &cpu->l;
        case 6: return NULL; // No hay registro HL
        case 7: return &cpu->a;
    }
    return NULL; // Indicador de error
}

// Helper: Escribe un valor de 16 bits en el par de registros dado
// index: 0=BC, 1=DE, 2=HL, 3=SP
void write_register_pair(GameBoy* gb, int index, u16 value) {
    switch (index) {
        case 0: // BC
            gb->cpu.b = (value >> 8) & 0xFF;
            gb->cpu.c = value & 0xFF;
            break;
        case 1: // DE
            gb->cpu.d = (value >> 8) & 0xFF;
            gb->cpu.e = value & 0xFF;
            break;
        case 2: // HL
            gb->cpu.h = (value >> 8) & 0xFF;
            gb->cpu.l = value & 0xFF;
            break;
        case 3: // SP
            gb->cpu.sp = value;
            break;
    }
}

// Helper: Lee un valor de 16 bits del par de registros dado
// index: 0=BC, 1=DE, 2=HL, 3=SP
u16 get_register_pair(GameBoy* gb, int index) {
    switch (index) {
        case 0: // BC
            return ((u16)gb->cpu.b << 8) | gb->cpu.c;
        case 1: // DE
            return ((u16)gb->cpu.d << 8) | gb->cpu.e;
        case 2: // HL
            return ((u16)gb->cpu.h << 8) | gb->cpu.l;
        case 3: // SP
            return gb->cpu.sp;
    }
    return 0; // Indicador de error
}

// ---------------------- IMPLEMENTACIÓN DE LAS INSTRUCCIONES -----------------

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
    u8 opcode = bus_read(gb, gb->cpu.pc - 1);

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
    u8 opcode = bus_read(gb, gb->cpu.pc - 1);

    // 2. Extraemos destino (Bits 3-5)
    // Patrón: 00 rrr 110
    u8 reg_idx = (opcode >> 3) & 0x07;

    // 3. Leemos el valor inmediato (d8)
    u8 val = bus_read(gb, gb->cpu.pc);
    gb->cpu.pc++; // ¡Buena práctica! Avanzar el PC justo después de leer

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
    u8 opcode = bus_read(gb, gb->cpu.pc - 1);

    // Bits 4-5: Índice del par (0=BC, 1=DE, 2=HL, 3=SP)
    int reg_pair_index = (opcode >> 4) & 0x03;

    // 2. Lectura de 16 bits
    u16 value = bus_read16(gb, gb->cpu.pc);

    // 3. Escritura en registro
    write_register_pair(gb, reg_pair_index, value);

    // 4. Avance del PC
    gb->cpu.pc += 2;
}

// ------------------- LD (rr), A ----------------------------------
// Escribe A en la dirección apuntada por BC, DE o HL (con inc/dec)
// Opcodes: 0x02, 0x12, 0x22, 0x32
void op_ld_addr_rr_a(GameBoy* gb) {
    // 1. Recuperamos opcode
    u8 opcode = bus_read(gb, gb->cpu.pc - 1);

    // 2. Extraemos el índice (Bits 4-5)
    RegisterPairIndex idx = (RegisterPairIndex)((opcode >> 4) & 0x03);

    u16 addr = 0;

    // 3. Lógica específica por registro
    switch (idx) {
        case REG_PAIR_BC: // 0x02
            addr = get_register_pair(gb, REG_PAIR_BC);
        break;

        case REG_PAIR_DE: // 0x12
            addr = get_register_pair(gb, REG_PAIR_DE);
        break;

        case REG_PAIR_HL: // 0x22: LD (HL+), A (También llamado LDI (HL), A)
            addr = get_register_pair(gb, REG_PAIR_HL);
            // Incrementamos HL después de obtener la dirección original
            write_register_pair(gb, REG_PAIR_HL, addr + 1);
        break;

        case REG_PAIR_SP: // 0x32: LD (HL-), A (También llamado LDD (HL), A)
            // ¡OJO! A nivel de bits del opcode es un '3' (lo que sería SP normalmente)
            // pero esta instrucción ESPECÍFICA lo interpreta como "HL con Decremento"
            addr = get_register_pair(gb, REG_PAIR_HL); // Leer HL
            write_register_pair(gb, REG_PAIR_HL, addr - 1);
        break;

        default:
            // No debería llegar aquí, pero así hacemos feliz al compilador
            printf("Error crítico: Índice de par inválido %d\n", idx);
            return;
    }

    // 4. Escribimos A en memoria
    bus_write(gb, addr, gb->cpu.a);
}
 
// ------------------- LD A, (rr) ----------------------------------
// Carga en A el valor de memoria apuntado por BC, DE o HL (con inc/dec)
// Opcodes: 0x0A, 0x1A, 0x2A, 0x3A
void op_ld_a_addr_rr(GameBoy* gb) {
    // 1. Recuperamos el opcode
    u8 opcode = bus_read(gb, gb->cpu.pc - 1);

    // 2. Extraemos el índice del par (Bits 4-5)
    RegisterPairIndex idx = (RegisterPairIndex)((opcode >> 4) & 0x03);

    u16 addr = 0;

    // 3. Obtenemos la dirección y aplicamos efectos secundarios (HL+/-)
    switch(idx) {
        case REG_PAIR_BC: // 0x02
            addr = get_register_pair(gb, REG_PAIR_BC);
            break;

        case REG_PAIR_DE: // 0x12
            addr = get_register_pair(gb, REG_PAIR_DE);
            break;

        case REG_PAIR_HL: // 0x22: LD (HL+), A
            addr = get_register_pair(gb, REG_PAIR_HL);
            // Efecto secundario: Incremento
            write_register_pair(gb, REG_PAIR_HL, addr + 1);
            break;

        case REG_PAIR_SP: // 0x32: LD (HL-), A
            // El índice binario 3 se interpreta como "HL con Decremento"
            addr = get_register_pair(gb, REG_PAIR_HL);

            // Efecto secundario: Decremento
            write_register_pair(gb, REG_PAIR_HL, addr - 1);
            break;

        default:
            // No debería llegar aquí, pero así hacemos feliz al compilador
            printf("Error crítico: Índice de par inválido en LD A, (rr): %d\n", idx);
            return;
    }

    // 4. Lectura de memoria hacia el registro A
    gb->cpu.a = bus_read(gb, addr);
}

//  ------------------- LD (a16),SP --------------------------------
// Opcode 0x08
void op_ld_a16_sp(GameBoy* gb) {
    // 1. Leemos la dirección donde queremos guardar el SP
    // (bbus_read16 lee PC y PC+1 correspondiente en Little Endian)
    u16 addr = bus_read16(gb, gb->cpu.pc);

    // 2. Avanzamos el PC
    // Hemos consumido 2 bytes de operandos (la dirección a16).
    // así que debemos saltarlos.
    gb->cpu.pc += 2;

    // 3. Guardamos SP en esa dirección
    bus_write16(gb, addr, gb->cpu.sp);
}

// -------------------- LD (a16), A --------------------------------
// Opcode 0xEA
void op_ld_a16_a(GameBoy* gb) {
    // 1. La dirección de destino está en los dos bytes siguientes al opcode
    u16 address = bus_read16(gb, gb->cpu.pc);

    // Al leer el valor inmediato de 16-bits el PC se ha incrementado en +2
    gb->cpu.pc += 2;

    // 2. Escribe el valor de A en la dirección de memoria
    bus_write(gb, address, gb->cpu.a);
}

// Función helper para obtener dirección 0xFF00 + offset inmediato (2 bytes)
static inline uint16_t get_ldh_address(GameBoy* gb)
{
    // 1. Leer el offet (PC apunta al byte siguiente al opcode)
    u8 offset = bus_read(gb, gb->cpu.pc);

    // 2. Avanzamos el PC 1 byte (hemos consumido el offset)
    gb->cpu.pc++;

    // 2. Calcular dirección (High Memory: 0xFF00 + offset)
    return (0xFF00 | offset);
}

// ------------------ LDH (a8), A ----------------------------------
// Opcode 0xE0
void op_ldh_a8_a(GameBoy* gb) {
    u16 address = get_ldh_address(gb);
    bus_write(gb, address, gb->cpu.a);
}

// ------------------- LDH A, (a8) ----------------------------------
// Opcode 0xF0
void op_ldh_a_a8(GameBoy* gb)
{
    u16 address = get_ldh_address(gb);
    gb->cpu.a = bus_read(gb, address);
}

// ------------------- LD A, (a16) ----------------------------------
// Opcode 0xFA
// Lee el byte en la dirección absoluta dada y lo guarda en A
void op_ld_a_addr(GameBoy* gb)
{
    // 1. Leer dirección de 16 bits y ajustar PC
    u16 addr = bus_read16(gb, gb->cpu.pc);
    gb->cpu.pc+=2;

    // 2. Leer valor desde esa dirección y guardarlo en A
    gb->cpu.a = bus_read(gb, addr);
}

// ----------------------- LDH (C), A -----------------------------
// Opcode 0xE2
void op_ldh_c_a(GameBoy* gb)
{
    u16 addr = 0xFF00 | gb->cpu.c;
    bus_write(gb, addr, gb->cpu.a);
}

// ----------------------- LDH A, (C) -----------------------------
// Opcode 0xF2
void op_ldh_a_c(GameBoy* gb)
{
    u16 addr = 0xFF00 | gb->cpu.c;
    gb->cpu.a = bus_read(gb, addr);
}

// ------------------------ LD SP, HL -----------------------------
// Opcode 0xF9
void op_ld_sp_hl(GameBoy* gb)
{
    u16 hl = get_register_pair(gb, REG_PAIR_HL);
    gb->cpu.sp = hl;
}

// ------------------ DI (Disable Interrupts) ---------------------
// Opcode 0xF3
void op_di(GameBoy* gb)
{
    gb->cpu.ime = false;
}
// ------------------ EI (Enable Interrupts) -----------------------------
// Opcode 0xFB
void op_ei(GameBoy* gb)
{
    gb->cpu.ime = true;
}

// ------------------- HALT ----------------------------------------
// La GameBoy original (DMG) tenía un bug: 
// Si se ejecuta HALT cuando:
//  1. Las interrupciones maestras están desactivadas (IME = 0)
//  2. PERO hay interrupciones pendientes habilitadas ( IE & IF & 0x1F != 0)
//
// El resultado: La CPU NO se detiene, pero debido a un fallo en el circuito de pre-fetch,
// la siguiente instrucción SE LEE DOS VECES.
// Muchos juegos (como The Legend of Zelda: Link's Awekening) usan este truco.
void op_halt(GameBoy* gb) {
    // Obtenemos las interrupciones pendientes que también están habilitadas
    u8 pending_interrupts = gb->cpu.ie & gb->cpu.if_reg & 0x1F;

    if (gb->cpu.ime) {
        // CASO NORMAL A: IME activado.
        // La CPU se duerme esperando una interrupción.
        gb->cpu.halted = true;
    }
    else {
        if (pending_interrupts == 0) {
            // CASO NORMAL B: IME desactivado y sin interrupciones pendientes.
            // La CPU se duerme, pero no saltará al vector de interrupción al despertar
            // (simplemente continuará ejecución en la siguiente línea).
            gb->cpu.halted = true;
        }
        else {
            // CASO HALT BUG: IME desactivado PERO hay interrupciones pendientes.
            // La CPU NO se duerme (halted = false).
            // Y ocurre el glitch: el PC falla al incrementar en la siguiente lectura.
            gb->cpu.halt_bug = true;
        }
    }
}

// ------------------- STOP ----------------------------------------
// Opcode 0x10
// Longitud efectiva: 2 bytes (0x10 + el byte 0x00 obligatorio)
void op_stop(GameBoy* gb) {
    // 1. Consumimos el byte "dummy" siguiente
    // El hardware espera que después de 0x10 venga un 0x00.
    // Lo leemos y avanzamos el PC para no ejecutar basura al despertar.
    bus_read(gb, gb->cpu.pc);
    gb->cpu.pc++;

    // 2. Activamos el modo STOP
    gb->cpu.stopped = true;

    // 3. TODO: Quirk de Hardware (DMG): Reset del DIV
    // Al entrar en STOP, el divisor interno del Timer se reinicia.
}

// -------------------------- INC r -------------------------
void op_inc_r(GameBoy* gb) {
    // 1. Decodificar registro (3-5)
    u8 opcode = bus_read(gb, gb->cpu.pc - 1);
    u8 reg_idx = (opcode >> 3) & 0x07;

    // 2. Obtener el valor actual
    u8 val;
    u8* reg_ptr = NULL;

    if (reg_idx == 6) { // INC (HL)
        u16 addr = (gb->cpu.h << 8)| gb->cpu.l;
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
    gb->cpu.f = (gb->cpu.f & FLAG_C);

    // Flag Z
    gb->cpu.f |= CHECK_ZERO(result);

    // Flag N: Siempre 0 en INC
    // (No hace falta hacer nada para este flag)

    // Flag H: Half Carry
    // Ocurre si pasamos de xxx1111 a xx10000.
    // Es decir, si los 4 bits bajos vaían 15 (0xF)
    gb->cpu.f |= ((val & 0x0F) == 0x0F) ? FLAG_H : 0;

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
    u8 opcode = bus_read(gb, gb->cpu.pc - 1);
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
    gb->cpu.f = gb->cpu.f & FLAG_C;
    
    // Flag Z: Si el resultado es 0
    gb->cpu.f |= CHECK_ZERO(result);

    // Flag N: Siempre 1 (es una resta)
    gb->cpu.f |= FLAG_N;

    // Flag H: Half Carry
    gb->cpu.f |= ((val & 0x0F) == 0) ? FLAG_H : 0;
    
    // 5. Escribimos el resultado
    if (reg_idx == 6) {
        u16 addr = get_hl(&gb->cpu);
        bus_write(gb, addr, result);
    }
    else {
        *reg_ptr = result;
    }
}

// --------------------- INC rr (16 bits) --------------------------
// Opcodes: 0x03, 0x13, 0x23, 0x33
void op_inc_rr(GameBoy* gb) {
    u8 opcode = bus_read(gb, gb->cpu.pc - 1);

    // Bits 4-5: Registro (BC, DE, HL, SP)
    int reg_idx = (opcode >> 4) & 0x03;

    u16 val = get_register_pair(gb, reg_idx);

    // Incremento simple (el desbordamiento de 0xFFFF a 0x0000 es automático en u16)
    val++;

    write_register_pair(gb, reg_idx, val);
}

// ---------------------- DEC rr (16 bits) ------------------------
void op_dec_rr(GameBoy* gb) {
    u8 opcode = bus_read(gb, gb->cpu.pc - 1);
    int reg_idx = (opcode >> 4) & 0x03;

    u16 val = get_register_pair(gb, reg_idx);

    // Decremento simple
    val--;

    write_register_pair(gb, reg_idx, val);
}

// ---------------------- ADD HL, rr (16 bits) --------------------
// Opcodes: 0x09, 0x19, 0x29, 0x39
void op_add_hl_rr(GameBoy* gb) {
    // 1. Recuperamos opcode
    u8 opcode = bus_read(gb, gb->cpu.pc - 1);

    // 2. Identificamos el registro fuente (BC, DE, HL, SP)
    // Bits 4-5: 00-BC, 01=DE, 10=HL, 11=SP
    RegisterPairIndex src_idx = (RegisterPairIndex)((opcode >> 4) & 0x03);

    // 3. Obtenemos valors
    u16 hl_val = get_register_pair(gb, REG_PAIR_HL);
    u16 rr_val = get_register_pair(gb, src_idx);

    // Usamos uint32_t para capturar el carry
    u32 result = hl_val + rr_val;

    // 4. GESTIÓN DE FLAGS

    // Flag N: Siempre 0
    gb->cpu.f &= ~FLAG_N;

    // Flag H: Half Carry en el bit 11
    // Comprobamos si la suma de los 12 bits bajos desborda
    if (((hl_val & 0x0FFF) + (rr_val & 0x0FFF)) > 0x0FFF) {
        gb->cpu.f |= FLAG_H;
    }
    else {
        gb->cpu.f &= ~FLAG_H;
    }

    // Flag Z: NO SE MODIFICA.
    // Mantenemos el valor que tuviera antes.

    // 5. Guardar el resultado (truncado a 16 bits)
    write_register_pair(gb, REG_PAIR_HL, (u16)result);
}

// =============================================================
// ROTACIONES DE ACUMULADOR (Z siempre es 0)
// =============================================================

// --------------------------- RLCA ----------------------------
// Rotate Left Circular Accumulator - Opcode 0x07
// Bit 7 -> Carry Y Bit 0
void op_rlca(GameBoy* gb) {
    u8 a = gb->cpu.a;
    u8 bit7 = (a >> 7) & 1; // Extraemos el bit que sale

    // Rotamos y metemos el bit 7 en la posición 0
    gb->cpu.a = (a << 1) | bit7;

    // FLAGS:
    // Z: Siempre 0 (Diferencia clave con CB RLC)
    // N: 0, H: 0
    // C: Copia del bit 7
    gb->cpu.f = 0;
    if (bit7) gb->cpu.f |= FLAG_C;
}

// --------------------------- RRCA ----------------------------
// Rotate Right Circular Accumulator - Opcode 0x0F
// Bit 0 -> Carry Y Bit 7
void op_rrca(GameBoy* gb) {
    u8 a = gb->cpu.a;
    u8 bit0 = a & 1; // Extraemos el bit que sale

    // Rotamos y metemos el bit 0 en la posición 7
    gb->cpu.a = (a >> 1) | (bit0 << 7);

    // FLAGS. Z=0, N=0, C=bit0
    gb->cpu.f = 0;
    if (bit0) gb->cpu.f |= FLAG_C;
}

// ---------------------------  RLA ----------------------------
// Rotate Left Accumulator through Carry - Opcode 0x17
// Carry antiguo -> Bit 0, Bit 7 -> Nuevo Carry
void op_rla(GameBoy* gb) {
    u8 a = gb->cpu.a;
    u8 bit7 = (a >> 7) & 1; // Lo que será el nuevo Carry
    u8 old_carry = (gb->cpu.f & FLAG_C) ? 1 : 0; // Lo que entra

    // Rotamos e inyectamos el carry antiguo
    gb->cpu.a = (a << 1) | old_carry;

    // FLAGS: Z=0, N=0, H=0, C=bit7
    gb->cpu.f = 0;
    if (bit7) gb->cpu.f |= FLAG_C;
}

// ---------------------------  RRA ----------------------------
// Rotate Right Accumulator through Carry - Opcode 0x1F
// Carry antiguo -> Bit 7, Bit 0 -> Nuevo Carry
void op_rra(GameBoy* gb) {
    u8 a = gb->cpu.a;
    u8 bit0 = a & 1; // Lo que será el nuevo Carry
    u8 old_carry = (gb->cpu.f & FLAG_C) ? 1 : 0; // Lo que entra

    // Rotamos e inyectamos el carry antiguo en la posición 7
    gb->cpu.a = (a >> 1) | (old_carry << 7);

    // FLAGS: Z=0, N=0, H=0, C=bit0
    gb->cpu.f = 0;
    if (bit0) gb->cpu.f |= FLAG_C;
}

// --------------------------- SCF ----------------------------
// Set Carry Flag - Opcode 0x37
void op_scf(GameBoy* gb) {
    // 1. Poner Carry a 1
    gb->cpu.f |= FLAG_C;

    // 2. N y H siempre a 0
    gb->cpu.f &= ~FLAG_N;
    gb->cpu.f &= ~FLAG_H;
}

// --------------------------- CCF ----------------------------
// Complement Carry Flag - Opcode 0x3F
void op_ccf(GameBoy* gb) {
    // 1. Invertir Carry (XOR es la forma más rápida de hacer toggle)
    gb->cpu.f ^= FLAG_C;

    // 2. N y H siempre a 0
    gb->cpu.f &= ~FLAG_N;
    gb->cpu.f &= ~FLAG_H;
}

// --------------------------- DAA ----------------------------
// Decimal Adjust Acumulator - Opcode 0x27
void op_daa(GameBoy* gb) {
    u8 a = gb->cpu.a;
    u16 correction = 0; // Usamos u16 para detectar overflows si fuera necesario

    bool flag_n = gb->cpu.f & FLAG_N;
    bool flag_h = gb->cpu.f & FLAG_H;
    bool flag_c = gb->cpu.f & FLAG_C;

    // 1. Determinar la corrección necesaria
    // -------------------------------------

    // CASO A: Ajuste del nibble BAJO (digitos 0-9)
    // Si hubo Half Carry o el nibble bajo es mayor que 9, sumamos 6.
    // (Ej: 0x0A + 0x06 = 0x10 -> Se "pasa" el 1 al nibble alto)
    if (flag_h || (!flag_n && (a & 0x0F) > 9)) {
        correction |= 0x06;
    }

    // CASE B: Ajuste del nibble ALTO (digitos 0-90)
    // Si hubo Carry o el número total es > 0x99, sumamos 60.
    if (flag_c || (!flag_n && a > 0x99)) {
        correction |= 0x60;

        // DAA activa el Carry permanentemente si hubo correción alta
        flag_c = true;
    }

    // 2. Aplicar la corrección
    // -------------------------------------
    if (flag_n) {
        // Si la operación anterior fue RESTA, restamos la corrección
        gb->cpu.a -= correction;
    }
    else {
        // Si fue SUMA, sumamos la corrección
        gb->cpu.a += correction;
    }

    // 3. Actualizar Flags
    // ---------------------------------------


    // Flag Z: Se actualiza según el nuevo valor de A
    if (gb->cpu.a == 0) gb->cpu.f |= FLAG_Z;
    else gb->cpu.f &= ~FLAG_Z;

    // Flag H: DAA siempre limpia el flag H
    gb->cpu.f &= ~FLAG_H;

    // Flag C: Se actualiza según la lógica calculada en el caso B
    if (flag_c) gb->cpu.f |= FLAG_C;
    else gb->cpu.f &= ~FLAG_C;

    // Flag N: NO se toca (se mantiene el valor que tenía)
}

// --------------------------- CPL ----------------------------
// Complement Accumulator - Opcode 0x2F
void op_cpl(GameBoy* gb) {
    // 1. Invertir bis de A
    gb->cpu.a = ~(gb->cpu.a);

    // 2. Gestionar Flags
    // CPL fuerza N y H a 1.
    gb->cpu.f |= FLAG_H;
    gb->cpu.f |= FLAG_N;

    // Z y C se mantienen intactos
}

// ===============================================================
//                        INSTRUCCIONES ALU
// ===============================================================
// Helper interno: Calcula flags para ADD y ADC
// Sirve para ADD (carry_in=0) y ADC (carry_in=flag_C)
static void set_add_adc_flags(GameBoy* gb, u8 val, u8 carry_in) {
    u8 a = gb->cpu.a;
    // Usamos int (o u16) para capturar el resultado completo (más de 255)
    int result = a + val + carry_in;

    // Lipiamos flags (ADD/ADC siempre ponen N a 0)
    gb->cpu.f = 0;

    // 1. Zero Flag (Z)
    gb->cpu.f |= CHECK_ZERO(result);

    // 2. Substract Flag (N)
    // Siempre es 0 en sumas. (Ya lo hicimos al limpiar F)

    // 3. Half Carry (H)
    gb->cpu.f |= CHECK_HALF_CARRY_ADD(a, val, carry_in);

    // 4. Carry Flag
    gb->cpu.f |= CHECK_CARRY_ADD(result);
}

// ------------------------ ADD ----------------------

// ADD A, r (Opcodes 0x80 - 0x87)
void op_add_a_r(GameBoy* gb) {
    // 1. Decodificar el registro origen
    u8 opcode = bus_read(gb, gb->cpu.pc - 1);
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
    gb->cpu.a += val;
}

// ADD A, d8 (Opcode 0xC6)
void op_add_a_d8(GameBoy* gb) {
    // 1. Leer inmediato
    u8 val = bus_read(gb, gb->cpu.pc);
    gb->cpu.pc++; // Avanzar PC por el dato leído

    // 2. Calcular flags
    set_add_adc_flags(gb, val, 0);

    // 3. Ejecutar suma
    gb->cpu.a += val;
}

// ------------------------ ADC ----------------------

// ADC A, r (Opcodes 0x88 - 0x8F)
void op_adc_a_r(GameBoy* gb) {
    u8 opcode = bus_read(gb, gb->cpu.pc - 1);
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
    u8 carry = (gb->cpu.f & FLAG_C) ? 1 : 0;

    // Calcular flags CON carry
    set_add_adc_flags(gb, val, carry);

    // Ejecutar suma completa
    gb->cpu.a += val + carry;
 }

 // ADC A, d8 (Opcode CE)
 void op_adc_a_d8(GameBoy* gb) {
    u8 val = bus_read(gb, gb->cpu.pc - 1);
    gb->cpu.pc++;

    u8 carry = (gb->cpu.f & FLAG_C) ? 1 : 0;

    set_add_adc_flags(gb, val, carry);

    gb->cpu.a += val + carry;
 }

// Helper para calcular (SP + r8) y gestionar flags correspondientes
static u16 add_sp_offset_logic(GameBoy* gb)
{
    // 1. Leer offset con signo
    int8_t offset = (int8_t)bus_read(gb, gb->cpu.pc);
    gb->cpu.pc++;

    u16 sp = gb->cpu.sp;

    // 2. Cálculo de Flags (Basado en el byte bajo)
    // Se usa casting a int para evitar promoción automática incorrecta
    int result = (sp & 0xFF) + (u8)offset;

    gb->cpu.f = 0; // Z y N siempre a 0

    // Carry en bit 8 (paso de 0xFF)
    if (result > 0xFF) gb->cpu.f |= FLAG_C;

    // Half Carry en bit 4 (paso de 0x0F)
    if (((sp & 0x0F) + (offset & 0x0F)) > 0x0F) gb->cpu.f |= FLAG_H;

    // 3. Devolver resultado final de 16 bits
    return sp + offset;
}

// ------------------------ ADD SP, r8 ----------------------
// Opcode 0xE8
void op_add_sp_r8(GameBoy* gb)
{
    gb->cpu.sp = add_sp_offset_logic(gb);
}

//------------------------ LD HL, SP+r8 ----------------------
// Opcode 0xF8
void op_ld_hl_sp_r8(GameBoy* gb)
{
    u16 res = add_sp_offset_logic(gb);
    write_register_pair(gb, REG_PAIR_HL, res);
}

// Helper interno: Calcula y actualiza flags para una resta (A - val - carry)
// Sirve para SUB (carry_in=0), CP (carry_in=0) y SBC (carry_in=flag_C)
static void set_sub_sbc_flags(GameBoy* gb, u8 val, u8 carry_in) {
    u8 a = gb->cpu.a;
    // Usamos int para capturar resultados negativos sin overflow
    int result = a - val - carry_in;

    gb->cpu.f = FLAG_N; // N siempre es 1 en restas

    // 1. Zero Flag
    gb->cpu.f |= CHECK_ZERO(result);
    
    // 2. Half Carry (H)
    gb->cpu.f |= CHECK_HALF_CARRY_SUB(gb->cpu.a, result, carry_in);

    // 3. Carry Flag (C)
    gb->cpu.f |= CHECK_CARRY_SUB(result);
}

// ------------------------ SUB ----------------------
// SUB A, r (Opcodes 0x90 - 0x97)
void op_sub_a_r(GameBoy* gb) {
    // 1. Decodificar registro origen
    u8 opcode = bus_read(gb, gb->cpu.pc - 1);
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
    gb->cpu.a -= val;
}

// SUB A, d8 (Opcode 0xD6)
void op_sub_a_d8(GameBoy* gb) {
    // 1. Leer inmediato
    u8 val = bus_read(gb, gb->cpu.pc);
    gb->cpu.pc++; // Avanzamos PC por el dato leído

    // 2. Calcular flags
    set_sub_sbc_flags(gb, val, 0);

    // 3. Guardar resultado
    gb->cpu.a -= val;
}

// ------------------------ SBC -----------------------

// SBC A, r (Opcodes 0x90 - 0x9F)
void op_sbc_a_r(GameBoy* gb) {
    u8 opcode = bus_read(gb, gb->cpu.pc - 1);
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
    u8 carry = gb->cpu.f ? 1 : 0;

    set_sub_sbc_flags(gb, val, carry);

    // Actualizamos el registro A con la resta con acarreo
    gb->cpu.a = gb->cpu.a - val - carry;
}

// SBC A, d8 (Opcode 0xDE)
void op_sbc_a_d8(GameBoy* gb) {
    u8 val = bus_read(gb, gb->cpu.pc);
    gb->cpu.pc++;

    u8 carry = gb->cpu.f ? 1 : 0;

    set_sub_sbc_flags(gb, val, carry);
    gb->cpu.a = gb->cpu.a - val - carry;
}

// ------------------------- CP -----------------------

// CP A, r (Opcodes 0xB8 - 0xBF)
void op_cp_a_r(GameBoy* gb) {
    u8 opcode = bus_read(gb, gb->cpu.pc - 1);
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
    u8 val = bus_read(gb, gb->cpu.pc);
    gb->cpu.pc++;

    set_sub_sbc_flags(gb, val, 0);
}

// Helper para AND, OR, XOR
// h_flag: 1 para AND, 0 para OR/XOR
static void set_logic_op_flags(GameBoy* gb, bool h_flag) {
    gb->cpu.f = 0; // N=0, C=0 siempre en estas ops

    // Flag Z: Se calcula sobre el registro A (que ya tiene el resultado)
    gb->cpu.f |= CHECK_ZERO(gb->cpu.a);

    // Flag H: Depende de la instrucción
    gb->cpu.f |= h_flag;
}

// ------------------------- AND ------------------------
void op_and_a_r(GameBoy* gb) {
    // 1. Fetch
    u8 opcode = bus_read(gb, gb->cpu.pc - 1);
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
    gb->cpu.a &= val;

    // 3. Flags (AND pone H a 1)
    set_logic_op_flags(gb, FLAG_H);
}

void op_and_a_d8(GameBoy* gb) {
    u8 val = bus_read(gb, gb->cpu.pc);
    gb->cpu.pc++;

    gb->cpu.a &= val;
    set_logic_op_flags(gb, FLAG_H);
}

// ------------------------- OR ------------------------
void op_or_a_r(GameBoy* gb) {
    // 1. Fetch
    u8 opcode = bus_read(gb, gb->cpu.pc - 1);
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
    gb->cpu.a |= val;

    // 3. Flags (AND pone H a 1)
    set_logic_op_flags(gb, 0);
}

void op_or_a_d8(GameBoy* gb) {
    u8 val = bus_read(gb, gb->cpu.pc);
    gb->cpu.pc++;

    gb->cpu.a |= val;
    set_logic_op_flags(gb, 0);
}

// ------------------------- XOR ------------------------
void op_xor_a_r(GameBoy* gb) {
    // 1. Fetch
    u8 opcode = bus_read(gb, gb->cpu.pc - 1);
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
    gb->cpu.a ^= val;

    // 3. Flags (AND pone H a 1)
    set_logic_op_flags(gb, 0);
}

void op_xor_a_d8(GameBoy* gb) {
    u8 val = bus_read(gb, gb->cpu.pc);
    gb->cpu.pc++;

    gb->cpu.a ^= val;
    set_logic_op_flags(gb, 0);
}

// --------------------- PUSH rr -------------------------
void op_push_rr(GameBoy* gb) {
    // 1. Recuperamos el opcode (PC ya avanzó en el bucle principal)
    u8 opcode = bus_read(gb, gb->cpu.pc - 1);

    // Bits 4-5 determinan el par: 00=BC, 01=DE, 10=HL, 11=AF
    int reg_pair_idx = (opcode >> 4) & 0x03;

    u16 value;

    // 2. Obtener el valor del registro (Manejando el caso especial AF)
    if (reg_pair_idx == 3) {
        // Caso especial: PUSH AF
        // Combinamos A (Alto) y F (Bajo)
        value = (gb->cpu.a << 8) | gb->cpu.f; 
    }
    else {
        // Casos normales: BC, DE, HL
        value = get_register_pair(gb, reg_pair_idx);
    }
    
    // 3. Escribir en la Pila (PUSH)
    // Orden: Primero HIGH byte, luego LOW byte. SP decrementa ANTES de escribir
    
    // Paso 1: Byte Alto
    gb->cpu.sp--;
    bus_write(gb, gb->cpu.sp, (value >> 8) & 0xFF);

    // Paso 2: Byte Bajo
    gb->cpu.sp--;
    bus_write(gb, gb->cpu.sp, value & 0xFF);
}

// --------------------- POP -----------------------------
void op_pop_rr(GameBoy* gb) {
    // 1. Decodificar
    u8 opcode = bus_read(gb, gb->cpu.pc - 1);
    int reg_pair_idx = (opcode >> 4) & 0x03;
    
    // 2. Leer de la Pila (POP)
    // Orden inverso a PUSH: Primero LOW byte, luego HIGH byte.

    // Paso 1: Byte Bajo
    u8 lo = bus_read(gb, gb->cpu.sp);
    gb->cpu.sp++;
    
    // Paso 2: Byte Alto
    u8 hi = bus_read(gb, gb->cpu.sp);
    gb->cpu.sp++;

    u16 value = (hi << 8)| lo;

    // 3. Escribir en el registro destino
    if (reg_pair_idx == 3) {
        // --- CASO ESPECIAL: POP AF ---
        gb->cpu.a = (value >> 8) & 0xFF; // Byte Alto -> A

        // ¡CRÍTICO! El registro F tiene los 4 bits bajos SIEMPRE a 0.
        gb->cpu.f = value & 0xF0;
    }
    else {
        // --- CASO NORMAL: BC, DE, HL ---
        write_register_pair(gb, reg_pair_idx, value);
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
    u8 f = gb->cpu.f;
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
    u16 target_addr = bus_read16(gb, gb->cpu.pc);

    // 2. Saltamos (sobreescribimos PC)
    gb->cpu.pc = target_addr;
}

// ---------------- JP cc, nn (Condicional) ------------------
void op_jp_cc_nn(GameBoy* gb) {
    u8 opcode = bus_read(gb, gb->cpu.pc);
    int cond = (opcode >> 3) && 0x03; // Bits 3-4

    // Siempre leemos los argumentos para avanzar el PC correctamente
    // si la condición NO se cumple.
    u16 target_addr = bus_read16(gb, gb->cpu.pc);
    gb->cpu.pc += 2; // Avanzamos por defecto (como si no saltáramos)

    if (check_condition(gb, cond)) {
        gb->cpu.pc = target_addr; // Si se cumple, saltamos
        gb->cpu.cycles += 1; // Coste extra si se toma el salto
    }
}

// ------------------- JP (HL) -> Opcode 0xE9 --------------------
// ¡CUIDADO! No lee memoria, salta a la dirección que conitene HL.
void op_jp_hl(GameBoy* gb) {
    u16 hl = get_hl(&gb->cpu);
    gb->cpu.pc = hl;
}

// ------------------------- JR ----------------------------------
// Usa un desplazamiento con signo de 8 bits (int8_t).
// Rango del salto: -128 a +127 bytes.

// Helper genérico para JR
void op_jr_common(GameBoy* gb, bool jump_taken) {
    // 1. Leemos el offset como SIGNED int8
    // Es vital el cast a (int8_t) para que C entienda que 0xFF es -1.
    int8_t offset = (int8_t)bus_read(gb, gb->cpu.pc);
    gb->cpu.pc++; // Consumimos el byte del offset

    if (jump_taken) {
        gb->cpu.pc += offset; // Suma con signo (puede restar)
    }
}

// --------------------- JR e (Incondicional) -----------------------
void op_jr_e(GameBoy* gb) {
    op_jr_common(gb, true);
}

// -------------------- JR cc, e (Condicional) -----------------------
// Opcodes 0x20, 0x28, 0x30, 0x38
void op_jr_cc_e(GameBoy* gb) {
    u8 opcode = bus_read(gb, gb->cpu.pc - 1);
    int cond = (opcode >> 3) & 0x03;

    bool jump_taken = check_condition(gb, cond); 
    op_jr_common(gb, jump_taken);

    if (jump_taken) gb->cpu.cycles += 1;
}

// ------------------------- CALL ----------------------------------

// Helper para guardar el PC actual en la pila
void push_pc(GameBoy* gb) {
    u16 return_addr = gb->cpu.pc;

    gb->cpu.sp--;
    u8 hi = (return_addr >> 8) & 0xFF;
    bus_write(gb, gb->cpu.sp, hi);

    gb->cpu.sp--;
    u8 lo = (return_addr & 0xFF);
    bus_write(gb, gb->cpu.sp, lo);
}

// ------------------- CALL nn (Incondicional ) ---------------------
// Opcode 0xCD
void op_call_nn(GameBoy* gb) {
    u16 target_addr = bus_read16(gb, gb->cpu.pc);
    gb->cpu.pc += 2; // El PC ahora apunta a la instrucción SIGUIENTE (Retorno)

    push_pc(gb);    // Guardamos esta dirección de retorno
    gb->cpu.pc = target_addr; // Saltamos
}

// ------------------- CALL cc, nn (Condicional) ---------------------
// Opcodes 0cC4, 0xCC, 0xD4, 0xDC
void op_call_cc_nn(GameBoy* gb) {
    u8 opcode = bus_read(gb, gb->cpu.pc - 1);
    int cond = (opcode >> 3) & 0x03;

    u16 target_adddr = bus_read16(gb, gb->cpu.pc);
    gb->cpu.pc += 2; // Preparamos el PC para continuar si NO saltamos

    if (check_condition(gb, cond)) {
        push_pc(gb); // Solo hacemos PUSH si la condición se cumple
        gb->cpu.pc = target_adddr;
        gb->cpu.cycles += 3; // Coste extra si se toma el salto
    }
}

// ------------------------- RET ----------------------------------
// Helper para restaurar el PC de la pila

// RET 
// Opcode: 0xC9
void op_ret(GameBoy* gb) {
    // Byte bajo primero, byte alto después
    u8 lo = bus_read(gb, gb->cpu.sp);
    gb->cpu.sp++;

    u8 hi = bus_read(gb, gb->cpu.sp);
    gb->cpu.sp++;

    // 2. Actualizar PC
    gb->cpu.pc = (hi << 8) | lo;
}

// RET cc (Condicional)
// Opcodes: 0xC0, 0xC8, 0xD0, 0xD8
void op_ret_cc(GameBoy* gb) {
    // 1. Decodificamos la condición (bits 3 y 4)
    u8 opcode = bus_read(gb, gb->cpu.pc - 1);
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

// ----------------- RST n (Restart / Call Vector) ----------------------
// Opcodes: 0xC7, 0xCF, 0xD7, 0xDF, 0xE7, 0xEF, 0xF7, 0xFF
//      RST 0 -> PC = $0000
//      RST 1 -> PC = $0008
//      RST 2 -> PC = $0010
//      RST 3 -> PC = $0018
//      RST 4 -> PC = $0020
//      RST 5 -> PC = $0028
//      RST 6 -> PC = $0030
//      RST 7 -> PC = $0038
void op_rst(GameBoy* gb) {
    // 1. Decodificar 't' (0 a 7)
    // Patrón: 11 ttt 111
    u8 opcode = bus_read(gb, gb->cpu.pc - 1);
    u8 t = (opcode >> 3) & 0x07;

    // 2. Guardar dirección de retorno (PC actual) en la Pila
    // RST funciona exactamente igual que un CALL
    push_pc(gb);

    // 3. Calcular nueva dirección (t * 8)
    u16 target_addr = t << 3;
    
    // 4. Saltar
    gb->cpu.pc = target_addr;
} 