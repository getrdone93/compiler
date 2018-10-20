#include <stdlib.h>
#include <string>
#include <iostream>
#include "symbolTable.h"

using namespace std;

void push_scope(vector<map<string, id_attrs> > *symTable) {
  map<string, id_attrs> scope;
  symTable -> push_back(scope);
  cout << "New scope level entered, size of stack is " << symTable -> size() << "\n";
}

void symbol_table(parsetree *root, vector<map<string, id_attrs> > *sym_table) {
  if (root == NULL) {
    cout << "null root, returning\n";
    return;
  }
  switch(root -> type) {
  case node_translation_unit:
    for (int i = 0; i < 10 && root -> children[i] != NULL; i++) {
      if (root -> children[i] -> type == node_decl) {
	process_decl_list(root -> children[i], sym_table);
      } else {
	symbol_table(root -> children[i], sym_table);
      }
    }
    break;
  case node_external_declaration:
    push_scope(sym_table);
    insert_sym_table(root -> children[0] -> children[1], function, 0, sym_table);
    process_formal_list(root -> children[1], sym_table);

    //run block statements
    if (root -> children[2] -> children[0] != NULL 
	&& (root -> children[2] -> children[0] -> type == node_decl
	    || root -> children[2] -> children[0] -> type == node_decl_list)) {
      process_decl_list(root -> children[2] -> children[0], sym_table); 
    } else {
      symbol_table(root -> children[2] -> children[0], sym_table);
    }
    symbol_table(root -> children[2] -> children[1], sym_table);
    break;
  case node_block: 
    push_scope(sym_table);
    if (root -> children[0] -> type == node_decl_list || root -> children[0] -> type == node_decl) {
      process_decl_list(root -> children[0], sym_table);
    }
    if (root -> children[1] != NULL) {
      symbol_table(root -> children[1], sym_table);
    }

    output_map(sym_table -> back());
    sym_table -> pop_back();
    break;
  case node_IDENTIFIER: {
    id_attrs *atts = in_scope(root -> str_ptr, sym_table);
    if (atts == NULL) {
      cout << "ERROR: symbol " << root -> str_ptr << " is out of scope at line: " << root -> line << "\n";
    } else {
      root -> symbol_table_ptr = atts;
    }
  }
    break;
  default: 
    for (int i = 0; i < 10 && root -> children[i] != NULL; i++) {
      symbol_table(root -> children[i], sym_table);
    }
    break;
  }
}

id_attrs* in_scope(string id, vector<map<string, id_attrs> > *sym_table) {
  id_attrs *ida = NULL;
  for (vector<map<string, id_attrs> >::iterator t = --sym_table -> end(); t >= sym_table -> begin(); t--) {
    if (key_exists(id, *t)) {
      ida = &(t -> find(id) -> second);
      break;
    }
  }
  return ida;
}

bool in_global_scope(string id, vector<map<string, id_attrs> > *sym_table) {
  return key_exists(id, sym_table -> front());
}

bool in_top_scope(string id, vector<map<string, id_attrs> > *sym_table) {
  return key_exists(id, sym_table -> back());
}

bool key_exists(string id, map<string, id_attrs> m) {
  return m.find(id) != m.end();
}

void output_map(map<string, id_attrs> m) {
  for (map<string, id_attrs>::const_iterator it = m.begin(); it != m.end(); it++) {
    cout << "id: " << it->first << " type: " << get_id_type_name(it -> second.it) << " is no longer in scope "
	 << " on line: " << it->second.line << "\n";
  }
}

void process_formal_list(parsetree *root, vector<map<string, id_attrs> > *sym_table) {
  switch(root -> type) {
  case node_formal_list:
    for (int i = 0; i < 10 && root -> children[i] != NULL; i++) {
      process_formal_list(root -> children[i], sym_table);
    }
    break;
  case node_int_ident:
    insert_sym_table(root -> children[1], integer, 0, sym_table);
    break;
  case node_formal:
    insert_sym_table(root -> children[0] -> children[1], integer_array, 0, sym_table);
    break;
  default:
    cout << "Error process_formal_list, arrived at node " << nodenames[root -> type] << "\n";
    break;    
  }
}

void process_decl_list(parsetree *root, vector<map<string, id_attrs> > *sym_table) {
  if (root == NULL) {
    cout << "null root, returning\n";
    return;
  }
  switch(root -> type) {
  case node_decl_list:
    for (int i = 0; i < 10 && root -> children[i] != NULL; i++) {
      process_decl_list(root -> children[i], sym_table);
    }     
    break;
  case node_decl:
    if (id_array(root, 1)) {
      insert_sym_table(root -> children[0] -> children[1], integer_array, 0, sym_table);
    } else {
      insert_sym_table(root -> children[0] -> children[1], integer, 0, sym_table);
    }

    if (root -> children[1] -> type == node_i_list) {
      process_decl_list(root -> children[1], sym_table);
    }
    break;
  case node_i_list:
    if (root -> children[0] == NULL) {
      return;
    }
    if (root -> children[0] -> type == node_subs) {
      if (id_array(root, 2)) {
	insert_sym_table(root -> children[1], integer_array, 0, sym_table);
      } else {
	insert_sym_table(root -> children[1], integer, 0, sym_table);
      }

      if (root -> children[2] -> type == node_i_list) {
	process_decl_list(root -> children[2], sym_table);
      }
    } else {
      if (id_array(root, 1)) {
	insert_sym_table(root -> children[0], integer_array, 0, sym_table);
      } else {
	insert_sym_table(root -> children[0], integer, 0, sym_table);
      }

      if (root -> children[1] -> type == node_i_list) {
	process_decl_list(root -> children[1], sym_table);
      }
    }
    break;
  default:
    cout << "Error process_decl_list, arrived at node " << nodenames[root -> type] << "\n";
    break;
  }
}

bool id_array(parsetree *node, int startChild) {
  const bool grandChildExists = node -> children[startChild] -> children[0] != NULL;
  return grandChildExists && node -> children[startChild] -> children[0] -> type == node_subs
    || grandChildExists && node -> children[startChild] -> type == node_subs;
}

void insert_sym_table(parsetree *node, id_type type, int seq, vector<map<string, id_attrs> > *sym_table) {
  id_attrs atts = {
    type,
    node -> line,
    node -> str_ptr,
    0 //initialize variables to 0
  };
  if (in_top_scope(atts.id_name, sym_table)) {
    cout << "ERROR: duplicate declaration of identifier " << atts.id_name << " found on line " << atts.line << "\n"; 
  } else {
    sym_table -> back().insert(pair<string, id_attrs>(atts.id_name, atts));
    cout << "new identifier " << atts.id_name << " with type " << get_id_type_name(atts.it) << " in scope at line: "
	 << atts.line << "\n";
  }
}

string get_id_type_name(id_type idt) {
  switch (idt) {
  case integer:
    return "integer";
  case integer_array:
    return "integer_array";
  case function:
    return "function";
  default:
    return "";
  }
}
