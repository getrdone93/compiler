#include "quad_gen.h"
#include <boost/algorithm/string/predicate.hpp>

string next_reg() {
  static int reg_seq = 0;
  return "R" + to_string(reg_seq++);
}

string error(string func, string error) {
  return "ERROR in function " + func + ": " + error + "\n";
}

list<quad> load_leaf(parsetree *node) {
  list<quad> leaf;
  switch (node -> type) {
     case node_IDENTIFIER:
       leaf.push_back(three_arity_quad(node_LOAD, next_reg(), node -> symbol_table_ptr -> id_name));
       break;
    case node_CONSTANT:
       leaf.push_back(three_arity_quad(node_LOAD, next_reg(), node -> str_ptr));
       break;
    case node_STRING_LITERAL:
      leaf.push_back(four_arity_quad(node_STR_LBL, "str_label_" + next_reg(), ".asciz", node -> str_ptr));
      leaf.push_back(three_arity_quad(node_LOAD, next_reg(), leaf.back().dest));
      break;
    default:
      cout << "hit default uh oh " << nodenames[node -> type] << "\n";
      break;
  }
  return leaf;
}

set<nodetype> set_ground_exp() {
  set<nodetype> exp_types;
  exp_types.insert(node_IDENTIFIER);
  exp_types.insert(node_CONSTANT);
  return exp_types;
}

list<quad> ground_expression(parsetree *root, set<nodetype> nested_exp, set<nodetype> accepted_exp) {
  //  cout << __FUNCTION__ << " at node: " << nodenames[root -> type] << "\n";
  parsetree *lc = root -> children[0];
  parsetree *left_child = NULL;
  if (contains(accepted_exp, lc -> type)) {
    left_child = lc;
  }

  parsetree *rc = root -> children[2];
  parsetree *right_child = NULL;
  if (contains(accepted_exp, rc -> type)) {
    right_child = rc;
  }

  if (left_child == NULL || right_child == NULL) {
    //do a warn or something
    list<quad> res;
    cout << "WARN, returning empty from " << __FUNCTION__ << "\n";
    return res;
  } else {
    list<quad> left = unary_post_pre_exp(lc, nested_exp, accepted_exp);
    list<quad> right = unary_post_pre_exp(rc, nested_exp, accepted_exp);
    nodetype op_type = root -> children[1] -> type;
    quad expr = four_arity_quad(op_type, next_reg(), 
				left.back().type == node_STOR ? left.back().opd1 : left.back().dest,
				right.back().type == node_STOR ? right.back().opd1 : right.back().dest);

    //handle two pre-increments in same sub exp and assignment on right of exp
    if (((right_child -> type == node_unary_expression && (right_child -> children[0] -> type == node_INC_OP
							 || right_child -> children[0] -> type == node_DEC_OP)
	|| left_child -> type == node_unary_expression && (left_child -> children[0] -> type == node_INC_OP
							   || left_child -> children[0] -> type == node_DEC_OP))
	&& right_child -> type != node_postfix_expression && left_child -> type != node_postfix_expression)
	|| (right_child -> type == node_assignment_expression && ((left_child -> type == node_assignment_expression
								  && strcmp(left_child -> children[0] -> symbol_table_ptr
									    -> id_name, right_child -> children[0]
									    -> symbol_table_ptr -> id_name) == 0)
								  || (left_child -> type == node_IDENTIFIER 
								      && strcmp(left_child -> symbol_table_ptr 
										-> id_name, right_child 
										-> children[0] -> symbol_table_ptr
										-> id_name) == 0)))) {
      quad rb = right.back();
      rb.dest = next_reg();
      right.push_back(rb);
      expr.opd1 = rb.dest;
    } 

    list<quad> res;
    res.insert(res.end(), left.begin(), left.end());
    res.insert(res.end(), right.begin(), right.end());
    res.push_back(expr);

    return res;
  }
}

void output_node(parsetree *node, string var_name) {
  if (node == NULL) {
    cout << var_name << " is NULL\n";
  } else {
    cout << var_name << " -> nodetype: " << nodenames[node -> type] << "\n";
  }
}

