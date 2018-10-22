#include "arm_output.h"

void test_traverse(parsetree *root) {
  switch(root -> type) {
  case node_IDENTIFIER:
    if (root -> symbol_table_ptr == NULL) {
      cout << "sym pointer: " << root -> symbol_table_ptr << " id name: " << root -> str_ptr << " line: "
	   << root -> line << "\n";
    } else {
      cout << "ID: " << root -> symbol_table_ptr -> id_name << " line: " << root -> symbol_table_ptr -> line
	   << " value: " << root -> symbol_table_ptr -> value << "\n";
    }
    break;
  default:
    for (int i = 0; i < 10 && root -> children[i] != NULL; i++) {
      test_traverse(root -> children[i]);
    }
    break;
  }
}

string load_ident(parsetree *p_expr, set<string> *regs_avail, set<pair<string, string> > *regs_used) {
  string expr;
  if (p_expr -> type == node_IDENTIFIER) {
    pair<string, string> entry = lookup_str(p_expr -> symbol_table_ptr -> id_name, regs_used);
    if (entry.first.empty()) {
      string reg = assoc_id_reg(regs_avail, regs_used, p_expr -> symbol_table_ptr -> id_name);
      expr = load_register(reg, p_expr -> symbol_table_ptr -> value);
    }
  }
  return expr;
}

string error(string func, string error) {
  return "ERROR in function " + func + ": " + error + "\n";
}

pair<string, string> load_into_reg(string id, string value, set<string> *regs_avail, 
				   set<pair<string, string> > *regs_used) {
  string load_str;
  string into_reg;
  if (id.empty() || value.empty()) {
    cout << error("load", "id or value or both was empty");
  } else {
    pair<string, string> entry = lookup_str(id, regs_used);
    into_reg = entry.first.empty() ? assoc_id_reg(regs_avail, regs_used, id) : entry.first;
    load_str = load_register(id, value);
  }
  return pair<string, string>(into_reg, load_str);
}

string load_const(parsetree *p_expr, set<string> *regs_avail, set<pair<string, string> > *regs_used) {
  string expr;
  if (p_expr -> type == node_CONSTANT) {
    string cname = p_expr -> str_ptr;
    string reg = lookup_str(cname, regs_used).first;
    if (reg.empty()) {
      string reg = assoc_id_reg(regs_avail, regs_used, cname);
      expr = load_register(reg, p_expr -> str_ptr);
    }
  }
  return expr;
}

bool ground_expr(parsetree *expr_node) {
  return expr_node -> children[0] -> type == node_primary_expression
    && (expr_node -> children[0] -> children[0] -> type == node_CONSTANT 
	|| expr_node -> children[0] -> children[0] -> type == node_IDENTIFIER)
    && expr_node -> children[2] -> type == node_primary_expression
    && (expr_node -> children[2] -> children[0] -> type == node_CONSTANT 
	|| expr_node -> children[2] -> children[0] -> type == node_IDENTIFIER);
}

string operator_to_arm(parsetree *op_node) {
  string arm_op;
  switch(op_node -> type) {
    case node_SUBTRACT:
      arm_op = SUB;
      break;
    case node_MULT:
      arm_op = MULT;
      break;
    case node_ADD:
      arm_op = ADD;
      break;
    default:
      break;
  }
  return arm_op;
}

string load_leaf(parsetree *node, set<string> *regs_avail, set<pair<string, string> > *regs_used) {
  string expr;
  switch (node -> type) {
    case node_IDENTIFIER:
      expr = load_ident(node, regs_avail, regs_used);
      break;
    case node_CONSTANT:
      expr = load_const(node, regs_avail, regs_used);
      break;
  default:
    break;
  }
  return expr;
}

string load_leafs(parsetree *expr_node, set<string> *regs_avail, set<pair<string, string> > *regs_used) {
  string res;
  parsetree *left_node = expr_node -> children[0] -> children[0];
  parsetree *right_node = expr_node -> children[2] -> children[0];
  res = update_output(res, load_leaf(left_node, regs_avail, regs_used));
  res = update_output(res, load_leaf(right_node, regs_avail, regs_used));
  return res;
}

