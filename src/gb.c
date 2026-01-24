#include "gb.h"

void gb_init(GameBoy* gb)
{
    cpu_init(&gb->cpu);
    timer_init(&gb->timer);

    // Usamos modelo de memoria de producción
    // y no el de 64KB lineal para tests
    gb->bus.test_mode = false;

    // TODO: Inicializar Memoria, etc.
}