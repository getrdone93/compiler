#include "arm_output.h"

using namespace std;

list<arm_register> make_registers(int num_regs) {
  list<arm_register> res;
  int i;
  for (i = 1; i <= num_regs; i++) {
    arm_register reg;
    reg.dt = NONE;
    reg.number = i;
    res.push_back(reg);
  }
  return res;
}

list<string> process_quads(list<quad> quads, set<arm_register> regs) {
  list<string> res;
  return res;
}
