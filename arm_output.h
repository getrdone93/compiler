#ifndef QUAD_H
#define QUAD_H
#include "quad.h"
#endif // QUAD_H
#include <set>

using namespace std;

const string LOAD = "ldr";
const string MOV = "mov";
const string ADD = "add";
const string SUB = "sub";
const string MULT = "mul";
const string SWI = "swi";
const string SEEK = "0x6b";
const string WORD = ".word";
const string DEFAULT_VALUE = "0";

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

string make_label(string id);
list<string> declare_idents(set<string> idents);
set<string> get_idents(list<quad> quads);
string four_arity(string op, string dest, string opd1, string opd2);
string three_arity(string op, string dest, string opd1);
string two_arity(string op, string dest);
string arm_constant(string val);
string arm_small_constant(string val);
list<string> process_quads(list<quad> quads);
list<string> process_quads(list<quad> quads, list<arm_register> *regs);
list<arm_register> make_registers(int num_regs);
