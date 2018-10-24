#include <string>
#include <list>
#include <iostream>
#include <sstream>

#ifndef NODE_NAMES_H
#define NODE_NAMES_H
#include "nodeNames.h"
#endif // NODE_NAMES_H

using namespace std;

struct quad {
  nodetype type;
  string dest;
  string opd2;
  string opd3;
};

string quad_list_to_str(list<quad> quads);
string test_for_error(nodetype type, int nodenames_size, string og);
string to_str(int num);
quad three_arity_quad(nodetype op, string opd1, string opd2);
quad two_arity_quad(nodetype op, string opd1);
quad four_arity_quad(nodetype op, string dest, string opd2, string opd3);
string dash_if_empty(string s);
void print_quad_list(list<quad> quads);
string quad_to_string(quad q);

