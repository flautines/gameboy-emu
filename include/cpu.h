#ifndef CPU_H
#define CPU_H

#include "common.h"

#define FLAG_Z  0x80 // Zero Flag
#define FLAG_N  0x40 // Subtract Flag
#define FLAG_H  0x20 // Half Carry Flag
#define FLAG_C  0x10 // Carry Flag

// Helper para el Flag Z (Zero)
#define CHECK_ZERO(value) ((value) == 0 ? FLAG_Z : 0)

// Helpers para Half Carry
// Comprobamos si la suma de los nibbles bajos desborda (supera 15)
// Fórmula: (A & 0xF) + (val & 0xF) + carry_in > 0xF
#define CHECK_HALF_CARRY_ADD(A, val, carry_in) ((((A) & 0x0F) + ((val) & 0x0F) + ((carry_in) & 0x0F)) > 0x0F ? FLAG_H : 0)

#define CHECK_HALF_CARRY_SUB(A, val, carry_in) (((((A) & 0x0F) - (carry_in)) < ((val) & 0x0F)) ? FLAG_H : 0)

// Si el resultado total no cabe en 8 bits (> 255)
#define CHECK_CARRY_ADD(result) ((result) > 0xFF ? FLAG_C : 0)

#define CHECK_CARRY_SUB(result) ((result) < 0 ? FLAG_C : 0)

typedef struct {
    // Registros de la CPU
    u8 A;        // Acumulador
    u8 F;        // Registro de flags
    u8 B;        // Registro B
    u8 C;        // Registro C
    u8 D;        // Registro D
    u8 E;        // Registro E
    u8 H;        // Registro H
    u8 L;        // Registro L
    u16 SP;      // Puntero de pila
    u16 PC;      // Contador de programa

    // Estado interno
    bool ime;    // Interrupt Master Enable
    bool halted; // Indica si la CPU está en modo halt

    u8 cycles;   // Ciclos totales ejecutados de reloj
} Cpu;

typedef enum {
    REG_PAIR_BC = 0,
    REG_PAIR_DE = 1,
    REG_PAIR_HL = 2,
    REG_PAIR_SP = 3, // Usado en instrucciones aritméticas/carga
    REG_PAIR_AF = 3, // Usado en PUSH/POP (Alias para claridad)
} RegisterPairIndex;

// Helpers para leer/escribir pares de registros
// Nota: GameBoy es Little Endian, pero los registros pares se leen High-Low.
// BC -> B es el byte alto, C es el byte bajo.
static inline u16 get_bc(Cpu* cpu) { return (cpu->B << 8) | cpu->C; }
static inline void set_bc(Cpu* cpu, u16 value) { cpu->B = (value >> 8) & 0xFF; cpu->C = value & 0xFF; }
static inline u16 get_de(Cpu* cpu) { return (cpu->D << 8) | cpu->E; }
static inline void set_de(Cpu* cpu, u16 value) { cpu->D = (value >> 8) & 0xFF; cpu->E = value & 0xFF; }
static inline u16 get_hl(Cpu* cpu) { return (cpu->H << 8) | cpu->L; }
static inline void set_hl(Cpu* cpu, u16 value) { cpu->H = (value >> 8) & 0xFF; cpu->L = value & 0xFF; }

void cpu_init(Cpu* cpu);
int cpu_step(GameBoy* gb);

#endif