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

const string STOR = "STR";
const string LOAD = "LDR";
const string MOV = "MOV";
const string ADD = "ADD";
const string SUB = "SUB";
const string MULT = "MUL";
const string SWI = "SWI";
const string SEEK = "0x6b";
const string WORD = ".word";
const string DEFAULT_VALUE = "0";
const string NO_REG = "no_reg";

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

string nt_to_arm(nodetype type);
string load(quad load, arm_register *value_reg);
string regify(int num);
vector<int> regs_with_dt(vector<arm_register> regs, data_type filter);
string at_address(string reg);
list<string> stor(quad store, arm_register *id_add, arm_register *value_reg);
string three_arity_nc(string op, string dest, string opd1);
string make_label(string id);
list<string> declare_idents(set<string> idents);
set<string> get_idents(list<quad> quads);
string four_arity(string op, string dest, string opd1, string opd2);
string three_arity(string op, string dest, string opd1);
string two_arity(string op, string dest);
string arm_constant(string val);
string arm_small_constant(string val);
list<string> process_quads(list<quad> quads);
list<string> process_quads(list<quad> quads, vector<arm_register> *regs);
vector<arm_register> make_registers(int num_regs);
