#ifndef GB_H
#define GB_H

#include "common.h"
#include "bus.h"
#include "cpu.h"
#include "timer.h"
#include "ppu.h"

// El contexto global de la emulación
struct GameBoy {
    Bus bus;
    Cpu cpu;
    Timer timer;
    Ppu ppu;
    bool paused;
    
    // Contador global de ciclos de sistema
    u64 ticks; 
};

void gb_init(GameBoy* gb);

#endif