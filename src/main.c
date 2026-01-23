#include <stdio.h>
#include <string.h>
#include "gb.h" // Incluir solo gb.h nos da acceso a todo
#include "tests.h" // Prueba de opcodes

int main(void) {
    printf("--- TEST DE CPU ---\n");
    printf("Opcodes 0x00 - 0xFF\n");
    char test_dir[] = "tests/sm83/v1/";
    char test_file[80];
    for (int opcode = 0x00; opcode <= 0xFF; opcode ++) {
        // 0x10 - STOP 
        // 0x76 - HALT
        // 0xFB - EI
        if (opcode == 0x10 || opcode == 0x76
        ||  opcode == 0xd3 || opcode == 0xdb
        ||  opcode == 0xdd || opcode == 0xe3
        ||  opcode == 0xe4 || opcode == 0xeb
        ||  opcode == 0xec || opcode == 0xed
        ||  opcode == 0xf4 || opcode == 0xfb
        ||  opcode == 0xfc || opcode == 0xfd ) 
            continue;

        memset(test_file, 0, sizeof(test_file));

        // CB 0x00 - 0xFF
        if (opcode == 0xCB) {
            for (int cb_opcode = 0x00; cb_opcode <= 0xFF; cb_opcode ++) {
                sprintf(test_file, "%scb %02x.json", test_dir, cb_opcode);
                if (run_tests_for_opcode(test_file)) {
                    printf(" OK\n");
                }
                else {
                    return -80;
                }
            }
            opcode++;
        }
        sprintf(test_file, "%s%02x.json", test_dir, opcode);
        if (run_tests_for_opcode(test_file)) {
            printf(" OK\n");
        }
        else {
            return -79;
        }
    }
    
    return 0;
}