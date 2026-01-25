#include <stdio.h>
#include <string.h>
#include "gb.h" // Incluir solo gb.h nos da acceso a todo
#include "tests.h" // Prueba de opcodes

int main(void) 
{
    #ifdef MODO_TEST
        #if defined(TEST_CH5)
        test_chapter5_timer();
        #elif defined(TEST_CPU)
        run_tests_for_opcode("tests/sm83/v1/00.json");
        #endif
    #else
        printf("--- GAMEBOY EMU ---\n");
    #endif
    
    return 0;
}