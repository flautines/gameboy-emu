#ifndef PPU_H
#define PPU_H

#include "common.h"

// Dimensiones del LCD visble
#define SCREEN_WIDTH  160
#define SCREEN_HEIGHT 144

// Constantes de direcciones de memoria de los registros PPU
#define LCDC_ADDR 0xFF40
#define STAT_ADDR 0xFF41
#define SCY_ADDR  0xFF42
#define SCX_ADDR  0xFF43
#define LY_ADDR   0xFF44
#define LYC_ADDR  0xFF45
#define BGP_ADDR  0xFF47

// STAT PPU status register bits
// Mascara de Selección de Interrupción según estado del LCDC
#define INTR_LYC   (1 << 6)
#define INTR_M2    (1 << 5)
#define INTR_M1    (1 << 4)
#define INTR_M0    (1 << 3)
#define LYC_STAT   (1 << 2)
#define LCD_MODE_0 0
#define LCD_MODE_1 1
#define LCD_MODE_2 2
#define LCD_MODE_3 3

typedef struct {
    u32 dot_ticks; // Contador de ciclos dentro de una línea (0- 456)

    // Buffer de pantalla (160x144 pixeles)
    // Aquí guardamos el ID de color (0-3) y luego aplicaremos paleta
    u8 frame_buffer[SCREEN_WIDTH * SCREEN_HEIGHT];

    // Registros PPU
    // Se ha optado por no tenerlos aquí y dejar el acceso directamente desde el array bus.io 
    // ya que ni lectura ni escritura requieren acciones adicionales
    
} Ppu;

// Función auxiliar: Obtiene el ID de color (0-3) de una fila de un Tile
// tile_addr: Dirección base del tile en VRAM
// row: Fila del tile (0-7)
// pixel: Columna del tile (0-7), donde 0 es izquierda, 7 es derecha
u8 ppu_get_color_id(GameBoy* gb, u16 tile_addr, u8 row, u8 pixel);

void ppu_tick(GameBoy* gb, u8 m_cycles);
#endif