#include "gb.h"
#include <stdio.h>

bool test_chapter5_timer(void)
{
    printf("=== TEST CAPITULO 5: TIMER & INTERRUPTS ===\n");

    GameBoy gb;
    gb_init(&gb);
    // Limpiar memoria IO
    for (int i=0; i<0x80; i++) gb.bus.io[i] = 0;

    // ---------------------------------------------
    // TEST 1: Verificar que DIV incrementa
    // ---------------------------------------------
    printf("[1/3] Test Registro DIV... ");

    // Ejecutar 256 M-Cycles (1024 T-Cycles).
    // 256 M-Cycles = 1024 T-Cycles, DIV incrementa cada 256 T-Cycles.
    // 1024 / 256 = 4 incrementos de DIV.

    for (int i=0; i<256; i++) {
        timer_tick(&gb, 1); // 1 M-Cycle por tick
    }

    u8 div_val = bus_read(&gb, 0xFF04);
    if (div_val == 4) {
        printf("\x1b[1;32mOK\x1b[0m (DIV = %d)\n", div_val);
    }
    else {
        printf("\x1b[1;31mFAIL\x1b[0m (Esperado 4, obtenido %d)\n", div_val);
        return false;
    }

    // ----------------------------------------------
    // TEST 2: TIMA (Timer Counter) y TAC
    // ----------------------------------------------
    printf("[2/3] Test TIMA Running... ");

    // Configurar TAC ($FF07)
    // Bit 2: Enable (1)
    // Bit 1-0: Clock Select 00 (4096Hz -> cada 1024 T-Cycles)
    // Valor: 0000 0100 -> 0x04
    bus_write(&gb, 0xFF07, 0x04);

    // Correr 1024 T-Cycles (256 M-Cycles). Tima debería TIMA debería incrementar en 1.
    for (int i=0; i<256; i++) {
        timer_tick(&gb, 1);
    }

    u8 tima_val = bus_read(&gb, 0xFF05);
    if (tima_val == 1) {
        printf("\x1b[1;32mOK\x1b[0m (TIMA = %d)\n", tima_val);
    }
    else {
        printf("\x1b[1;31mFAIL\x1b[0m (Esperado 1, obtenido %d)\n", tima_val);
        return false;
    }
    // ---------------------------------------------
    // TEST 3: Desbordamiento e Interrupción
    // ---------------------------------------------
    printf("[3/3] Test Overflow & Interrupt Request... ");

    // Poner TIMA al borde del overflow
    bus_write(&gb, 0xFF05, 0xFF);
    // Configurar TMA(Recarga) a 0x50
    bus_write(&gb, 0xFF06, 0x50);

    // Correr lo suficiente para triggerear un incremento
    for (int i=0; i<300; i++) timer_tick(&gb, 1);

    // Verificaciones:
    // 1. TIMA debería ser igual a TMA (0x50) tras el reload
    // 2. Registro IF ($FF0F) debería tener el bit 2 activo (0x04)

    u8 tima_after = bus_read(&gb, 0xFF05);
    u8 if_flag = bus_read(&gb, 0xFF0F);

    bool ok_val = (tima_after == 0x50);
    bool ok_int = (if_flag & 0x04);

    if (ok_val && ok_int) {
        printf("\x1b[1;32mOK\x1b[0m\n");
    }
    else {
        printf("\x1b[1;31mFAIL\x1b[0m (TIMA: %02X, IF: %02X)\n", tima_after, if_flag);
        return false;
    }

    return true;
}