set<nodetype> set_expression_types() {
  set<nodetype> exp_types;
  exp_types.insert(node_additive_expression);
  exp_types.insert(node_multiplicative_expression);
  exp_types.insert(node_relational_expression);
  exp_types.insert(node_inclusive_or_expression);
  exp_types.insert(node_and_expression);
  exp_types.insert(node_exclusive_or_expression);
  exp_types.insert(node_logical_and_expression);
  exp_types.insert(node_logical_or_expression);
  return exp_types;
}

bool contains(set<nodetype> types, nodetype type) {
  return types.find(type) != types.end();
}

list<quad> nested_expression(parsetree *root, set<nodetype> set_exp, set<nodetype> ge) {
  //  cout << __FUNCTION__ << " at node: " << nodenames[root -> type] << "\n";
    if (contains(ge, root -> type)) {
      return unary_post_pre_exp(root, set_exp, ge);
    }

    parsetree *lc = root -> children[0];
    parsetree *left_child = NULL;
    if (lc == NULL) {
      left_child = NULL;
    } else if (contains(set_exp, lc -> type)) {
      left_child = lc;
    }

    parsetree *mid_child = root -> children[1];

    parsetree *rc = root -> children[2];
    parsetree *right_child = NULL;
    if (rc == NULL) {
      right_child = NULL;
    } else if (contains(set_exp, rc -> type)) {
      right_child = rc;
    } 

    if (mid_child == NULL) {
    //do debug or something
    cout << "not an eggsicutable eggspression, mid_child: " << mid_child << "\n";
    list<quad> res;
    res.push_back(two_arity_quad(node_ERROR, "nested_expression saw a null input"));
    return res;
  } else {
    if (left_child == NULL && right_child == NULL) {
      return ground_expression(root, set_exp, ge);
    } else {
      if (left_child != NULL && right_child != NULL) {
	list<quad> l1 = nested_expression(left_child, set_exp, ge);
	list<quad> l2 = nested_expression(right_child, set_exp, ge);
	quad expr = four_arity_quad(mid_child -> type, next_reg(), l1.back().dest, l2.back().dest);

	l1.insert(l1.end(), l2.begin(), l2.end());
	l1.push_back(expr);
	return l1;
      } else {
	list<quad> l1 = nested_expression(left_child == NULL ? right_child : left_child, set_exp, ge);

	parsetree *ground_node = left_child == NULL ? root -> children[0] : root -> children[2];
	list<quad> gn = unary_post_pre_exp(ground_node, set_exp, ge); 

	quad expr = four_arity_quad(mid_child -> type, next_reg(), l1.back().dest, 
				    gn.back().type == node_STOR ? gn.back().opd1 : gn.back().dest);
	
	l1.insert(l1.end(), gn.begin(), gn.end());
	l1.push_back(expr);
	return l1;
      }
    }
  }
}

list<quad> handle_assignment(parsetree *root, set<nodetype> exp_types, set<nodetype> ge) {
  list<quad> res;
  parsetree *lc = root -> children[0];
  parsetree *rc = root -> children[2];
  list<quad> right_side = nested_expression(rc, exp_types, ge);

  if (right_side.empty()) {
    cout << "ERROR " << __FUNCTION__ << ", right side of assignment was empty\n";
  } else  {
    res.insert(res.end(), right_side.begin(), right_side.end());
    string last_loc = right_side.back().dest;
    if (boost::starts_with(last_loc, "R")) {
      res.push_back(three_arity_quad(node_STOR, lc -> symbol_table_ptr -> id_name, last_loc));
    } else {
      res.push_back(three_arity_quad(node_LOAD, next_reg(), last_loc));
      res.push_back(three_arity_quad(node_STOR, lc -> symbol_table_ptr -> id_name, res.back().dest));
    }
  }
  return res;
}

list<quad> rw_exp_quad(parsetree *rw_node, parsetree *ident) {
  list<quad> res;
  if (rw_node == NULL || ident == NULL) {
    //do a warn or something
    quad err;
    err.type = node_ERROR;
    err.dest = "write_expression saw a null input";
    res.push_back(err);
  } else {
    list<quad> write = write_to_quad(rw_node, ident);
    list<quad> read = read_to_quad(rw_node, ident);
    res.insert(res.end(), write.begin(), write.end());
    res.insert(res.end(), read.begin(), read.end());
  }

  return res;
}

