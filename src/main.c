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

    // Como aún no disponemos de función para cargar cartucho, asignamos el PC
    // a WRAM para pruebas de instrucciones
    // Test de INC y DEC
    // INC C
    gb.cpu.PC = 0xC001; // Dirección en WRAM
    // Escribimos la instrucción INC C (0x0C) en 0
    bus_write(&gb, 0xC001, 0x0C);
    gb.cpu.C = 0xFF; // Valor inicial
    printf("\n[TEST] Probando instrucción INC C (C=0x%02X)...\n", gb.cpu.C);
    cpu_step(&gb);
    printf("C después de INC: 0x%02X\n", gb.cpu.C);
    if (gb.cpu.C == 0x00 && (gb.cpu.F & CPU_F_Z)) {
        printf("INC C funcionó correctamente -> OK\n");
    } else {
        printf("INC C falló -> ERROR, C: 0x%02X\n", gb.cpu.C);
    }

    // DEC C
    // Escribimos la instrucción DEC C (0x0D) en 0
    bus_write(&gb, gb.cpu.PC, 0x0D);
    printf("\n[TEST] Probando instrucción DEC C (C=0x%02X)...\n", gb.cpu.C);
    cpu_step(&gb);
    printf("C después de DEC: 0x%02X\n", gb.cpu.C);
    if (gb.cpu.C == 0xFF && (gb.cpu.F & CPU_F_N)) {
        printf("DEC C funcionó correctamente -> OK\n");
    } else {
        printf("DEC C falló -> ERROR\n");
    }

    // Test de ADD A, r
    // Escribimos la instrucción ADD A,C (0x81) en 0
    bus_write(&gb, gb.cpu.PC, 0x81);
    gb.cpu.A = 0x14; // Valor inicial A
    gb.cpu.C = 0x22; // Valor inicial C
    // Activamos flag de carry a 1 para probar que no afecta a ADD
    gb.cpu.F |= CPU_F_C;
    printf("\n[TEST] Probando instrucción ADD A,C (A=0x%02X, C=0x%02X)...\n", gb.cpu.A, gb.cpu.C);
    cpu_step(&gb);
    printf("A después de ADD: 0x%02X\n", gb.cpu.A);
    if (gb.cpu.A == 0x36) {
        printf("ADD A,C funcionó correctamente -> OK\n");
    } else {
        printf("ADD A,C falló -> ERROR\n");
    }

    // TEST de ADDC A, r con acarreo
    // Escribimos la instrucción ADDC A,C (0x89) en 0
    bus_write(&gb, gb.cpu.PC, 0x89);
    gb.cpu.A = 0x50; // Valor inicial A
    gb.cpu.C = 0x30; // Valor inicial C
    // Activamos flag de carry a 1
    gb.cpu.F |= CPU_F_C;

    printf("\n[TEST] Probando instrucción ADDC A,C con carry (A=0x%02X, C=0x%02X)...\n", gb.cpu.A, gb.cpu.C);
    cpu_step(&gb);
    printf("A después de ADDC: 0x%02X\n", gb.cpu.A);
    if (gb.cpu.A == 0x81) {
        printf("ADDC A,C funcionó correctamente -> OK\n");
    } else {
        printf("ADDC A,C falló -> ERROR\n");
    }

    // Test de SUB A, r
    // Escribimos la instrucción SUB A,C (0x91) en 0
    bus_write(&gb, gb.cpu.PC, 0x91);
    gb.cpu.A = 0x30; // Valor inicial A
    gb.cpu.C = 0x10; // Valor inicial C
    // Limpiamos flag de carry para probar que no afecta a SUB
    gb.cpu.F &= ~CPU_F_C;
    printf("\n[TEST] Probando instrucción SUB A,C (A=0x%02X, C=0x%02X)...\n", gb.cpu.A, gb.cpu.C);
    cpu_step(&gb);
    printf("A después de SUB: 0x%02X\n", gb.cpu.A);
    if (gb.cpu.A == 0x20) {
        printf("SUB A,C funcionó correctamente -> OK\n");
    } else {
        printf("SUB A,C falló -> ERROR\n");
    }

    // TEST de SBC A, (HL) con acarreo
    // Escribimos la instrucción SBC A,(HL) (0x9E) en 0
    bus_write(&gb, gb.cpu.PC, 0x9E);
    gb.cpu.A = 0x50; // Valor inicial A
    gb.cpu.H = 0xC0;
    gb.cpu.L = 0x20;
    bus_write(&gb, 0xC020, 0x20); // Escribimos un valor en (HL)
    // Activamos flag de carry a 1
    gb.cpu.F |= CPU_F_C;
    printf("\n[TEST] Probando instrucción SBC A,(HL) con carry (A=0x%02X, (HL)=0x%02X)...\n", gb.cpu.A, bus_read(&gb, 0xC020));
    cpu_step(&gb);
    printf("A después de SBC: 0x%02X\n", gb.cpu.A);
    if (gb.cpu.A == 0x2F) {
        printf("SBC A,(HL) funcionó correctamente -> OK\n");
    } else {
        printf("SBC A,(HL) falló -> ERROR\n");
    }

    // Test DEC (HL)
    // Escribimos la instrucción DEC (HL) (0x35) en 0
    bus_write(&gb, gb.cpu.PC, 0x35);
    gb.cpu.H = 0xC2;
    gb.cpu.L = 0x34;
    bus_write(&gb, 0xC234, 0xFF); // Escribimos un valor en HL
    printf("\n[TEST] Probando instrucción DEC (HL)...\n");
    cpu_step(&gb);
    u8 val_dec = bus_read(&gb, 0xC234);
    printf("Valor después de DEC (HL): 0x%02X\n", val_dec);
    if (val_dec == 0xFE && (gb.cpu.F & CPU_F_N)) {
        printf("DEC (HL) funcionó correctamente -> OK\n");
    } else {
        printf("DEC (HL) falló -> ERROR\n");
    }

    // Test ADD A, d8
    // Escribimos la instrucción ADD A,d8 (0xC6) en 0
    bus_write(&gb, gb.cpu.PC, 0xC6);
    bus_write(&gb, gb.cpu.PC + 1, 0x12);
    gb.cpu.A = 0x20; // Valor inicial A
    // Limpiamos flag de carry para probar ADD
    gb.cpu.F &= ~CPU_F_C;
    printf("\n[TEST] Probando instrucción ADD A,d8 (A=0x%02X, d8=0x12)...\n", gb.cpu.A);
    cpu_step(&gb);
    printf("A después de ADD A,d8: 0x%02X\n", gb.cpu.A);
    if (gb.cpu.A == 0x32) {
        printf("ADD A,d8 funcionó correctamente -> OK\n");
    } else {
        printf("ADD A,d8 falló -> ERROR\n");
    }

    // Test ADDC A, d8 with carry
    // Escribimos la instrucción ADDC A,d8 (0xCE) en 0
    bus_write(&gb, gb.cpu.PC, 0xCE);
    bus_write(&gb, gb.cpu.PC + 1, 0x15);
    gb.cpu.A = 0x30; // Valor inicial A
    // Activamos flag de carry a 1
    gb.cpu.F |= CPU_F_C;
    printf("\n[TEST] Probando instrucción ADDC A,d8 con carry (A=0x%02X, d8=0x15)...\n", gb.cpu.A);
    cpu_step(&gb);
    printf("A después de ADDC A,d8: 0x%02X\n", gb.cpu.A);
    if (gb.cpu.A == 0x46) {
        printf("ADDC A,d8 funcionó correctamente -> OK\n");
    } else {
        printf("ADDC A,d8 falló -> ERROR\n");
    }

    // Test de AND A, r
    // Escribimos la instrucción AND A,C (0xA1) en 0
    bus_write(&gb, gb.cpu.PC, 0xA1);
    gb.cpu.A = 0xF0; // Valor inicial A
    gb.cpu.C = 0x0F; // Valor inicial C
    printf("\n[TEST] Probando instrucción AND A,C (A=0x%02X, C=0x%02X)...\n", gb.cpu.A, gb.cpu.C);
    cpu_step(&gb);
    printf("A después de AND: 0x%02X\n", gb.cpu.A);
    if (gb.cpu.A == 0x00 && (gb.cpu.F & CPU_F_Z) && (gb.cpu.F & CPU_F_H) && !(gb.cpu.F & CPU_F_C) && !(gb.cpu.F & CPU_F_N)) {
        printf("AND A,C funcionó correctamente -> OK\n");
    } else {
        printf("AND A,C falló -> ERROR\n");
    }

    // Test de AND A, d8
    // Escribimos la instrucción AND A,d8 (0xE6) en 0
    bus_write(&gb, gb.cpu.PC, 0xE6);
    bus_write(&gb, gb.cpu.PC + 1, 0x3C);
    gb.cpu.A = 0xF0; // Valor inicial A
    printf("\n[TEST] Probando instrucción AND A,d8 (A=0x%02X, d8=0x3C)...\n", gb.cpu.A);
    cpu_step(&gb);
    printf("A después de AND A,d8: 0x%02X\n", gb.cpu.A);
    if (gb.cpu.A == 0x30 && !(gb.cpu.F & CPU_F_Z) && (gb.cpu.F & CPU_F_H) && !(gb.cpu.F & CPU_F_C) && !(gb.cpu.F & CPU_F_N)) {
        printf("AND A,d8 funcionó correctamente -> OK\n");
    } else {
        printf("AND A,d8 falló -> ERROR\n");
    }

    // Test de AND A, (HL)
    // Escribimos la instrucción AND A,(HL) (0xA6) en 0
    bus_write(&gb, gb.cpu.PC, 0xA6);
    gb.cpu.H = 0xC3;
    gb.cpu.L = 0x45;
    bus_write(&gb, 0xC345, 0x0F); // Escribimos un valor en (HL)
    gb.cpu.A = 0xF0; // Valor inicial A
    printf("\n[TEST] Probando instrucción AND A,(HL) (A=0x%02X, (HL)=0x%02X)...\n", gb.cpu.A, bus_read(&gb, 0xC345));
    cpu_step(&gb);
    printf("A después de AND A,(HL): 0x%02X\n", gb.cpu.A);
    if (gb.cpu.A == (0xF0 & 0x0F) && (gb.cpu.F & CPU_F_Z) && (gb.cpu.F & CPU_F_H) && !(gb.cpu.F & CPU_F_C) && !(gb.cpu.F & CPU_F_N)) {
        printf("AND A,(HL) funcionó correctamente -> OK\n");
    } else {
        printf("AND A,(HL) falló -> ERROR\n");
    }

    // Test de XOR A, r, d8 y (HL)
    // XOR A,B
    bus_write(&gb, gb.cpu.PC, 0xA8);
    gb.cpu.A = 0xFF;
    gb.cpu.B = 0x0F;
    printf("\n[TEST] Probando instrucción XOR A,B (A=0x%02X, B=0x%02X)...\n", gb.cpu.A, gb.cpu.B);
    cpu_step(&gb);
    printf("A después de XOR A,B: 0x%02X\n", gb.cpu.A);
    if (gb.cpu.A == (0xFF ^ 0x0F) && !(gb.cpu.F & CPU_F_Z) && !(gb.cpu.F & CPU_F_N) && !(gb.cpu.F & CPU_F_H) && !(gb.cpu.F & CPU_F_C)) {
        printf("XOR A,B funcionó correctamente -> OK\n");
    } else {
        printf("XOR A,B falló -> ERROR\n");
    }
    // XOR A,d8
    bus_write(&gb, gb.cpu.PC, 0xEE);
    bus_write(&gb, gb.cpu.PC + 1, 0xF0);
    gb.cpu.A = 0x0F;
    printf("\n[TEST] Probando instrucción XOR A,d8 (A=0x%02X, d8=0xF0)...\n", gb.cpu.A);
    cpu_step(&gb);
    printf("A después de XOR A,d8: 0x%02X\n", gb.cpu.A);
    if (gb.cpu.A == (0x0F ^ 0xF0) && !(gb.cpu.F & CPU_F_Z) && !(gb.cpu.F & CPU_F_N) && !(gb.cpu.F & CPU_F_H) && !(gb.cpu.F & CPU_F_C)) {
        printf("XOR A,d8 funcionó correctamente -> OK\n");
    } else {
        printf("XOR A,d8 falló -> ERROR\n");
    }
    // XOR A,(HL)
    bus_write(&gb, gb.cpu.PC, 0xAE);
    gb.cpu.H = 0xC4;
    gb.cpu.L = 0x56;
    bus_write(&gb, 0xC456, 0xFF); // Escribimos un valor en (HL)
    gb.cpu.A = 0xAA;
    printf("\n[TEST] Probando instrucción XOR A,(HL) (A=0x%02X, (HL)=0x%02X)...\n", gb.cpu.A, bus_read(&gb, 0xC456));
    cpu_step(&gb);
    printf("A después de XOR A,(HL): 0x%02X\n", gb.cpu.A);
    if (gb.cpu.A == (0xAA ^ 0xFF) && !(gb.cpu.F & CPU_F_Z) && !(gb.cpu.F & CPU_F_N) && !(gb.cpu.F & CPU_F_H) && !(gb.cpu.F & CPU_F_C)) {
        printf("XOR A,(HL) funcionó correctamente -> OK\n");
    } else {
        printf("XOR A,(HL) falló -> ERROR\n");
    }

    // Test de HALT   
    bus_write(&gb, gb.cpu.PC, 0x76);
    
    printf("\n[TEST] Probando instrucción HALT...\n");
    cpu_step(&gb); // Ejecutamos NOP en 0x0101
    printf("PC después de HALT: 0x%04X\n", gb.cpu.PC);
    if (gb.cpu.halted) {
        printf("CPU está en modo HALT -> OK\n");
    } else {    
        printf("CPU no entró en modo HALT -> ERROR\n");
    }

    printf("\n--- TEST FINALIZADO CON ÉXITO ---\n");
    return 0;
}