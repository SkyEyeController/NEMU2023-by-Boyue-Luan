#include "cpu/exec/template-start.h"
#define instr cmp

static void do_execute(){
    DATA_TYPE ret=op_dest->val -op_src ->val;
    update_eflags_pf_zf_sf(ret);
    cpu.eflags.CF=op_dest -> val < op_src -> val;
    int tmp1 = (op_dest -> val) >> ((DATA_BYTE << 3) - 1);
    int tmp2 = (op_src -> val) >> ((DATA_BYTE << 3) - 1);
    cpu.eflags.OF=(tmp1 != tmp2 && tmp2 == cpu.eflags.SF);
    print_asm_template2();
}

make_instr_helper(i2a)
make_instr_helper(i2rm)
make_instr_helper(r2rm)
make_instr_helper(rm2r)
#if DATA_BYTE ==2||DATA_BYTE == 4
make_instr_helper(si2rm)
#endif

#include "cpu/exec/template-end.h"