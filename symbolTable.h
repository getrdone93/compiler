#include <stack>
#include <map>
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
  int v;
};

bool id_array(parsetree *node, int startChild);
void push_scope(stack<map<string, id_attrs> > *symTable);
void output_map(map<string, id_attrs> m);
void process_formal_list(parsetree *root, stack<map<string, id_attrs> > *symTable);
void insert_sym_table(parsetree *node, id_type type, int seq, stack<map<string, id_attrs> > *symTable);
void process_decl_list(parsetree *root, stack<map<string, id_attrs> > *symTable);
void symbol_table(parsetree *root, stack<map<string, id_attrs> > *symTable);
bool in_scope(string id, stack<map<string, id_attrs> > *symTable);
bool in_top_scope(string id, stack<map<string, id_attrs> > *symTable);
bool in_global_scope(string id, stack<map<string, id_attrs> > *symTable);
string get_id_type_name(id_type idt);
