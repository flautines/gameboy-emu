#ifndef CPU_H
#define CPU_H

#include "common.h"

#define FLAG_Z       0x80 // Zero Flag
#define FLAG_N       0x40 // Subtract Flag
#define FLAG_H       0x20 // Half Carry Flag
#define FLAG_C       0x10 // Carry Flag

// Tipos de Interrupción (Bits en IE/IF)
#define INT_VBLANK   0x01 // Bit 0: V-Blank
#define INT_LCD_STAT 0x02 // Bit 1: LCD Status (HBlank, etc)
#define INT_TIMER    0x04 // Bit 2: Timer Overflow
#define INT_SERIAL   0x08 // Bit 3: Serial Transfer
#define INT_JOYPAD   0x10 // Bit 4: Joypad Press

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
    u8 a;           // Acumulador
    u8 f;           // Registro de flags
    u8 b;           // Registro B
    u8 c;           // Registro C
    u8 d;           // Registro D
    u8 e;           // Registro E
    u8 h;           // Registro H
    u8 l;           // Registro L
    u16 sp;         // Puntero de pila
    u16 pc;         // Contador de programa

    // Gestión de Interrupciones
    u8 ie;          // Interrupt Enable ($FFFF)
    u8 if_reg;      // Interrupt Flag   ($FF0F)
    bool ime;       // Interrupt Master Enable (Flag interno, NO tiene dirección de memoria)

    // Estado interno
    bool halted;    // Indica si la CPU está en modo halt
    bool halt_bug;  // Indica si se produjo el bug en la instrucción HALT
    bool stopped;   // Indica si la CPU está en modo STOP

    u8 cycles;      // Ciclos totales ejecutados de reloj
} Cpu;

typedef enum {
    REG_PAIR_BC = 0,
    REG_PAIR_DE = 1,
    REG_PAIR_HL = 2,
    REG_PAIR_SP = 3, // Usado en instrucciones aritméticas/carga
    REG_PAIR_AF = 3, // Usado en PUSH/POP (Alias para claridad)
} RegisterPairIndex;

void cpu_init(Cpu* cpu);
int cpu_step(GameBoy* gb);

// Función para solicitar la interrupción de tipo correspondiente, indicada por type
void cpu_request_interrupt(GameBoy* gb, u8 type);

#endif