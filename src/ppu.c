// src/ppu.c
#include "gb.h"

u8 ppu_get_color_id(GameBoy* gb, u16 tile_addr, u8 row, u8 pixel)
{
    // Cada fila ocupa 2 bytes. La fila 0 está en offset 0 y 1. La fila 1 en 2 y 3...
    u16 offset = row * 2;

    u8 byte1 = bus_read(gb, tile_addr + offset);     // LSB plane
    u8 byte2 = bus_read(gb, tile_addr + offset + 1); // MSB plane

    // El bit 7 es el pixel 0, el bit 0 es el pixel 7.
    // Necesitamos desplazar para traer el bit de interés a la posición 0.
    u8 shift = 7 - pixel;

    u8 lsb = (byte1 >> shift) & 1;
    u8 msb = (byte2 >> shift) & 1;

    return (msb << 1) | lsb;
}

void ppu_init(GameBoy* gb)
{
    gb->ppu.dot_ticks = 0;

    // LCDC tiene el valor $91 despues de reset
    bus_write(gb, LCDC_ADDR, 0x91);
}

// Función auxiliar para saber si el LCD está encendido
static bool is_lcd_enabled(GameBoy* gb)
{
    return (bus_read(gb, LCDC_ADDR) & (1 << 7)) != 0;
}

