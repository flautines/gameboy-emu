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

    // Test de instrucciones CP A, r y CP A, d8
    // r = B, C, D, E, H, L, (HL), A
    // Cargamos las instrucciones en memoria
    // Apuntamos HL a 0xC500 y colocamos un valor allí
    gb.cpu.H = 0xC5;
    gb.cpu.L = 0x00;
    bus_write(&gb, 0xC500, 0x01); // Valor en (HL)
    bus_write(&gb, 0xC001, 0xb8); // CP B
    bus_write(&gb, 0xC002, 0xb9); // CP C
    bus_write(&gb, 0xC003, 0xba); // CP D
    bus_write(&gb, 0xC004, 0xbb); // CP E
    bus_write(&gb, 0xC005, 0xbc); // CP H
    bus_write(&gb, 0xC006, 0xbd); // CP L
    bus_write(&gb, 0xC007, 0xbe); // CP (HL)
    bus_write(&gb, 0xC008, 0xbf); // CP A
    bus_write(&gb, 0xC009, 0xFE); // CP d8
    bus_write(&gb, 0xC00A, 0x05); // d8 = 0x05
    // Ejecutamos las instrucciones
    for (int i = 0; i < 9; i++) {
        cpu_step(&gb);
    }

    
    printf("\n--- TEST FINALIZADO CON ÉXITO ---\n");
    return 0;
}