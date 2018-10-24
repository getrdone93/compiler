#include <string>
#include <set>
#include <list>
#include <iostream>
#include <sstream>
#include <map>

#ifndef PARSE_H
#define PARSE_H
#include "parse.h"
#endif // PARSE_H

using namespace std;

const string LOAD = "ldr";
const string MOV = "mov";
const string ADD = "add";
const string SUB = "sub";
const string MULT = "mul";
const string SWI = "swi";
const string SEEK = "0x6b";

struct quad {
  nodetype type;
  string dest;
  string opd2;
  string opd3;
  string opd4;
};

string quad_to_string(quad q);
string dash_if_empty(string s);
quad two_arity_quad(nodetype op, string opd1);
quad three_arity_quad(nodetype op, string opd1, string opd2);
quad four_arity_quad(nodetype op, string opd1, string opd2, string opd3);
string list_pairs_to_string(list<pair<string, string> > lis);
void output_node(parsetree *node, string var_name);
void output_pair(pair<string, string> p, string var_name);
list<nodetype> expression_types();
list<nodetype> operator_types();
list<quad> arm_output_new(parsetree *root, set<string> *regs_avail, set<pair<string, string> > *regs_used, list<quad> res);
list<quad> ground_expression(parsetree *root, set<string> *regs_avail, 
					      set<pair<string, string> > *regs_used);
list<quad> nested_expression(parsetree *root, set<string> *regs_avail, 
					      set<pair<string, string> > *regs_used, list<nodetype> exp_types);
parsetree * zero_depth_child(parsetree *root, int child, nodetype type);
parsetree * zero_depth_child(parsetree *root, int child, list<nodetype> poss_types);
quad load_leaf_new(parsetree *node, set<string> *regs_avail, set<pair<string, string> > *regs_used);
parsetree * get_id_or_const(parsetree* root, int child);
quad simple_assignment(parsetree *ident, parsetree *assign, parsetree *constant, set<string> *regs_avail, 
			 set<pair<string, string> > *regs_used);
list<quad> handle_assignment(parsetree *root, set<string> *regs_avail, set<pair<string, string> > *regs_used);
string complex_expression(parsetree *root, set<string> *regs_avail, set<pair<string, string> > *regs_used);
string arm_small_constant(string val);
string arm_constant(string val);
parsetree * get_const(parsetree *ae, int child);
parsetree * get_assign(parsetree *ae);
parsetree * node_search(parsetree *root, list<pair<int, nodetype> > path);
parsetree * get_ident(parsetree *ae, int child);
string four_arity(string op, string opd1, string opd2, string opd3);
string three_arity(string op, string opd1, string opd2);
string two_arity(string op, string opd1);
string assoc_if_not_used(string id, set<string> *regs_avail, set<pair<string, string> > *regs_used);
quad load_into_reg(string id, string value, set<string> *regs_avail, 
				   set<pair<string, string> > *regs_used);
string error(string func, string error);
int get_value(parsetree *node);
void assign_to_ident(parsetree *ident_node, parsetree *const_node);
string to_string(int num);
int to_int(string str);
string update_output_nnl(string output, string new_str);
string load_const(parsetree *p_expr, set<string> *regs_avail, set<pair<string, string> > *regs_used);
string load_leaf(parsetree *node, set<string> *regs_avail, set<pair<string, string> > *regs_used);
string load_leafs(parsetree *expr_node, set<string> *regs_avail, set<pair<string, string> > *regs_used);
void release_reg(parsetree *root, set<string> *regs_avail, set<pair<string, string> > *regs_used);
pair<string, string> release_reg(string id, set<string> *regs_avail, set<pair<string, string> > *regs_used);
pair<string, string> lookup_str(string str, set<pair<string, string> > *regs_used);
string operator_to_arm(parsetree *op_node);
string update_output(string output, string new_str);
string sa(parsetree *root, set<string> *regs_avail, set<pair<string, string> > *regs_used);
string assoc_id_reg(set<string> *regs_avail, set<pair<string, string> > *regs_used, string id);
string first(set<string> *regs);
string print_register(string reg);
string load_ident(parsetree *p_expr, set<string> *regs_avail, set<pair<string, string> > *regs_used);
bool write_exp(parsetree *root);
string arm_output(parsetree *root, set<string> *regs_avail, set<pair<string, string> > *regs_used,  string *output);
void test_traverse(parsetree *root);