string eval_expr(parsetree *expr_node, set<string> *regs_avail, set<pair<string, string> > *regs_used) {
  string res;
  string expr_reg = first(regs_avail); 
  string op = operator_to_arm(expr_node -> children[1]);
  parsetree *left_node = expr_node -> children[0] -> children[0];
  parsetree *right_node = expr_node -> children[2] -> children[0];
  string fv = lookup_str(left_node -> str_ptr, regs_used).first;
  string sv = lookup_str(right_node -> str_ptr, regs_used).first;
  
  string expr = op + "\t" + expr_reg + ", " + fv + ", " + sv;
  regs_used -> insert(pair<string, string>(expr_reg, expr));
  return expr;
}

void release_reg(parsetree *root, set<string> *regs_avail, set<pair<string, string> > *regs_used) {
  if (root -> type == node_IDENTIFIER || root -> type == node_CONSTANT) {
    release_reg(root -> str_ptr, regs_avail, regs_used);
  }
}

void release_reg(string id, set<string> *regs_avail, set<pair<string, string> > *regs_used) {
    pair<string, string> up = lookup_str(id, regs_used);
    if (!up.first.empty()) {
      regs_avail -> insert(up.first);
      regs_used -> erase(up);
    }
}

int to_int(string str) {
  int num;
  stringstream ss(str);
  ss >> num;
  return num;
}

int get_value(parsetree *node) {
  int v;
  switch (node -> type) {
  case node_IDENTIFIER:
    v = node -> symbol_table_ptr -> value;
    break;
  case node_CONSTANT:
    v = to_int(node -> str_ptr);
    break;
  }
  return v;
}

string eval_unary_expr(parsetree *expr) {
  string res;
  switch (expr -> children[0] -> children[0] -> type) {
  case node_UNARY_MINUS: {
    int v = get_value(expr -> children[1] -> children[0]);
    res = to_string(-v);
  }
    break;
  case node_NEGATE:
    int v = get_value(expr -> children[1] -> children[0]);
    res = to_string(!v);
    break;
  }
  return res;
}

