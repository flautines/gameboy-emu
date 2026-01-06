#include <stdio.h>
#include "gb.h" // Incluir solo gb.h nos da acceso a todo

int main(void) {
    printf("--- INCIANDO TEST DE ARQUITECTURA GAMEBOY ---\n");

    // 1. Creación de la instancia
    // Al usar instancias directas en el struct, no necesitamos mallocs complejos
    GameBoy gb;

    // 2. Inicialización simple (limpiar basura de memoria)
    cpu_init(&gb.cpu);

    printf("GameBoy creada. PC inicial: 0x%04X\n", gb.cpu.PC);

    // 3. Prueba de VRAM (Zona Video 0x8000 - 0x9FFF)
    printf("\n[TEST] Escribiendo 0xAA en VRAM (0x8010)...\n");
    bus_write(&gb, 0x8010, 0xAA);

    u8 val_vram = bus_read(&gb, 0x8010);
    printf("[TEST] Leído de VRAM: 0x%02X ", val_vram);
    if (val_vram == 0xAA) printf("-> OK\n"); else printf("-> ERROR\n");

    // 4. Prueba de WRAM (Zona RAM Trabajo 0xC000 - 0xDFFF)
    printf("\n[TEST] Escribiendo 0x55 en WRAM (0xC050)...\n");
    bus_write(&gb, 0xC050, 0x55);

    u8 val_wram = bus_read(&gb, 0xC050);
    if (val_wram == 0x55) printf("-> OK\n"); else printf("-> ERROR\n");

    // 5. Prueba de ROM (Zona Protegida 0x0000 - 0x7FFF)
    printf("\n[TEST] Intentando escribir en ROM (0x0100)...\n");
    bus_write(&gb, 0x0100, 0xFF);

    // Leemos de nuevo (debería ser 0 porque no cargamos cartucho y el bus_read devuelve 0 en ROM por defecto)
    u8 val_rom = bus_read(&gb, 0x0100);
    printf("[TEST] Leído de ROM: 0x%02X (Debería ser 0)\n", val_rom);

    // Ejecutamos un ciclo
    cpu_step(&gb);
    printf("\n[TEST] Despues de ejecutar NOP: PC: 0x%04X\n", gb.cpu.PC);

    printf("\n--- TEST FINALIZADO CON ÉXITO ---\n");
    return 0;
}