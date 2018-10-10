#include <stack>
#include <map>
using namespace std;

// bool idArray(parsetree *node, int startChild);
// void pushScope(stack<map<string, id_type> > *symTable);
// void outputMap(map<string, id_type> m);
// void process_formal_list(parsetree *root, stack<map<string, id_type> > *symTable);
// void insertSymTable(string ident, id_type idt, stack<map<string, id_type> > *symTable, int line);
// void process_decl_list(parsetree *root, stack<map<string, id_type> > *symTable);
// void symbolTable(parsetree *root, stack<map<string, id_type> > *symTable);
// bool inScope(string id, stack<map<string, id_type> > *symTable);
// bool inTopScope(string id, stack<map<string, id_type> > *symTable);
// bool inGlobalScope(string id, stack<map<string, id_type> > *symTable);
// string getIdTypeName(id_type idt);

void pushScope(stack<map<string, id_type> > *symTable) {
  map<string, id_type> scope;
  symTable -> push(scope);
  cout << "New scope level entered, size of stack is " << symTable -> size() << "\n";
}

void symbolTable(parsetree *root, stack<map<string, id_type> > *symTable) {
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
	  symbolTable(root -> children[i], symTable);
	}
      }
      break;
    case node_external_declaration:
        pushScope(symTable);
	insertSymTable(root -> children[0] -> children[1] -> str_ptr, function, symTable, 
		       root -> children[0] -> children[1] -> line);
	process_formal_list(root -> children[1], symTable);

	//run block statements
	if (root -> children[2] -> children[0] != NULL 
	    && (root -> children[2] -> children[0] -> type == node_decl
		|| root -> children[2] -> children[0] -> type == node_decl_list)) {
	  process_decl_list(root -> children[2] -> children[0], symTable); 
	} else {
	  symbolTable(root -> children[2] -> children[0], symTable);
	}
	symbolTable(root -> children[2] -> children[1], symTable);
      break;
    case node_block: 
        pushScope(symTable);
	if (root -> children[0] -> type == node_decl_list || root -> children[0] -> type == node_decl) {
	  process_decl_list(root -> children[0], symTable);
	}
	if (root -> children[1] != NULL) {
	  symbolTable(root -> children[1], symTable);
	}

	outputMap(symTable -> top());
	symTable -> pop();
      break;
    case node_IDENTIFIER:
      if (inScope(root -> str_ptr, symTable)) {
	cout << "I see symbol " << root -> str_ptr << " and its in scope at line: " << root -> line << "\n";
      } else {
	cout << "ERROR: symbol " << root -> str_ptr << " is out of scope at line: " << root -> line << "\n";
      }
      break;
    default: 
      for (int i = 0; i < 10 && root -> children[i] != NULL; i++) {
	symbolTable(root -> children[i], symTable);
      }
      break;
  }
}

bool inScope(string id, stack<map<string, id_type> > *symTable) {
  return inTopScope(id, symTable) || inGlobalScope(id, symTable);
}

bool inGlobalScope(string id, stack<map<string, id_type> > *symTable) {
  stack<map<string, id_type> > copySymTable = *symTable;
  map<string, id_type> global_scope = copySymTable.top();
  while(copySymTable.size() > 1) {
    copySymTable.pop();
    global_scope = copySymTable.top();
  }
  return global_scope.find(id) != global_scope.end();
}

bool inTopScope(string id, stack<map<string, id_type> > *symTable) {
  return symTable -> top().find(id) != symTable -> top().end();
}

void outputMap(map<string, id_type> m) {
  for (map<string, id_type>::const_iterator it = m.begin(); it != m.end(); it++) {
    cout << "id: " << it->first << " type: " << getIdTypeName(it->second) << " is no longer in scope\n";
  }
}

void process_formal_list(parsetree *root, stack<map<string, id_type> > *symTable) {
  switch(root -> type) {
    case node_formal_list:
      for (int i = 0; i < 10 && root -> children[i] != NULL; i++) {
	process_formal_list(root -> children[i], symTable);
      }
      break;
    case node_int_ident:
      insertSymTable(root -> children[1] -> str_ptr, integer, symTable, root -> children[1] -> line);
      break;
    case node_formal:
      insertSymTable(root -> children[0] -> children[1] -> str_ptr, integer_array, symTable,
		     root -> children[0] -> children[1] -> line);
      break;
    default:
      cout << "Error process_formal_list, arrived at node " << nodenames[root -> type] << "\n";
      break;    
  }
}

void process_decl_list(parsetree *root, stack<map<string, id_type> > *symTable) {
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
      if (idArray(root, 1)) {
	insertSymTable(root -> children[0] -> children[1] -> str_ptr, integer_array, symTable,
		       root -> children[0] -> children[1] -> line);
      } else {
	insertSymTable(root -> children[0] -> children[1] -> str_ptr, integer, symTable,
		       root -> children[0] -> children[1] -> line);
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
	if (idArray(root, 2)) {
	  insertSymTable(root -> children[1] -> str_ptr, integer_array, symTable,
			 root -> children[1] -> line);
	} else {
	  insertSymTable(root -> children[1] -> str_ptr, integer, symTable,
			 root -> children[1] -> line);
	}

	if (root -> children[2] -> type == node_i_list) {
	  process_decl_list(root -> children[2], symTable);
	}
      } else {
	if (idArray(root, 1)) {
	  insertSymTable(root -> children[0] -> str_ptr, integer_array, symTable, root -> children[0] -> line);
	} else {
	  insertSymTable(root -> children[0] -> str_ptr, integer, symTable, root -> children[0] -> line);
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

bool idArray(parsetree *node, int startChild) {
  const bool grandChildExists = node -> children[startChild] -> children[0] != NULL;
  return grandChildExists && node -> children[startChild] -> children[0] -> type == node_subs
    || grandChildExists && node -> children[startChild] -> type == node_subs;
}

void insertSymTable(string ident, id_type idt, stack<map<string, id_type> > *symTable, int line) {
  map<string, id_type> tempMap = symTable -> top();
  tempMap.insert(pair<string, id_type>(ident, idt));
  symTable -> pop();
  symTable -> push(tempMap);
  cout << "new identifier " << ident << " with type " << getIdTypeName(idt) << " in scope at line: " << line << "\n";
}

string getIdTypeName(id_type idt) {
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

