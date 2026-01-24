// src/bus.c
#include "gb.h"
#include "timer.h"

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

    // I/O Registers
    else if (address >= 0xFF00 && address < 0xFF80) {
        // 1. Joypad (0xFF00)
        if (address == 0xFF00) {
            // return joypad_read(gb, address);
        }
        // 2. Serial (0xFF01 - 0xFF02)
        else if (address == 0xFF01 || address == 0xFF02) {
            // return serial_read(gb, address);
        }
        // 3. Timers (0xFF04 - 0xFF07)
        else if (address >= 0xFF04 && address <= 0xFF07) {
            return timer_read(gb, address);
        }
        // Registro IF (Interrupt Flag)
        else if (address == 0xFF0F) {
            // Interceptamos lectura de IF
            // Los bits superiores (5-7) de IF siempre devuelven 1 (hardware quirk)
            return gb->cpu.if_reg | 0xE0;
        }
        // 4. Audio (0xFF10 - 0xFF26)
        else if (address ) {
            // return (gb, address);
        }
        // ... PPU, DMA, etc
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
    // I/O Registers
    else if (address >= 0xFF00 && address < 0xFF80) {
        // Guardamos el valor en el array "crudo"
        // Aunque no usemos el valor, será útil para depurar
        gb->bus.io[address - 0xFF00] = value;

        // --- Despachador ---

        // 1. Joypad (0xFF00)
        if (address == 0xFF00) {
            // joypad_write(gb, address, value);
        }
        // 2. Serial (0xFF01 - 0xFF02)
        else if (address == 0xFF01 || address == 0xFF02) {
            // serial_write(gb, address, value)
        }
        // 3. Timers (0xFF04 - 0xFF07)
        else if (address >= 0xFF04 && address <= 0xFF07) {
            timer_write(gb, address, value);
        }
        // IF Register
        else if (address == 0xFF0F) {
        // Escritura en IF (El juego puede querer limpiar una interrupción manualmente)
            gb->cpu.if_reg = value | 0xE0;
        }
        // 4. Audio (0xFF10 - 0xFF26)
        else if (address >= 0xFF10 && address <= 0xFF26) {
            // apu_write(gb, address, value);
        }
        // ... PPU, DMA, etc ...
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