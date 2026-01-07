#include <stdio.h>
#include "gb.h" // Incluir solo gb.h nos da acceso a todo

// Función auxiliar que muestra el estado de la CPU
void print_cpu_state(const Cpu* cpu) {
    printf("Estado de la CPU:\n");
    printf("A: 0x%02X ", cpu->A);
    printf("B: 0x%02X C: 0x%02X ", cpu->B, cpu->C);
    printf("D: 0x%02X E: 0x%02X ", cpu->D, cpu->E);
    printf("H: 0x%02X L: 0x%02X\n", cpu->H, cpu->L);

    printf("SP: 0x%04X ", cpu->SP);
    printf("PC: 0x%04X ", cpu->PC);

    // Mostrar flags de Z, N, H, C por separado
    printf("%c %c %c %c\n",
           (cpu->F & CPU_F_Z) ? 'Z' : '.',
           (cpu->F & CPU_F_N) ? 'N' : '.',
           (cpu->F & CPU_F_H) ? 'H' : '.',
           (cpu->F & CPU_F_C) ? 'C' : '.');

    printf("IME: %d ", cpu->ime);
    printf("Halted: %d\n", cpu->halted);
}

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
    print_cpu_state(&gb.cpu);

    // Test INC BC, INC DE, INC HL, INC SP
    
    printf("\n--- TEST FINALIZADO CON ÉXITO ---\n");
    return 0;
}