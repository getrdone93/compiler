#include "quad.h"
#include <set>

using namespace std;

const string LOAD = "ldr";
const string MOV = "mov";
const string ADD = "add";
const string SUB = "sub";
const string MULT = "mul";
const string SWI = "swi";
const string SEEK = "0x6b";

enum data_type {
  CONST,
  MEM_ADD,
  DATA, 
  NONE
};

struct arm_register {
  data_type dt;
  int number;
};

list<string> process_quads(list<quad> quads);
list<string> process_quads(list<quad> quads, list<arm_register> *regs);
list<arm_register> make_registers(int num_regs);
