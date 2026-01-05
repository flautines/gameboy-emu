#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

// Definiciones e tipos para ganar claridad
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

// Declaramos que "existe" una estructura llamada GameBoy.
// Esto permite usar (GameBoy*) en los prototipos de función de bus.h y cpu.h
typedef struct GameBoy GameBoy;

// Macros de manipulación de bits (Vitales para GameBoy)
// n = número de bit (0-7)

// Leer el bit n de a: (a >> n) & 1
#define BIT(a, n) (((a) >> (n)) & 1)

// Poner a 1 el bit n de a
#define SET_BIT(a, n) ((a) |= (1 << (n)))

// Poner a 0 el bit n de a
#define RESET_BIT(a, n) ((a) &= ~(1 << (n)))

// Macro para detectar errores fatales
#define NO_IMPL { printf("No implementado: %s\n", __func__); exit(1); }

#endif