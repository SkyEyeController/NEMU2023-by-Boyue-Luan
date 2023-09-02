#include<cpu/exec/template-start.h>

#define instr pop
//指令pop 58+n，pop_r_v
static void do_execute()
{
    if(DATA_BYTE==2)
    {
        swaddr_write((DATA_TYPE)op_src->reg,2,MEM_R(R_ESP));
        reg_l(R_ESP)+=DATA_BYTE;
    }
    else
    {
        swaddr_write((DATA_TYPE)op_src->reg,4,MEM_R(R_ESP));
        reg_l(R_ESP)+=DATA_BYTE;
    }
    print_asm_template2();
}
make_instr_helper(r);
make_instr_helper(rm);
#include<cpu/exec/template-end.h>
