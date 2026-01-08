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

    // Ejecutamos un ciclo
    cpu_step(&gb);

    // Como aún no disponemos de función para cargar cartucho, asignamos el PC
    // a WRAM para pruebas de instrucciones
    gb.cpu.PC = 0xC001; // Dirección en WRAM

    // Test de opciones PUSH y POP
    // Cargamos instrucciones en memoria
    bus_write(&gb, 0xC001, 0xE5); // PUSH BC
    bus_write(&gb, 0xC002, 0xD5); // PUSH DE
    bus_write(&gb, 0xC003, 0xC5); // PUSH HL
    bus_write(&gb, 0xC004, 0xF5); // PUSH AF

    // INC r para ver cambios antes de POP
    bus_write(&gb, 0xC005, 0x04); // INC B
    bus_write(&gb, 0xC006, 0x14); // INC D
    bus_write(&gb, 0xC007, 0x24); // INC H
    bus_write(&gb, 0xC008, 0x3C); // INC A

    bus_write(&gb, 0xC009, 0xF1); // POP AF
    bus_write(&gb, 0xC00A, 0xC1); // POP HL
    bus_write(&gb, 0xC00B, 0xD1); // POP DE
    bus_write(&gb, 0xC00C, 0xE1); // POP BC
    
    

    printf("-------------- [TEST] PUSH/POP rr ---------------------\n");
    for (int i = 0; i < 12; i++) {
        cpu_step(&gb);
    }
    
    printf("\n--- TEST FINALIZADO CON ÉXITO ---\n");
    return 0;
}