// Lógica de renderizado de UNA línea del fondo
static void ppu_draw_scanline(GameBoy* gb)
{
    u8 ly = bus_read(gb, LY_ADDR);
    u8 lcdc = bus_read(gb, LCDC_ADDR);
    u8 scy = bus_read(gb, SCY_ADDR);
    u8 scx = bus_read(gb, SCX_ADDR);
    u8 bgp = bus_read(gb, BGP_ADDR);

    // Si el fondo está desactivado, salimos
    if (!(lcdc & 1)) return;

    // 1. Determinar qué mapa de tiles usar (Bit 3)
    u16 tile_map_base = (lcdc & (1 << 3)) ? 0x9C00 : 0x9800;

    // 2. Determinar qué set de datos de tiles usar (Bit 4)
    bool signed_tile_addressing = !(lcdc & (1 << 4));
    u16 tile_data_base = (lcdc & (1 << 4)) ? 0x8000 : 0x9000;

    // Coordenada Y en el mapa de 256 x 256
    u8 map_y = ly + scy;

    // File dentro del tile (0-7)
    u8 tile_row = map_y %8;

    // Fila del tile en la cuadrícula de 32x32 (0-31)
    u8 map_row_idx = map_y / 8;

    // Iteramos los 160 píxeles de la pantalla
    for (int px = 0; px < SCREEN_WIDTH; px++) {
        // Coordenada X en el mapa de 256x256
        u8 map_x = px + scx;

        // Columna del tile en la cuadrícula (0-31)
        u8 map_col_idx = map_x / 8;

        // Calcular la dirección del tile ID en el mapa
        // Mapa = Base + (Fila * 32) + Columna
        u16 tile_id_addr = tile_map_base + (map_row_idx * 32) + map_col_idx;
        u8 tile_id = bus_read(gb, tile_id_addr);

        // Calcular dirección real de los datos del tile
        u16 tile_addr;
        if (signed_tile_addressing) {
            // Modo $8800: tile_id es signedd (-128 a 127)
            int8_t id_signed = (int8_t)tile_id;
            // Cada tile son 16 bytes.
            // Base $9000. Si id es 0 -> $9000. Si id es -1 -> $8FF0
            tile_addr = tile_data_base + (id_signed * 16);
        }
        else {
            // Modo $8000: tile_id es unsigned (0 a 255)
            tile_addr = tile_data_base + (tile_id * 16);
        }

        // Obtener el color ID (0-3)
        u8 column_in_tile = map_x % 8;
        u8 color_id = ppu_get_color_id(gb, tile_addr, tile_row, column_in_tile);

        // Mapear color ID a través de la paleta (BGP)
        // BGP bits: 7-6 (Color 3), 5-4 (Color 2), 3-2 (Color 1), 1-0 (Color 0)
        u8 real_color = (bgp >> (color_id * 2)) & 0x03;

        // Escribir en el framebuffer
        gb->ppu.frame_buffer[ly * SCREEN_WIDTH + px] = real_color;
    }
}
void ppu_tick(GameBoy* gb, u8 m_cycles)
{
    if (!is_lcd_enabled(gb)) {
        // Si LCD está apagado, LY se fija a 0 y salimos
        // TODO: Resetear estado y timers
        bus_write(gb, LY_ADDR, 0);
        gb->ppu.dot_ticks = 0;
        return;
    }

    u32 t_cycles = m_cycles * 4;
    gb->ppu.dot_ticks += t_cycles;

    u8 ly = bus_read(gb, LY_ADDR);
    u8 stat = bus_read(gb, STAT_ADDR);

    // Máquina de estados basada en dot_ticks (Total por línea: 456)
    // Nota: esto es una simplificación funcional. Un emulador perfecto
    // cambia de modos exactamente en los ciclos precisos.

    if (ly < 144) {
        // --- SCANLINE VISIBLE ---

        // Mode 2: OAM Search (0 - 80 dots)
        if (gb->ppu.dot_ticks <= 80) {
            // Set STAT Mode 2
            stat = (stat & 0xFC) | LCD_MODE_2;
            // Solicitar interrupción STAT si está habilitada
            if (stat & INTR_M2) { // Mode 2 Interrupt Enabled?
                cpu_request_interrupt(gb, INT_LCD_STAT);
            }
        }
        // Mode 3: Pixel Transfer (80 - ~252 dots) - Variable en realidad
        else if (gb->ppu.dot_ticks <= 252) {
            stat = (stat & 0xFC) | LCD_MODE_3;
        }
        // Mode 0: H-Blank (252 - 456 dots)
        else {
            stat = (stat & 0xFC) | LCD_MODE_0;
            // Solicitar interrupción STAT si está habilitada
            if (stat & INTR_M0) { // Mode 0 Interrupt Enabled?
                cpu_request_interrupt(gb, INT_LCD_STAT);
            }
        }
    } else {
        // --- V-BLANK (Líneas 144-153) ---
        stat = (stat & 0xFC) | LCD_MODE_1;
    }

    // Guardar STAT actualizado
    bus_write(gb, STAT_ADDR, stat);

    // FIN DE LA LÍNEA
    if (gb->ppu.dot_ticks >= 456) {
        gb->ppu.dot_ticks -= 456;
        ly++;

        // Si alcanzamos la línea 144, entramos en V-BLANK
        if (ly == 144) {
            cpu_request_interrupt(gb, INT_VBLANK);
            // TODO: RENDERIZAR CON RAYLIB EL FRAME COMPLETO
        }
        else if (ly > 153) {
            // Fin de V-Blank, volvemos a línea 0
            ly = 0;
        }

        // Actualizamos LY en memoria
        gb->bus.io[LY_ADDR - 0xFF00] = ly;

        // Comparación LYC = LY (Lógica para INTR_LYC)
        u8 lyc = bus_read(gb, LYC_ADDR);
        if (ly == lyc) {
            stat |= LYC_STAT; // Set LYC=LY Flag
            // Lanzar interrupción STAT si está ITNR_LYC habilitada
            if (stat & INTR_LYC) { // LYC=LY Interrupt Enabled?
                cpu_request_interrupt(gb, INT_LCD_STAT);
            }
        }
        else {
            // Reset LYC=LY Flag
            stat &= ~LYC_STAT;
        }
        bus_write(gb, STAT_ADDR, stat); // Actualizar Flag

        // DIBUJAR LÍNEA (Si estamos en rango visible)
        if (ly < 144) {
            ppu_draw_scanline(gb);
        }
    }
}