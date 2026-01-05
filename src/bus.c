// src/bus.c
#include "gb.h"

u8 bus_read(GameBoy* gb, u16 address) {
    // 1. ROM (Cartucho)
    if (address < 0x8000) {
        return 0; // TODO: Cartucho
        //return cart_read(gb->cart, address);
    }

    // 2. VRAM (Video)
    else if (address < 0xA000) {
        return gb->bus.vram[address - 0x8000];
    }

    // 3. External RAM (Cartucho)
    else if (address < 0xC000) {
        return 0; // TODO: Cartucho
        //return cart_read(gb->cart, address);
    }

    // 4. WRAM (Working RAM)
    else if (address < 0xE000) {
        return gb->bus.wram[address - 0xC000];
    }

    // 5. Echo RAM (Espejo de WRAM)
    else if (address < 0xFE00) {
        return gb->bus.wram[address - 0xE000];
    }
    // 6. OAM (Sprites)
    else if (address < 0xFEA0) {
        // TODO: Bloquear si la PPU está leyendo (DMA)
        return gb->bus.oam[address - 0xFE00];
    }

    // 7. Zona Prohibida (No usable)
    else if (address < 0xFF00) {
        return 0xFF; // Comportamiento indefinido, devolver FF es seguro
    }

    // 8. I/O Registers
    else if (address < 0xFF80) {
        // Aquí manejamos joypad, timers, audio...
        return gb->bus.io[address - 0xFF00];
    }

    // 9. HRAM
    else if (address < 0xFFFF) {
        return gb->bus.hram[address - 0xFF80];
    }

    // 10. IE Register
    else if (address == 0xFFFF) {
        return gb->bus.ie_register;
    }

    return 0xFF;
}

void bus_write(GameBoy* gb, u16 address, u8 value) {
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
    else if (address < 0xFF00) {
        // Unusable
    }
    else if (address < 0xFF80) {
        // IO Registers
        // TODO: Algunos registros son de solo lectura o tienen efectos secundarios
        gb->bus.io[address - 0xFF00] = value;
    }
    else if (address < 0xFFFF) {
        gb->bus.hram[address - 0xFF80] = value;
    }
    else if (address == 0xFFFF) {
        gb->bus.ie_register = value;
    }
    
}