list<quad> read_to_quad(parsetree *wn, parsetree *ident) {
  list<quad> res;
  if (wn != NULL && ident != NULL && wn -> type == node_READ) {
    res.push_back(three_arity_quad(node_LOAD, next_reg(), "0"));
    res.push_back(three_arity_quad(node_READ, ident -> symbol_table_ptr -> id_name, res.back().dest));
    res.push_back(three_arity_quad(node_STOR, ident -> symbol_table_ptr -> id_name, res.front().dest));
  }
  return res;
}

list<quad> write_to_quad(parsetree *wn, parsetree *ident) {
  list<quad> res;
  if (wn != NULL && ident != NULL && wn -> type == node_WRITE) {
    list<quad> ll_list = load_leaf(ident);
    nodetype wrt = ident -> type == node_STRING_LITERAL ? node_WRITE_STR : node_WRITE;
    if (ll_list.size() == 1) {
      quad ll = ll_list.front();
      res.push_back(ll);
      res.push_back(three_arity_quad(node_LOAD, next_reg(), "1"));
      res.push_back(three_arity_quad(wrt, res.back().dest, ll.dest));    
    } else {
      res.insert(res.end(), ll_list.begin(), ll_list.end());
      res.push_back(three_arity_quad(node_LOAD, next_reg(), "1"));
      res.push_back(three_arity_quad(wrt, res.back().dest, ll_list.back().dest));
    }
  } 
  return res;
}

set<nodetype> post_pre_ops() {
  set<nodetype> unaries;
  unaries.insert(node_INC_OP);
  unaries.insert(node_DEC_OP);
  return unaries;
}

list<quad> prefix_postfix_exp(parsetree *node, set<nodetype> post_pre_ops) {
    list<quad> res;
    parsetree *lc = node -> children[0];
    parsetree *rc = node -> children[1];
    parsetree *leaf = NULL;
    parsetree *op = NULL;
    if (contains(post_pre_ops, lc -> type)) {
      op = lc;      
      leaf = rc;
    } else {
      op = rc;
      leaf = lc;
    }
    quad ll = load_leaf(leaf).front();
    res.push_back(ll);
    res.push_back(three_arity_quad(node_LOAD, next_reg(), "1"));

    nodetype expr_type;
    if (node -> type == node_postfix_expression) {
      expr_type = op -> type == node_INC_OP ? node_POST_ADD : node_POST_SUB;
    } else if (node -> type == node_unary_expression) {
      expr_type = op -> type == node_INC_OP ? node_PRE_ADD : node_PRE_SUB;
    } else {
      cout << __FUNCTION__ << " ERROR, did not see expected nodetype of either postfix_expression or unary_expression\n";
    }
    res.push_back(four_arity_quad(expr_type, next_reg(), ll.dest, res.back().dest));
    res.push_back(three_arity_quad(node_STOR, leaf -> symbol_table_ptr -> id_name, res.back().dest));

    if (node -> type == node_postfix_expression) {
      //force caller to grab register of original leaf prior to bumping it
      res.push_back(three_arity_quad(node_LOAD, ll.dest, ll.dest));
    } else if (node -> type == node_unary_expression) {
      //load last value published for pre inc
      res.push_back(three_arity_quad(node_LOAD, next_reg(), res.back().dest));
    }
    return res;
}
 
list<quad> unary_post_pre_exp(parsetree *node, set<nodetype> nested_exp, set<nodetype> ge) {
  if (node -> type == node_CONSTANT || node -> type == node_IDENTIFIER) {
    list<quad> res;
    res.push_back(load_leaf(node).front());
    return res;
  } else if (contains(nested_exp, node -> type)) {
    return nested_expression(node, nested_exp, ge);
  } else if (node -> type == node_assignment_expression) {
    list<quad> ha = handle_assignment(node, nested_exp, ge);
    ha.push_back(three_arity_quad(node_LOAD, next_reg(), ha.back().dest));
    return ha;
  } else if (node -> type == node_postfix_expression || node -> children[0] -> type == node_INC_OP
	     || node -> children[0] -> type == node_DEC_OP) {
    return prefix_postfix_exp(node, post_pre_ops());
  } else if (node -> type == node_unary_expression) {
    list<quad> right = unary_post_pre_exp(node -> children[1], nested_exp, ge);
    if (node -> children[0] -> type == node_UNARY_MINUS) {
      right.push_back(three_arity_quad(node_LOAD, next_reg(), "0"));
      right.push_back(four_arity_quad(node_SUBTRACT, next_reg(), right.back().dest, right.front().dest));
    } else if (node -> children[0] -> type == node_NEGATE) {
      right.push_back(three_arity_quad(node_NEGATE, next_reg(), right.back().dest));
    } else {
      cout << __FUNCTION__ << " will not handle node: " << nodenames[node -> children[0] -> type] << "\n";
    }
    return right;
  }
}

