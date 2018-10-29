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
#include <map>

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

string nt_to_arm(nodetype t);
list<quad> binary_operator(quad binary, arm_register *dest_reg, vector<arm_register> *regs, map<string, int> *fake_to_real);
bool contains(set<string> strs, string s);
list<string> asm_quads_to_asm(list<quad> asm_quads);
string quad_to_arm(quad q);
list<quad> load(quad load, arm_register *value_reg, set<string> idents, map<string, int> *fake_to_real);
string regify(int num);
vector<int> regs_with_dt(vector<arm_register> *regs, data_type filter);
string at_address(string reg);
list<quad> stor(quad store, vector<arm_register> *regs, map<string, int> *fake_to_real);
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
