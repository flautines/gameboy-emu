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
    printf("\n[TEST] Despues de ejecutar NOP: PC: 0x%04X\n", gb.cpu.PC);

    // Como aún no disponemos de función para cargar cartucho, asignamos el PC
    // a WRAM para pruebas de instrucciones
    gb.cpu.PC = 0xC001; // Dirección en WRAM
    
    printf("\n--- TEST FINALIZADO CON ÉXITO ---\n");
    return 0;
}