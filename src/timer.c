#include "gb.h"
#include "timer.h"

void timer_init(Timer* timer)
{
    // En el hardware real inicializa a un valor aleatorio
    timer->div_counter = 0;
    timer->tima = 0;
    timer->tma = 0;
    timer->tac = 0;
    timer->prev_signal = false;
}

// Función auxiliar para obtener el bit del system counter según TAC
static bool get_timer_bit(Timer* timer)
{
    u16 counter = timer->div_counter;

    // TAC bits 1-0 determinan qué bit del contador monitoreamos
    switch (timer->tac & 0x03) {
        case 0: return (counter & (1 << 9)) != 0; // Bit 9 (4096 Hz)
        case 1: return (counter & (1 << 3)) != 0; // Bit 3 (262144 Hz)
        case 2: return (counter & (1 << 5)) != 0; // Bit 5 (65536 Hz)
        case 3: return (counter & (1 << 7)) != 0; // Bit 7 (16384 Hz)
    }
    return false;
}

void timer_tick(GameBoy* gb, u8 m_cycles)
{
    Timer* gb_timer = &gb->timer;

    // Procesamos ciclo a ciclo (T-Cycle) para máxima precisión
    // 1 M-Cycle = 4 T-Cycles
    u32 t_cycles = m_cycles * 4;

    for (u32 i = 0; i < t_cycles; i++) {
        // 1. Incrementamos el contador del sistema (DIV es div_counter >> 8)
        gb_timer->div_counter++;

        // 2. Detección de Flanco (Edge Detection)
        // La señal es: BitSeleccionado AND TimerEnable (TAC bit 2)
        bool bit_value = get_timer_bit(gb_timer);
        bool timer_enable = (gb_timer->tac & (1 << 2)) != 0;

        bool current_signal = bit_value && timer_enable;

        // Si la señal pasa de 1 a 0 (Falling Edge), incrementamos TIMA
        if (gb_timer->prev_signal && !current_signal) {
            gb_timer->tima++;
            
            // 3. Manejo de Overflow
            if (gb_timer->tima == 0) { // Desbordó de 255 a 0
                gb_timer->tima = gb_timer->tma; // Recarga

                // Solicitar interrupción
                cpu_request_interrupt(gb, INT_TIMER);
            }
        }

        // Actualizamos el estado anterior para el siguiente ciclo
        gb_timer->prev_signal = current_signal;
    }
}

u8 timer_read(GameBoy* gb, u16 address)
{
    switch (address) {
        case 0xFF04: return gb->timer.div_counter >> 8; // DIV
        case 0xFF05: return gb->timer.tima;
        case 0xFF06: return gb->timer.tma;
        case 0xFF07: return gb->timer.tac;
        default: return 0xFF;
    }
}

void timer_write(GameBoy* gb, u16 address, u8 value)
{
    Timer* gb_timer = &gb->timer;
    switch (address)
    {
    case 0xFF04:
        // Escritura en DIV resetea el contador interno a 0.
        // IMPORTANTE: Esto puede provocar un "Falling Edge" inmediato y aumentar TIMA.
        // Al cambiar div_counter, la señal get_timer_bit puede cambiar.
        gb_timer->div_counter = 0;
        // Nota: En una emulación perfecta, deberíamos ejecutar la lógica de flanco aquí mismo.
        // Por simplicidad en este paso, asumimos que se corregirá en el siguiente tick.
        break;
    case 0xFF05: gb_timer->tima = value; break;
    case 0xFF06: gb_timer->tma = value; break;
    case 0xFF07: gb_timer->tac = value; break;
    default:
        break;
    }
}