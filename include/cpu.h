#ifndef CPU_H
#define CPU_H

#include "common.h"

#define CPU_F_Z  0x80 // Zero Flag
#define CPU_F_N  0x40 // Subtract Flag
#define CPU_F_H  0x20 // Half Carry Flag
#define CPU_F_C  0x10 // Carry Flag

// Helper para el Flag Z (Zero)
#define CHECK_ZERO(value) ((value) == 0 ? CPU_F_Z : 0)

// Helpers para Half Carry
#define CHECK_HALF_CARRY_ADD(a, b) ((((a) & 0x0F) + ((b) & 0x0F)) > 0x0F ? CPU_F_H : 0)
#define CHECK_HALF_CARRY_SUB(a, b) ((((a) & 0x0F) < ((b) & 0x0F)) ? CPU_F_H : 0)
#define CHECK_CARRY_ADD(a, b) (((u16)(a) + (u16)(b)) > 0xFF ? CPU_F_C : 0)
#define CHECK_CARRY_SUB(a, b) ((a) < (b) ? CPU_F_C : 0)

typedef struct {
    // Registros de la CPU
    u8 A;  // Acumulador
    u8 F;  // Registro de flags
    u8 B;  // Registro B
    u8 C;  // Registro C
    u8 D;  // Registro D
    u8 E;  // Registro E
    u8 H;  // Registro H
    u8 L;  // Registro L
    u16 SP; // Puntero de pila
    u16 PC; // Contador de programa

    // Estado interno
    bool ime;        // Interrupt Master Enable
    bool halted;     // Indica si la CPU está en modo halt
} Cpu;

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