#include <stack>
#include <map>
#include <vector>
#include <set>
#include <sstream>
#include <string>

#ifndef PARSE_H
#define PARSE_H
#include "parse.h"
#endif // PARSE_H

using namespace std;

bool id_array(parsetree *node, int start_child);
void push_scope(vector<map<string, id_attrs> > *sym_table);
void output_map(map<string, id_attrs> m);
void process_formal_list(parsetree *root, vector<map<string, id_attrs> > *sym_table);
void insert_sym_table(parsetree *node, id_type type, int seq, vector<map<string, id_attrs> > *sym_table);
void process_decl_list(parsetree *root, vector<map<string, id_attrs> > *sym_table);
void symbol_table(parsetree *root, vector<map<string, id_attrs> > *sym_table);
id_attrs* in_scope(string id, vector<map<string, id_attrs> > *sym_table);
bool key_exists(string id, map<string, id_attrs> m);
bool in_top_scope(string id, vector<map<string, id_attrs> > *sym_table);
bool in_global_scope(string id, stack<map<string, id_attrs> > *sym_table);
string get_id_type_name(id_type idt);
