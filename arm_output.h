#include <string>
#include <set>
#include <list>
#include <iostream>
#include <sstream>
#include <map>

#include "quad.h"

#ifndef PARSE_H
#define PARSE_H
#include "parse.h"
#endif // PARSE_H

#ifndef HELPER_H
#define HELPER_H
#include "helper_funcs.h"
#endif // HELPER_H

using namespace std;

const string LOAD = "ldr";
const string MOV = "mov";
const string ADD = "add";
const string SUB = "sub";
const string MULT = "mul";
const string SWI = "swi";
const string SEEK = "0x6b";

quad store_leaf(parsetree *node);
string next_reg();
quad two_arity_quad(nodetype op, string opd1);
quad three_arity_quad(nodetype op, string opd1, string opd2);
quad four_arity_quad(nodetype op, string opd1, string opd2, string opd3);
void output_node(parsetree *node, string var_name);
list<nodetype> expression_types();
list<nodetype> operator_types();
list<quad> arm_output_new(parsetree *root, list<quad> res);
list<quad> ground_expression(parsetree *root, set<string> *regs_avail, 
					      set<pair<string, string> > *regs_used);
list<quad> nested_expression(parsetree *root, set<string> *regs_avail, 
					      set<pair<string, string> > *regs_used, list<nodetype> exp_types);
parsetree * zero_depth_child(parsetree *root, int child, nodetype type);
parsetree * zero_depth_child(parsetree *root, int child, list<nodetype> poss_types);
parsetree * get_id_or_const(parsetree* root, int child);
quad simple_assignment(parsetree *ident, parsetree *assign, parsetree *constant, set<string> *regs_avail, 
			 set<pair<string, string> > *regs_used);
list<quad> handle_assignment(parsetree *root, set<string> *regs_avail, set<pair<string, string> > *regs_used);
string arm_small_constant(string val);
string arm_constant(string val);
parsetree * get_const(parsetree *ae, int child);
parsetree * get_assign(parsetree *ae);
parsetree * node_search(parsetree *root, list<pair<int, nodetype> > path);
parsetree * get_ident(parsetree *ae, int child);
string four_arity(string op, string opd1, string opd2, string opd3);
string three_arity(string op, string opd1, string opd2);
string two_arity(string op, string opd1);
string error(string func, string error);
void assign_to_ident(parsetree *ident_node, parsetree *const_node);
string print_register(string reg);
bool write_exp(parsetree *root);
