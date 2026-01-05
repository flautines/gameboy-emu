#ifndef GB_H
#define GB_H

#include "common.h"
#include "bus.h"
//#include "cpu.h"

// Forward declarations para evitar dependencias circulares
typedef struct Cpu Cpu;
typedef struct Ppu Ppu;
// ... otros componentes

// El contexto global de la emulación
struct GameBoy {
    Bus bus;
    bool paused;
    // Contador global de ciclos de sistema
    u64 ticks; 
};

#endif