#include <string>
#include <set>
#include <list>
#include <iostream>
#include <sstream>
#include <map>

#ifndef QUAD_H
#define QUAD_H
#include "quad.h"
#endif // QUAD_H

#ifndef PARSE_H
#define PARSE_H
#include "parse.h"
#endif // PARSE_H

#ifndef HELPER_H
#define HELPER_H
#include "helper_funcs.h"
#endif // HELPER_H

using namespace std;

list<quad> unary_minus(parsetree *node, set<nodetype> unaries);
set<nodetype> unary_p_m();
list<quad> prefix_postfix_exp(parsetree *node, set<nodetype> unary_ops);
set<nodetype> unary_ops();
set<nodetype> set_ground_exp();
list<quad> unary_post_pre_exp(parsetree *node, set<nodetype> nested_exp, set<nodetype> ge);
set<nodetype> set_op_types();
bool contains(set<nodetype> types, nodetype type);
set<nodetype> set_expression_types();
quad load_leaf(parsetree *node);
string next_reg();
void output_node(parsetree *node, string var_name);
list<nodetype> expression_types();
list<nodetype> operator_types();
list<quad> make_quads(parsetree *root, list<quad> res);
list<quad> ground_expression(parsetree *root, set<nodetype> accepted_exp);
list<quad> nested_expression(parsetree *root, set<nodetype> set_exp, set<nodetype> ge);
list<quad> handle_assignment(parsetree *root, set<nodetype> exp_types, set<nodetype> ge);
string arm_small_constant(string val);
string arm_constant(string val);
string error(string func, string error);
quad write_exp_quad(parsetree *write_node, parsetree *ident);
