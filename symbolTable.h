#include <stack>
#include <map>
#include <vector>
#include <set>
#include "parse.h"

using namespace std;

enum id_type {
  integer,
  integer_array,
  function
};

struct id_attrs {
  id_type it;
  int line;
  const char *id_name;
  int value;
};

const string LOAD = "ldr";
const string MOV = "mov";
const string SWI_SEEK = "swi\t0x6b";

bool simple_assign_exp(parsetree *root);
string we(parsetree *root);
string output_register();
string update_output(string output, string new_str);
string simple_assignment(parsetree *root, set<string> *regs_avail, set<pair<string, string> > *regs_used);
string sa(parsetree *root, set<string> *regs_avail, set<pair<string, string> > *regs_used);
string grab_reg_by_id(set<string> *regs_avail, set<pair<string, string> > *regs_used, string id);
string grab_register(set<string> *regs);
string load_register(string reg, string value);
bool write_exp(parsetree *root);
string arm_output(parsetree *root, set<string> *regs_avail, set<pair<string, string> > *regs_used,  string *output);
void test_traverse(parsetree *root);
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
