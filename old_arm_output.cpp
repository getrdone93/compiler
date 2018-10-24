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