string arm_output(parsetree *root, set<string> *regs_avail, set<pair<string, string> > *regs_used, string *output) {
  switch (root -> type) {
  case node_assignment_expression: {
    parsetree *expr_root = root -> children[2];
    switch (expr_root -> type) {
    case node_primary_expression:
      *output = update_output(*output, simple_assignment(root, regs_avail, regs_used));	
      break;
    case node_additive_expression:
    case node_multiplicative_expression: {
      string expr = arm_output(expr_root, regs_avail, regs_used, output);
      string expr_reg = lookup_str(expr, regs_used).first;
      string to_reg = lookup_str(root -> children[0] -> children[0] -> str_ptr, regs_used).first;
      if (to_reg.empty()) {
	to_reg = assoc_id_reg(regs_avail, regs_used, root -> children[0] -> children[0] -> str_ptr);
      }
      *output = update_output(*output, mov(to_reg, expr_reg));
      release_reg(expr_reg, regs_avail, regs_used);
    }
      break;
    case node_unary_expression: {
      string reg = lookup_str(root -> children[0] -> children[0] -> str_ptr, regs_used).first;
      if (reg.empty()) {
	reg = assoc_id_reg(regs_avail, regs_used, root -> children[0] -> children[0] -> str_ptr);
      }
      string val = eval_unary_expr(expr_root);
      *output = update_output(*output, load_register(reg, val));
    }
      break;
    }
  }
    break;
  case node_additive_expression:
  case node_multiplicative_expression:
    if (ground_expr(root)) {
      string leafs = load_leafs(root, regs_avail, regs_used);
      *output = update_output_nnl(*output, leafs);
      string expr = eval_expr(root, regs_avail, regs_used);
      *output = update_output(*output, expr);
      release_reg(root -> children[0] -> children[0], regs_avail, regs_used);
      release_reg(root -> children[2] -> children[0], regs_avail, regs_used);
      return expr;
    } else {
      string left_expr;      
      if (root -> children[0] -> type == node_additive_expression 
	  || root -> children[0] -> type == node_multiplicative_expression) {
	left_expr = arm_output(root -> children[0], regs_avail, regs_used, output);
	left_expr = lookup_str(left_expr, regs_used).first;
      } else if (root -> children[0] -> children[0] -> type == node_additive_expression
		 || root -> children[0] -> children[0] -> type == node_multiplicative_expression) {
	left_expr = arm_output(root -> children[0] -> children[0], regs_avail, regs_used, output);
	left_expr = lookup_str(left_expr, regs_used).first;
      } else {	
	*output = update_output(*output, load_leaf(root -> children[0] -> children[0], regs_avail, regs_used));
	left_expr = lookup_str(root -> children[0] -> children[0] -> str_ptr, regs_used).first;
      }

      string right_expr;
      if (root -> children[2] -> type == node_additive_expression 
	  || root -> children[2] -> type == node_multiplicative_expression) {
	right_expr = arm_output(root -> children[2], regs_avail, regs_used, output);
	right_expr = lookup_str(right_expr, regs_used).first;
      } else if (root -> children[2] -> children[0] -> type == node_additive_expression
	  || root -> children[2] -> children[0] -> type == node_multiplicative_expression) {
	right_expr = arm_output(root -> children[2] -> children[0], regs_avail, regs_used, output);
	right_expr = lookup_str(right_expr, regs_used).first;
      } else {
	*output = update_output(*output, load_leaf(root -> children[2] -> children[0], regs_avail, regs_used));
	right_expr = lookup_str(root -> children[2] -> children[0] -> str_ptr, regs_used).first;
      }
      
      string expr_reg = first(regs_avail);
      string final_expr = operator_to_arm(root -> children[1]) + "\t" + expr_reg + ", " + left_expr + ", " + right_expr; 
      regs_used -> insert(pair<string, string>(expr_reg, final_expr));
      *output = update_output(*output, final_expr);
      
      release_reg(left_expr, regs_avail, regs_used);
      release_reg(right_expr, regs_avail, regs_used);
      return final_expr;
    }
    break;
  case node_statement:
    if (write_exp(root)) {
      const string id = root -> children[1] -> children[0] -> str_ptr;
      *output = update_output(*output, print_register(lookup_str(id, regs_used).first));	
       release_reg(id, regs_avail, regs_used);
    }
    break;
  case node_postfix_expression: {
    string id = root -> children[0] -> children[0] -> str_ptr;
    string id_reg = lookup_str(id, regs_used).first;
    if (id_reg.empty()) {
      id_reg = assoc_id_reg(regs_avail, regs_used, id);
    }
    string exp = basic_exp(root -> children[1] -> type == node_DEC_OP ? SUB : ADD, id_reg, id_reg, "#1");
    *output = update_output(*output, exp);
    return exp;
  }
    break;
  default:
    for (int i = 0; i < 10 && root -> children[i] != NULL; i++) {
      arm_output(root -> children[i], regs_avail, regs_used, output);
    }
    break;      
  }
  return *output;
}

string basic_exp(string op, string exp_reg, string r1, string r2) {
  return op + "\t" + exp_reg + ", " + r1 + ", " + r2;
}

pair<string, string> lookup_str(string str, set<pair<string, string> > *regs_used) {
  pair<string, string> res;
  for (set<pair<string, string> >::iterator i = regs_used -> begin(); i != regs_used -> end(); i++) {
    if (i -> first == str || i -> second == str) {
      res = *i;
      break;
    }
  }
  return res;
}

bool write_exp(parsetree *root) {
  parsetree *left = root -> children[0];
  parsetree *right = root -> children[1];
  return left != NULL && left -> type == node_WRITE && right != NULL && right -> type == node_primary_expression
    && right -> children[0] != NULL && right -> children[0] -> type == node_IDENTIFIER;
}

