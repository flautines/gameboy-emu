#include <stdlib.h>
#include <string.h>
#include "cjson/cJSON.h"
#include <gb.h>

void set_state(GameBoy* gb, cJSON* state)
{
    // 0. Activamos el modo test
    gb->bus.test_mode = true;

    // 1. Cargar Registros
    gb->cpu.pc = cJSON_GetObjectItem(state, "pc")->valueint;
    gb->cpu.sp = cJSON_GetObjectItem(state, "sp")->valueint;
    gb->cpu.a = cJSON_GetObjectItem(state, "a")->valueint;
    gb->cpu.f = cJSON_GetObjectItem(state, "f")->valueint;
    gb->cpu.b = cJSON_GetObjectItem(state, "b")->valueint;
    gb->cpu.c = cJSON_GetObjectItem(state, "c")->valueint;
    gb->cpu.d = cJSON_GetObjectItem(state, "d")->valueint;
    gb->cpu.e = cJSON_GetObjectItem(state, "e")->valueint;
    gb->cpu.h = cJSON_GetObjectItem(state, "h")->valueint;
    gb->cpu.l = cJSON_GetObjectItem(state, "l")->valueint;
    gb->cpu.ime = cJSON_GetObjectItem(state, "ime")->valueint;
    gb->cpu.ie = cJSON_GetObjectItem(state, "ie")->valueint;

    // 2. Cargar RAM (Modificada)
    // El JSON trae la RAM como una lista de arrays: [ [addr, val], [addr, val] ... ]
    cJSON* ram = cJSON_GetObjectItem(state, "ram");
    int ram_count = cJSON_GetArraySize(ram);

    // Limpiamos la memoria plana antes de empezar
    memset(gb->bus.flat_memory, 0, 65536);

    for (int i = 0; i < ram_count; i++) {
        cJSON* entry = cJSON_GetArrayItem(ram, i);
        u16 addr = cJSON_GetArrayItem(entry, 0)->valueint;
        u8 val = cJSON_GetArrayItem(entry, 1)->valueint;

        bus_write(gb, addr, val);
    }
}

bool check_state(GameBoy* gb, cJSON* expected)
{
    bool passed = true;

    // Comparar registros
    if (gb->cpu.a != cJSON_GetObjectItem(expected, "a")->valueint) {
        printf("FAIL: Register A missmatch\n");
        passed = false;
    }
    if (gb->cpu.f != cJSON_GetObjectItem(expected, "f")->valueint) {
        printf("FAIL: Register F missmatch\n");
        passed = false;
    }
    if (gb->cpu.b != cJSON_GetObjectItem(expected, "b")->valueint) {
        printf("FAIL: Register B missmatch\n");
        passed = false;
    }
    if (gb->cpu.c != cJSON_GetObjectItem(expected, "c")->valueint) {
        printf("FAIL: Register C missmatch\n");
        passed = false;
    }
    if (gb->cpu.d != cJSON_GetObjectItem(expected, "d")->valueint) {
        printf("FAIL: Register D missmatch\n");
        passed = false;
    }
    if (gb->cpu.e != cJSON_GetObjectItem(expected, "e")->valueint) {
        printf("FAIL: Register E missmatch\n");
        passed = false;
    }
    if (gb->cpu.h != cJSON_GetObjectItem(expected, "h")->valueint) {
        printf("FAIL: Register H missmatch\n");
        passed = false;
    }
    if (gb->cpu.l != cJSON_GetObjectItem(expected, "l")->valueint) {
        printf("FAIL: Register L missmatch\n");
        passed = false;
    }

    if (gb->cpu.pc != cJSON_GetObjectItem(expected, "pc")->valueint) {
        printf("FAIL: Register PC missmatch\n");
        passed = false;
    }
    if (gb->cpu.sp != cJSON_GetObjectItem(expected, "sp")->valueint) {
        printf("FAIL: Register SP missmatch\n");
        passed = false;
    }

    if (gb->cpu.ime != cJSON_GetObjectItem(expected, "ime")->valueint) {
        printf("FAIL: Register IME missmatch\n");
        passed = false;
    }

    // Comparar RAM (solo las direcciones que menciona el JSON)
    cJSON* ram = cJSON_GetObjectItem(expected, "ram");
    int ram_count = cJSON_GetArraySize(ram);
    for (int i = 0; i < ram_count; i++) {
        cJSON* entry = cJSON_GetArrayItem(ram, i);
        u16 addr = cJSON_GetArrayItem(entry, 0)->valueint;
        u8 expected_val = cJSON_GetArrayItem(entry, 1)->valueint;
        u8 actual_val = bus_read(gb, addr);

        if (actual_val != expected_val) {
            printf("FAIL: RAM missmatch at %d\n", addr);
            passed = false;
        }
    }

    return passed;
}

bool run_tests_for_opcode(const char* filename)
{
    bool passed = true;

    // 1. Leer el archivo completo a un buffer
    FILE* f = fopen(filename, "rb");
    fseek(f, 0, SEEK_END);
    size_t length = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* buffer = malloc(length + 1);
    size_t ret_code = fread(buffer, 1, length, f);
    if (ret_code < length) {
        if (feof(f))
            printf("Error leyendo %s: fin de fichero inesperado\n", filename);
        else if (ferror(f))
            perror("Error leyendo %s fichero de tests");
        exit(-1);
    }
    fclose(f);
    buffer[length] = '\0';

    // 2. Parsear JSON
    cJSON* tests = cJSON_Parse(buffer);

    int total_tests = cJSON_GetArraySize(tests);
    printf("Ejecutando %d tests del archivo %s...", total_tests, filename);

    GameBoy gb;
    cpu_init(&gb.cpu);

    for (int i = 0; i < total_tests; i++) {
        cJSON* test_case = cJSON_GetArrayItem(tests, i);

        // --- PREPARAR ---
        set_state(&gb, cJSON_GetObjectItem(test_case, "initial"));

        // --- EJECUTAR ---
        // Ejecuta UN paso de la CPU
        cpu_step(&gb);

        // --- VERIFICAR ---
        if (!check_state(&gb, cJSON_GetObjectItem(test_case, "final"))) {
            printf("Falló el test: %s\n", cJSON_GetObjectItem(test_case, "name")->valuestring);
            // Salimos al primer fallo para poder depurar
            passed = false;
            break;
        }
    }

    cJSON_Delete(tests);
    free(buffer);

    return passed;
}