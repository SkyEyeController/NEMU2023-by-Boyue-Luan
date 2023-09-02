#include "cpu/exec/template-end.h"
#include "cpu/exec/template-start.h"
//指令——RET（C3)
make_helper(concat(ret_nothing_,SUFFIX))
{
   cpu.eip=MEM_R(reg_l(R_ESP));
   reg_l(R_ESP)+=DATA_BYTE;
   print_asm("ret");
   return 0;
}