string mov(string to_reg, string from_reg) {
  return MOV + "\t" + to_reg + ", " + from_reg;
}

string print_register(string reg) {
  return mov("r1", reg) + "\n"
    + mov("r0", "#1") + "\n"
    + SWI_SEEK;
}

string update_output(string output, string new_str) {
  return new_str.empty() ? output : output + new_str + "\n";
}

string update_output_nnl(string output, string new_str) {
  return new_str.empty() ? output : output + new_str;
}

string simple_assignment(parsetree *root, set<string> *regs_avail, set<pair<string, string> > *regs_used) {
  return simple_assign_exp(root) ? sa(root, regs_avail, regs_used) : "";
}

bool simple_assign_exp(parsetree *root) {
  parsetree *left = root -> children[0];
  parsetree *mid = root -> children[1];
  parsetree *right = root -> children[2];
  return left -> type == node_primary_expression && left -> children[0] -> type == node_IDENTIFIER 
    && mid -> type == node_ASSIGNMENT && right -> type == node_primary_expression 
    && right -> children[0] -> type == node_CONSTANT;
}

string sa(parsetree *root, set<string> *regs_avail, set<pair<string, string> > *regs_used) {
  parsetree *left = root -> children[0] -> children[0];
  parsetree *right = root -> children[2] -> children[0];
  assign_to_ident(left, right);
  string reg = assoc_id_reg(regs_avail, regs_used, left -> symbol_table_ptr -> id_name);
  return load_register(reg, right -> str_ptr);
}

void assign_to_ident(parsetree *ident_node, parsetree *const_node) {
  ident_node -> symbol_table_ptr -> value = to_int(const_node -> str_ptr);
}

string to_string(int num) {
  stringstream ss;
  ss << num;
  return ss.str();
}

string load_register(string reg, int constant) {
  return load_register(reg, to_string(constant));
}

string load_register(string reg, string constant) {
  return LOAD + "\t" + reg + ", =" + constant;
}

string assoc_if_not_used(string id, set<string> *regs_avail, set<pair<string, string> > *regs_used) {
  pair<string, string> id_reg = lookup_str(id, regs_used);
  return id_reg.first.empty() ? assoc_id_reg(regs_avail, regs_used, id) : id_reg.first;
}

string assoc_id_reg(set<string> *regs_avail, set<pair<string, string> > *regs_used, string id) {
  string reg = first(regs_avail);
  if (reg.empty()) {
    error("assoc_id_reg", "regs_avail was empty");
  } else {
    regs_used -> insert(pair<string, string>(reg, id));
  }
  return reg;
}

string first(set<string> *regs) {
  string result;
  if (regs -> size() > 0) {
    result = *regs -> begin();
    regs -> erase(regs -> begin());
  }
  return result;
}

string four_arity(string op, string opd1, string opd2, string opd3) {
  if (op.empty()) {
    error(__FUNCTION__, "op was empty");
  } else if (opd1.empty()) {
    error(__FUNCTION__, "opd1 was empty");
  } else if (opd2.empty()) {
    error(__FUNCTION__, "opd2 was empty");
  } else if (opd3.empty()) {
    error(__FUNCTION__, "opd3 was empty");
  }
  return op + "\t" + opd1 + ", " + opd2 + ", " + opd3 + "\n";
}

string three_arity(string op, string opd1, string opd2) {
  if (op.empty()) {
    error(__FUNCTION__, "op was empty");
  } else if (opd1.empty()) {
    error(__FUNCTION__, "opd1 was empty");
  } else if (opd2.empty()) {
    error(__FUNCTION__, "opd2 was empty");
  }
  return op + "\t" + opd1 + ", " + opd2 + "\n";
}

string two_arity(string op, string opd1) {
  if (op.empty()) {
    error(__FUNCTION__, "op was empty");
  } else if (opd1.empty()) {
    error(__FUNCTION__, "opd1 was empty");
  }
  return op + "\t" + opd1 + "\n";
}
