#include <stack>
#include <map>
#include <stdlib.h>
#include <string>
#include <iostream>
#include "symbolTable.h"

using namespace std;

void push_scope(stack<map<string, id_attrs> > *symTable) {
  map<string, id_attrs> scope;
  symTable -> push(scope);
  cout << "New scope level entered, size of stack is " << symTable -> size() << "\n";
}

void symbol_table(parsetree *root, stack<map<string, id_attrs> > *symTable) {
  if (root == NULL) {
    cout << "null root, returning\n";
    return;
  }
  switch(root -> type) {
    case node_translation_unit:
      for (int i = 0; i < 10 && root -> children[i] != NULL; i++) {
	if (root -> children[i] -> type == node_decl) {
	  process_decl_list(root -> children[i], symTable);
	} else {
	  symbol_table(root -> children[i], symTable);
	}
      }
      break;
    case node_external_declaration:
        push_scope(symTable);
	insert_sym_table(root -> children[0] -> children[1], function, 0, symTable);
	process_formal_list(root -> children[1], symTable);

	//run block statements
	if (root -> children[2] -> children[0] != NULL 
	    && (root -> children[2] -> children[0] -> type == node_decl
		|| root -> children[2] -> children[0] -> type == node_decl_list)) {
	  process_decl_list(root -> children[2] -> children[0], symTable); 
	} else {
	  symbol_table(root -> children[2] -> children[0], symTable);
	}
	symbol_table(root -> children[2] -> children[1], symTable);
      break;
    case node_block: 
        push_scope(symTable);
	if (root -> children[0] -> type == node_decl_list || root -> children[0] -> type == node_decl) {
	  process_decl_list(root -> children[0], symTable);
	}
	if (root -> children[1] != NULL) {
	  symbol_table(root -> children[1], symTable);
	}

	output_map(symTable -> top());
	symTable -> pop();
      break;
    case node_IDENTIFIER:
      if (in_scope(root -> str_ptr, symTable)) {
	cout << "I see symbol " << root -> str_ptr << " and its in scope at line: " << root -> line << "\n";
      } else {
	cout << "ERROR: symbol " << root -> str_ptr << " is out of scope at line: " << root -> line << "\n";
      }
      break;
    default: 
      for (int i = 0; i < 10 && root -> children[i] != NULL; i++) {
	symbol_table(root -> children[i], symTable);
      }
      break;
  }
}

bool in_scope(string id, stack<map<string, id_attrs> > *symTable) {
  return in_top_scope(id, symTable) || in_global_scope(id, symTable);
}

bool in_global_scope(string id, stack<map<string, id_attrs> > *symTable) {
  stack<map<string, id_attrs> > copySymTable = *symTable;
  map<string, id_attrs> global_scope = copySymTable.top();
  while(copySymTable.size() > 1) {
    copySymTable.pop();
    global_scope = copySymTable.top();
  }
  return global_scope.find(id) != global_scope.end();
}

bool in_top_scope(string id, stack<map<string, id_attrs> > *symTable) {
  return symTable -> top().find(id) != symTable -> top().end();
}

void output_map(map<string, id_attrs> m) {
  for (map<string, id_attrs>::const_iterator it = m.begin(); it != m.end(); it++) {
    cout << "id: " << it->first << " type: " << get_id_type_name(it -> second.it) << " is no longer in scope "
	 << " on line: " << it->second.line << "\n";
  }
}

void process_formal_list(parsetree *root, stack<map<string, id_attrs> > *symTable) {
  switch(root -> type) {
    case node_formal_list:
      for (int i = 0; i < 10 && root -> children[i] != NULL; i++) {
	process_formal_list(root -> children[i], symTable);
      }
      break;
    case node_int_ident:
      insert_sym_table(root -> children[1], integer, 0, symTable);
      break;
    case node_formal:
      insert_sym_table(root -> children[0] -> children[1], integer_array, 0, symTable);
      break;
    default:
      cout << "Error process_formal_list, arrived at node " << nodenames[root -> type] << "\n";
      break;    
  }
}

void process_decl_list(parsetree *root, stack<map<string, id_attrs> > *symTable) {
  if (root == NULL) {
    cout << "null root, returning\n";
    return;
  }
  switch(root -> type) {
    case node_decl_list:
      for (int i = 0; i < 10 && root -> children[i] != NULL; i++) {
	 process_decl_list(root -> children[i], symTable);
      }     
      break;
    case node_decl:
      if (id_array(root, 1)) {
	insert_sym_table(root -> children[0] -> children[1], integer_array, 0, symTable);
      } else {
	insert_sym_table(root -> children[0] -> children[1], integer, 0, symTable);
      }

      if (root -> children[1] -> type == node_i_list) {
	process_decl_list(root -> children[1], symTable);
      }
      break;
   case node_i_list:
     if (root -> children[0] == NULL) {
       return;
     }
     if (root -> children[0] -> type == node_subs) {
	if (id_array(root, 2)) {
	  insert_sym_table(root -> children[1], integer_array, 0, symTable);
	} else {
	  insert_sym_table(root -> children[1], integer, 0, symTable);
	}

	if (root -> children[2] -> type == node_i_list) {
	  process_decl_list(root -> children[2], symTable);
	}
      } else {
	if (id_array(root, 1)) {
	  insert_sym_table(root -> children[0], integer_array, 0, symTable);
	} else {
	  insert_sym_table(root -> children[0], integer, 0, symTable);
	}

	if (root -> children[1] -> type == node_i_list) {
	  process_decl_list(root -> children[1], symTable);
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

void insert_sym_table(parsetree *node, id_type type, int seq, stack<map<string, id_attrs> > *symTable) {
  id_attrs atts = {
    type,
    node -> line,
    node -> str_ptr,
    0 //initialize variables to 0
  };
  if (in_top_scope(atts.id_name, symTable)) {
    cout << "ERROR: duplicate declaration of identifier " << atts.id_name << " found on line " << atts.line << "\n"; 
  } else {
    symTable -> top().insert(pair<string, id_attrs>(atts.id_name, atts));
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

