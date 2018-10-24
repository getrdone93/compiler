string eval_expr(parsetree *expr_node, set<string> *regs_avail, set<pair<string, string> > *regs_used) {
  string res;
  string expr_reg = first(regs_avail); 
  string op = operator_to_arm(expr_node -> children[1]);
  parsetree *left_node = expr_node -> children[0] -> children[0];
  parsetree *right_node = expr_node -> children[2] -> children[0];
  string fv = lookup_str(left_node -> str_ptr, regs_used).first;
  string sv = lookup_str(right_node -> str_ptr, regs_used).first;
  
  string expr = four_arity(op, expr_reg, fv, sv);
  regs_used -> insert(pair<string, string>(expr_reg, expr));
  return expr;
}

void release_reg(parsetree *root, set<string> *regs_avail, set<pair<string, string> > *regs_used) {
  if (root -> type == node_IDENTIFIER || root -> type == node_CONSTANT) {
    release_reg(root -> str_ptr, regs_avail, regs_used);
  }
}


string load_ident(parsetree *p_expr, set<string> *regs_avail, set<pair<string, string> > *regs_used) {
  string expr;
  if (p_expr -> type == node_IDENTIFIER) {
    pair<string, string> entry = lookup_str(p_expr -> symbol_table_ptr -> id_name, regs_used);
    if (entry.first.empty()) {
      string reg = assoc_id_reg(regs_avail, regs_used, p_expr -> symbol_table_ptr -> id_name);
      expr = three_arity(LOAD, reg, arm_constant(to_string(p_expr -> symbol_table_ptr -> value)));
    }
  }
  return expr;
}


string load_const(parsetree *p_expr, set<string> *regs_avail, set<pair<string, string> > *regs_used) {
  string expr;
  if (p_expr -> type == node_CONSTANT) {
    string cname = p_expr -> str_ptr;
    string reg = lookup_str(cname, regs_used).first;
    if (reg.empty()) {
      string reg = assoc_id_reg(regs_avail, regs_used, cname);
      expr = three_arity(LOAD, reg, arm_constant(p_expr -> str_ptr));
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

string update_output(string output, string new_str) {
  return new_str.empty() ? output : output + new_str + "\n";
}

string update_output_nnl(string output, string new_str) {
  return new_str.empty() ? output : output + new_str;
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


string arm_output(parsetree *root, set<string> *regs_avail, set<pair<string, string> > *regs_used, string *output) {
  switch (root -> type) {
  case node_assignment_expression: {
    parsetree *expr_root = root -> children[2];
    switch (expr_root -> type) {
    case node_primary_expression:
      *output = update_output(*output, simple_assignment(get_ident(root, 0), get_assign(root), get_const(root, 2), 
							 regs_avail, regs_used));	
      break;
    case node_additive_expression:
    case node_multiplicative_expression: {
      string expr = arm_output(expr_root, regs_avail, regs_used, output);
      string expr_reg = lookup_str(expr, regs_used).first;
      string to_reg = lookup_str(root -> children[0] -> children[0] -> str_ptr, regs_used).first;
      if (to_reg.empty()) {
	to_reg = assoc_id_reg(regs_avail, regs_used, root -> children[0] -> children[0] -> str_ptr);
      }
      *output = update_output_nnl(*output, three_arity(MOV, to_reg, expr_reg));
      release_reg(expr_reg, regs_avail, regs_used);
    }
      break;
    case node_unary_expression: {
      string reg = lookup_str(root -> children[0] -> children[0] -> str_ptr, regs_used).first;
      if (reg.empty()) {
	reg = assoc_id_reg(regs_avail, regs_used, root -> children[0] -> children[0] -> str_ptr);
      }
      string val = eval_unary_expr(expr_root);
      *output = update_output_nnl(*output, three_arity(LOAD, reg, arm_constant(val)));
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
    string exp = four_arity(root -> children[1] -> type == node_DEC_OP ? SUB : ADD, id_reg, id_reg,
			    arm_small_constant("1"));
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
