#ifndef BUS_H
#define BUS_H

#include "common.h"

// Definición de tamaños
#define WRAM_SIZE 8192
#define VRAM_SIZE 8192
#define HRAM_SIZE 127
#define OAM_SIZE  160

typedef struct {
    // Memoria interna de la consola
    u8 wram[WRAM_SIZE]; // Working RAM
    u8 vram[VRAM_SIZE]; // Video RAM
    u8 hram[HRAM_SIZE]; // High RAM
    u8 oam[OAM_SIZE];   // Object Attribute Memory

    // Registros de Hardware (IO)
    u8 io[0x80]; // $FF00 - $FF7F

    // ... Punteros al Cartucho (Lo veremos luego)
} Bus;

// Prototipos de funcions
// El compilador sabe que GameBoy es un tipo válido (un puntero),
// aunque no sabe qué hay dentro todavía.
u8 bus_read(GameBoy* gb, u16 address);
void bus_write(GameBoy* gb, u16 address, u8 value);

u16 bus_read16(GameBoy* gb, u16 address);

#endif