set<string> get_identifiers(parsetree *root, set<string> res) {
  if (root -> type == node_IDENTIFIER && root -> symbol_table_ptr != NULL) {
    res.insert(root -> symbol_table_ptr -> id_name);
    return res;
  } else {
    for (int i = 0; i < 10 && root -> children[i] != NULL; i++) {
      set<string> ret = get_identifiers(root -> children[i], res);
      res.insert(ret.begin(), ret.end());
    }
    return res;
  }
}

list<quad> handle_if(parsetree *root, set<nodetype> set_exp, set<nodetype> ge) {
  //  cout << __FUNCTION__ << " at node: " << nodenames[root -> type] << "\n";
  list<quad> res;

  parsetree *if_child = root -> children[0];
  parsetree *condition = root -> children[1];
  parsetree *curr_block = root -> children[2];
  parsetree *else_child = root -> children[3];

  list<quad> next_block;
  string next_br;
  
  if (else_child == NULL) {
    next_br = make_end_label(string(if_child -> str_ptr));
  } else {
    next_br = string(else_child -> str_ptr);
    next_block = make_quads(else_child -> children[0], next_block);
    next_block.push_front(two_arity_quad(node_FUNC_LABEL, next_br));    
    //check if label or branch exists because if not then make_quads didn't call back to here
    quad nbb = next_block.back();
    next_block.push_back(two_arity_quad(node_FUNC_LABEL, make_end_label(next_br)));
  }

  list<quad> cond_quads = nested_expression(condition, set_exp, ge);

  list<quad> branch;
  branch.push_back(three_arity_quad(node_LOAD, next_reg(), "0"));
  branch.push_back(three_arity_quad(node_CMP, cond_quads.back().dest, branch.back().dest));
  branch.push_back(two_arity_quad(node_BR_EQ, next_br));
  
  list<quad> block_quads;
  block_quads = make_quads(curr_block, block_quads);
  if (else_child == NULL) {
    block_quads.push_back(two_arity_quad(node_FUNC_LABEL, next_br));    
  } else {
    string el = end_label(else_child, string(else_child -> str_ptr));
    block_quads.push_back(two_arity_quad(node_BR, el));
  }

  res.insert(res.end(), cond_quads.begin(), cond_quads.end());
  res.insert(res.end(), branch.begin(), branch.end());
  res.insert(res.end(), block_quads.begin(), block_quads.end());
  res.insert(res.end(), next_block.begin(), next_block.end());
  return res;
}

int last_child(parsetree *node) {
  int res = -1;
  for (int c = 0; c < 10 && node -> children[c] != NULL; c++) {
    res = c;
  }
  return res;
}

int num_children(parsetree *node) {
  return last_child(node) + 1;
}

string end_label(parsetree *node, string last_else) {
  if (node == NULL || num_children(node) == 0) {
    return last_else;
  } else {
    int lc = last_child(node);
    string if_label = node -> type == node_selection_stmt ? make_end_label(node -> children[0] -> str_ptr)
      : last_else;
    return end_label(node -> children[lc], node -> type == node_ELSE ? make_end_label(string(node -> str_ptr))
		     : if_label);    
  }
}

string make_end_label(string label) {
  return label + "_end";
}

set<nodetype> set_pass_nodes() {
  set<nodetype> nodes;
  nodes.insert(node_ELSE);
  nodes.insert(node_block);
  return nodes;
}

