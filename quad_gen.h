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

set<nodetype> set_ground_exp();
list<quad> handle_ground_node(parsetree *node);
set<nodetype> set_op_types();
bool contains(set<nodetype> types, nodetype type);
set<nodetype> set_expression_types();
quad store_leaf(parsetree *node);
string next_reg();
void output_node(parsetree *node, string var_name);
list<nodetype> expression_types();
list<nodetype> operator_types();
list<quad> make_quads(parsetree *root, list<quad> res);
list<quad> ground_expression(parsetree *root);
list<quad> nested_expression(parsetree *root, set<nodetype> exp_types);
quad simple_assignment(parsetree *ident, parsetree *assign, parsetree *constant);
list<quad> handle_assignment(parsetree *root);
string arm_small_constant(string val);
string arm_constant(string val);
string error(string func, string error);
quad write_exp_quad(parsetree *write_node, parsetree *ident);
