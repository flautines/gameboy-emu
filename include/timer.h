#ifndef TIMER_H
#define TIMER_H
#include "common.h"

typedef struct {
    // Contador interno de 16 bits (DIV es la parte alta)
    u16 div_counter;

    // Registros de Timer
    u8 tima; // $FF05
    u8 tma;  // $FF06
    u8 tac;  // $FF07

    // Estado interno para detección de flancos (Edge Detector)
    // Necesario para emular correctamente los quirks del timer
    bool prev_signal;
} Timer;

void timer_init(Timer* timer);
void timer_tick(GameBoy* gb, u8 m_cycles);

// Funciones I/O para el Bus
void timer_write(GameBoy* gb, u16 address, u8 value);
u8 timer_read(GameBoy* gb, u16 address);

#endif