list<quad> handle_for(parsetree *node, set<nodetype> set_exp, set<nodetype> ge) {
  list<quad> res;

  string end_label = make_end_label(node -> children[0] -> str_ptr);
  string begin_label = node -> children[0] -> str_ptr;
  list<quad> assignment = nested_expression(node -> children[1], set_exp, ge);
  assignment.push_back(two_arity_quad(node_FUNC_LABEL, begin_label));

  list<quad> condition = nested_expression(node -> children[2], set_exp, ge);
  quad ld = three_arity_quad(node_LOAD, next_reg(), "0");
  quad cmp = three_arity_quad(node_CMP, condition.back().dest, ld.dest);
  condition.push_back(ld);
  condition.push_back(cmp);
  condition.push_back(two_arity_quad(node_BR_EQ, end_label));

  list<quad> modifier = nested_expression(node -> children[3], set_exp, ge);
  modifier.push_back(two_arity_quad(node_BR, begin_label));
  modifier.push_back(two_arity_quad(node_FUNC_LABEL, end_label));

  list<quad> body;
  body = make_quads(node -> children[4], body);

  res.insert(res.end(), assignment.begin(), assignment.end());
  res.insert(res.end(), condition.begin(), condition.end());
  res.insert(res.end(), body.begin(), body.end());
  res.insert(res.end(), modifier.begin(), modifier.end());
  return res;
}

list<quad> handle_while(parsetree *node, set<nodetype> set_exp, set<nodetype> ge) {
  list<quad> res;
  
  string end_label = make_end_label(node -> children[0] -> str_ptr);
  string begin_label = node -> children[0] -> str_ptr;
  list<quad> condition = nested_expression(node -> children[1], set_exp, ge);
  quad ld = three_arity_quad(node_LOAD, next_reg(), "0");
  quad cmp = three_arity_quad(node_CMP, condition.back().dest, ld.dest);
  condition.push_back(ld);
  condition.push_back(cmp);
  condition.push_back(two_arity_quad(node_BR_EQ, end_label));
  condition.push_front(two_arity_quad(node_FUNC_LABEL, begin_label));

  list<quad> block;
  block = make_quads(node -> children[2], block);
  block.push_back(two_arity_quad(node_BR, begin_label));
  block.push_back(two_arity_quad(node_FUNC_LABEL, end_label));
  
  res.insert(res.end(), condition.begin(), condition.end());
  res.insert(res.end(), block.begin(), block.end());
  return res;
}

list<quad> handle_iter(parsetree *node, set<nodetype> set_exp, set<nodetype> ge) {
  list<quad> res;
  if (node -> children[0] -> type == node_WHILE) {
    res = handle_while(node, set_exp, ge);
  } else if (node -> children[0] -> type == node_FOR) {
    res = handle_for(node, set_exp, ge);
  } else {
    cout << __FUNCTION__ << " WARN: will not process node " << nodenames[node -> type] << "\n";
  }
  return res;
}

list<quad> make_quads(parsetree *root, list<quad> res) {
  if (root == NULL) {
    return res;
  }

  set<nodetype> expr_types = set_expression_types();
  set<nodetype> ge = set_ground_exp();
  ge.insert(node_unary_expression);
  ge.insert(node_postfix_expression);
  ge.insert(node_assignment_expression);
  switch(root -> type) {
    case node_assignment_expression: {
      list<quad> assign = handle_assignment(root, expr_types, ge);
      return assign;
    }
    break;
  case node_unary_expression:
  case node_postfix_expression: {
    list<quad> postfix = unary_post_pre_exp(root, expr_types, ge);
    return postfix;
  }
    break;
  case node_statement: {
    list<quad> read_write = rw_exp_quad(root -> children[0], root -> children[1]);
    res.insert(res.end(), read_write.begin(), read_write.end());
  }
    break;
  case node_selection_stmt: {
    list<quad> if_quads = handle_if(root, expr_types, ge);
    res.insert(res.end(), if_quads.begin(), if_quads.end());
  }
    break;
  case node_iteration_stmt: {
    list<quad> iter_quads = handle_iter(root, expr_types, ge);
    res.insert(res.end(), iter_quads.begin(), iter_quads.end());
  }
    break;
  default:
      list<quad> recur_ret;
      for (int i = 0; i < 10 && root -> children[i] != NULL; i++) {
	list<quad> child_quads = make_quads(root -> children[i], res);
	recur_ret.insert(recur_ret.end(), child_quads.begin(), child_quads.end());
      }
      res.insert(res.begin(), recur_ret.begin(), recur_ret.end());
      break;
  }

  return res;
}

