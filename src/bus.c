// src/bus.c
#include "gb.h"

u8 bus_read(GameBoy* gb, u16 address) {
    // 1. MODO TEST
    if (gb->bus.test_mode) {
        return gb->bus.flat_memory[address];
    }
    
    // 2. MODO PRODUCCIÓN
    // ROM (Cartucho)
    if (address < 0x8000) {
        return 0; // TODO: Cartucho
        //return cart_read(gb->cart, address);
    }

    // VRAM (Video)
    else if (address < 0xA000) {
        return gb->bus.vram[address - 0x8000];
    }

    // External RAM (Cartucho)
    else if (address < 0xC000) {
        return 0; // TODO: Cartucho
        //return cart_read(gb->cart, address);
    }

    // WRAM (Working RAM)
    else if (address < 0xE000) {
        return gb->bus.wram[address - 0xC000];
    }

    // Echo RAM (Espejo de WRAM)
    else if (address < 0xFE00) {
        return gb->bus.wram[address - 0xE000];
    }
    // OAM (Sprites)
    else if (address < 0xFEA0) {
        // TODO: Bloquear si la PPU está leyendo (DMA)
        return gb->bus.oam[address - 0xFE00];
    }

    // Zona Prohibida (No usable)
    else if (address < 0xFF00) {
        return 0xFF; // Comportamiento indefinido, devolver FF es seguro
    }

    // IF register
    else if (address == 0xFF0F) {
        // Interceptamos lectura de IF
        // Los bits superiores (5-7) de IF siempre devuelven 1 (hardware quirk)
        return gb->cpu.if_reg | 0xE0;
    }

    // I/O Registers
    else if (address >= 0xFF00 && address < 0xFF80) {
        // Aquí manejamos joypad, timers, audio...
        return gb->bus.io[address - 0xFF00];
    }

    // HRAM
    else if (address >= 0xFF80 && address < 0xFFFF) {
        return gb->bus.hram[address - 0xFF80];
    }

    // IE Register
    else if (address == 0xFFFF) {
        // Interceptamos lectura de IE
        return gb->cpu.ie;
    }

    return 0xFF;
}

u16 bus_read16(GameBoy* gb, u16 addr) {
    u8 lo = bus_read(gb, addr);
    u8 hi = bus_read(gb, addr + 1);
    return (hi << 8) | lo;
}

void bus_write(GameBoy* gb, u16 address, u8 value) {
    // 1. MODO TEST (Usamos flat_memory)
    if (gb->bus.test_mode) {
        // Escribimos siempre en la memoria plana (para que el JSON verify funcione)
        gb->bus.flat_memory[address] = value;

        return; // Salimos, no hacemos nada más
    }

    // 2. MODO PRODUCCIÓN
    if (address < 0x8000) {
        // ¡IMPORTANTE! Escribir en ROM configura el MBC (Banking)
        // TODO: Cartucho
        // cart_write(gb->cart, address, value);
    }
    else if (address < 0xA000) {
        gb->bus.vram[address - 0x8000] = value;
    }
    else if (address < 0xC000) {
        // TODO: Cartucho
        // cart_write(gb->cart, address, value);
    }
    else if (address < 0xE000) {
        gb->bus.wram[address - 0xC000] = value;
    }
    else if (address < 0xFE00) {
        // Echo RAM: Escribir aquí escribe en WRAM
        gb->bus.wram[address - 0xE000] = value;
    }
    else if (address < 0xFEA0) {
        gb->bus.oam[address - 0xFE00] = value;
    }
    // Not Usable
    else if (address < 0xFF00) {

    }
    // IF Register
    else if (address == 0xFF0F) {
        // Escritura en IF (El juego puede querer limpiar una interrupción manualmente)
        gb->cpu.if_reg = value | 0xE0; // Bits 
    }
    // I/O Registers
    else if (address >= 0xFF00 && address < 0xFF80) {
        // IO Registers
        // TODO: Algunos registros son de solo lectura o tienen efectos secundarios
        gb->bus.io[address - 0xFF00] = value;
    }
    // HRAM Registers
    else if (address < 0xFFFF) {
        gb->bus.hram[address - 0xFF80] = value;
    }
    // IE Register
    else if (address == 0xFFFF) {
        // Escritura en IE
        gb->cpu.ie = value; 
    } 
}

void bus_write16(GameBoy* gb, u16 addr, u16 value) {
    // Parte Baja
    u8 lo = (value & 0x00FF);
    bus_write(gb, addr, lo);

    // Parte Alta
    u8 hi = (value >> 8);
    bus_write(gb, addr + 1, hi);  
}