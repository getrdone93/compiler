#include <string>
#include <set>
#include <iostream>
#include <sstream>

#ifndef PARSE_H
#define PARSE_H
#include "parse.h"
#endif // PARSE_H

using namespace std;

const string LOAD = "ldr";
const string MOV = "mov";
const string SWI_SEEK = "swi\t0x6b";
const string ADD = "add";
const string SUB = "sub";
const string MULT = "mul";

pair<string, string> load_into_reg(string id, string value, set<string> *regs_avail, set<pair<string, string> > *regs_used);
string error(string func, string error);
void test_traverse(parsetree *root);
string basic_exp(string op, string exp_reg, string r1, string r2);
int get_value(parsetree *node);
void assign_to_ident(parsetree *ident_node, parsetree *const_node);
string to_string(int num);
int to_int(string str);
string update_output_nnl(string output, string new_str);
string load_const(parsetree *p_expr, set<string> *regs_avail, set<pair<string, string> > *regs_used);
string load_leaf(parsetree *node, set<string> *regs_avail, set<pair<string, string> > *regs_used);
string load_leafs(parsetree *expr_node, set<string> *regs_avail, set<pair<string, string> > *regs_used);
void release_reg(parsetree *root, set<string> *regs_avail, set<pair<string, string> > *regs_used);
void release_reg(string id, set<string> *regs_avail, set<pair<string, string> > *regs_used);
string mov(string to_reg, string from_reg);
pair<string, string> lookup_str(string str, set<pair<string, string> > *regs_used);
string operator_to_arm(parsetree *op_node);
bool simple_assign_exp(parsetree *root);
string update_output(string output, string new_str);
string simple_assignment(parsetree *root, set<string> *regs_avail, set<pair<string, string> > *regs_used);
string sa(parsetree *root, set<string> *regs_avail, set<pair<string, string> > *regs_used);
string grab_reg_by_id(set<string> *regs_avail, set<pair<string, string> > *regs_used, string id);
string first(set<string> *regs);
string load_register(string reg, string constant);
string load_register(string reg, int constant);
string print_register(string reg);
string load_ident(parsetree *p_expr, set<string> *regs_avail, set<pair<string, string> > *regs_used);
bool write_exp(parsetree *root);
string arm_output(parsetree *root, set<string> *regs_avail, set<pair<string, string> > *regs_used,  string *output);
void test_traverse(parsetree *root);
