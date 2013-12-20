#include "emu.h"

#include <stdio.h>
#include "debug.h"
#include "functions.h"
#include "io.h"
#include "opcodes.h"

#define NEXT_BYTE(cpu) ((cpu)->mem[(cpu)->pc + (pc_offset++)])

cpu *mcpu;

uint8_t opcode;
uint8_t arg1, arg2, t1;
int8_t s1;
uint16_t r1, r2;

// pc_offset is used to read from memory like a stream when processing
// bytecode without modifying the pc. pc_start is the memory address of the
// currently-executing opcode; if pc == pc_start at the end of a simulation
// step, we add pc_offset to get the start of the next instruction. if pc !=
// pc_start, we branched so we don't touch the pc.
uint8_t pc_offset = 0;
uint16_t pc_start;

// branch_offset is an offset that will be added to the program counter
// after we move to the next instruction
int8_t branch_offset = 0;

// int cycles_per_step = 100;

void em_main_loop() {
    cpu *m = mcpu;
    // int cycles;
    // for (cycles = 0; cycles < cycles_per_step; cycles++ ) {
        DUMP_DEBUG(m);

        reset_emu_flags(m);

        pc_offset = 0;
        branch_offset = 0;
        pc_start = m->pc;
        opcode = NEXT_BYTE(m);

        switch (opcode) {
            case NOP:
                break;

#ifndef DISABLE_EXTENSIONS
            case EXT:
                goto end;

            case DUMP:
                dump_cpu(m);
                break;
#endif

            #include "opcode_handlers/arithmetic.h"
            #include "opcode_handlers/branch.h"
            #include "opcode_handlers/compare.h"
            #include "opcode_handlers/flags.h"
            #include "opcode_handlers/incdec.h"
            #include "opcode_handlers/jump.h"
            #include "opcode_handlers/load.h"
            #include "opcode_handlers/logical.h"
            #include "opcode_handlers/shift.h"
            #include "opcode_handlers/stack.h"
            #include "opcode_handlers/store.h"
            #include "opcode_handlers/transfer.h"

            default:
                printf("ERROR: got unknown opcode %02x\n", opcode);
                goto end;
        }

        if (m->pc == pc_start) {
            m->pc += pc_offset;
        }
        m->pc += branch_offset;

        handle_io(m);

        if (m->interrupt_waiting && !get_flag(m, FLAG_INTERRUPT)) {
            STACK_PUSH(m) = (m->pc & 0xFF00) >> 8;
            STACK_PUSH(m) = m->pc & 0xFF;
            STACK_PUSH(m) = m->sr;

            m->interrupt_waiting = 0x00;
            m->pc = mem_abs(m->mem[0xFFFE], m->mem[0xFFFF], 0);
            m->sr |= FLAG_INTERRUPT;
        }

        m->last_opcode = opcode;
    // } // finished cycles
    return;
end:
    finish_io();
}


void main_loop(cpu *m) {
    init_io();

    mcpu = m;
    emscripten_set_main_loop(em_main_loop, 100, 1);

}