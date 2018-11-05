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

list<quad> prepare_dest(string fake_reg, int dest_real, vector<arm_register> *regs, map<string, int> *fake_to_real);
list<quad> prepare_operand(string fake_reg, int dest_real, vector<arm_register> *regs, map<string, int> *fake_to_real);
list<quad> call_divide(quad div, int dest_reg, vector<arm_register> *regs, map<string, int> *fake_to_real);
list<quad> handle_mod(quad div, vector<arm_register> *regs, map<string, int> *fake_to_real);
list<quad> handle_divide(quad div, vector<arm_register> *regs, map<string, int> *fake_to_real);
pair<string, int> pair_exists(int real_reg, map<string, int> *fake_to_real);
bool pair_exists(string fake_reg, map<string, int> *fake_to_real);
quad move_to_first_unused(pair<string, int> from, vector<arm_register> *regs, map<string, int> *fake_to_real);
quad move_to(pair<string, int> from, pair<string, int> to, vector<arm_register> *regs, map<string, int> *fake_to_real);
bool pair_exists(string fake_reg, map<string, int> *fake_to_real);
void reg_pair(pair<string, int> reg,  data_type type, vector<arm_register> *regs, map<string, int> *fake_to_real);
void free_pair(pair<string, int> free, vector<arm_register> *regs, map<string, int> *fake_to_real);
list<quad> arm_func_sdiv();
list<quad> handle_negate(quad negate, vector<arm_register> *regs, map<string, int> *fake_to_real);
list<quad> arm_funcs();
list<quad> arm_func_negate(string in_reg, string ret_reg);
nodetype post_to_regular(nodetype type);
string nt_to_arm(nodetype t);
list<quad> binary_operator(quad binary, arm_register *dest_reg, vector<arm_register> *regs, 
			   map<string, int> *fake_to_real);
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
