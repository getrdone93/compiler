#ifndef QUAD_H
#define QUAD_H
#include "quad.h"
#endif // QUAD_H

#ifndef HELPER_H
#define HELPER_H
#include "helper_funcs.h"
#endif // HELPER_H

#include <set>
#include <vector>

using namespace std;

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

list<string> asm_quads_to_asm(list<quad> asm_quads);
string quad_to_arm(quad q);
quad load(quad load, arm_register *value_reg);
string regify(int num);
vector<int> regs_with_dt(vector<arm_register> regs, data_type filter);
string at_address(string reg);
list<quad> stor(quad store, arm_register *id_add, arm_register *value_reg);
string three_arity_nc(string op, string dest, string opd1);
string make_label(string id);
list<quad> declare_idents(set<string> idents);
set<string> get_idents(list<quad> quads);
string four_arity(string op, string dest, string opd1, string opd2);
string three_arity(string op, string dest, string opd1);
string two_arity(string op, string dest);
string arm_constant(string val);
string arm_small_constant(string val);
list<quad> quads_to_asm(list<quad> quads);
list<quad> quads_to_asm(list<quad> quads, vector<arm_register> *regs);
vector<arm_register> make_registers(int num_regs);
