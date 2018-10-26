#include "arm_output.h"

string next_reg() {
  static int reg_seq = 0;
  return "R" + to_string(reg_seq++);
}

string error(string func, string error) {
  return "ERROR in function " + func + ": " + error + "\n";
}

//delete this
quad load_into_reg(string id, string value, set<string> *regs_avail, 
				   set<pair<string, string> > *regs_used) {
  if (id.empty() || value.empty()) {
    cout << error("load", "id or value or both was empty");
    return two_arity_quad(node_ERROR, "");
  } else {
    string into_reg = assoc_if_not_used(id, regs_avail, regs_used);
    return three_arity_quad(node_STOR, into_reg, arm_constant(value));
  }
}

string operator_to_arm_new(nodetype type) {
  string arm_op;
  switch(type) {
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

quad store_leaf(parsetree *node, set<string> *regs_avail, set<pair<string, string> > *regs_used) {
  quad reg_expr;
  switch (node -> type) {
     case node_IDENTIFIER:
       reg_expr = three_arity_quad(node_STOR, next_reg(), node -> symbol_table_ptr -> id_name);
       break;
    case node_CONSTANT:
       reg_expr = three_arity_quad(node_STOR, next_reg(), node -> str_ptr);
       break;
    default:
      break;
  }
  return reg_expr;
}

//move to gen funcs
pair<string, string> release_reg(string id, set<string> *regs_avail, set<pair<string, string> > *regs_used) {
    pair<string, string> up = lookup_str(id, regs_used);
    if (!up.first.empty()) {
      regs_avail -> insert(up.first);
      regs_used -> erase(up);
    }
    return up;
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


quad simple_assignment(parsetree *ident, parsetree *assign, parsetree *constant, set<string> *regs_avail, 
			 set<pair<string, string> > *regs_used) {
  return ident == NULL || assign == NULL || constant == NULL ? 
    two_arity_quad(node_ERROR, "simple_assignment saw a null input") 
    : three_arity_quad(node_STOR, next_reg(), arm_constant(constant -> str_ptr));
}

parsetree * get_ident(parsetree *ae, int child) {
  list<pair<int, nodetype> > search;
  search.push_back(pair<int, nodetype> (child, node_primary_expression));
  search.push_back(pair<int, nodetype> (0, node_IDENTIFIER));
  return node_search(ae, search);
}

parsetree * get_id_or_const(parsetree* root, int child) {
  parsetree *res = get_ident(root, child);
  if (res == NULL) {
    res = get_const(root, child);
  }
  return res;
}

parsetree * zero_depth_child(parsetree *root, int child, list<nodetype> poss_types) {
  if (root == NULL || poss_types.empty()) {
    //    cout << "returning NULL from zdc\n";
    return NULL;
  } else {
    parsetree *node = zero_depth_child(root, child, poss_types.front());
    if (node == NULL) {
      poss_types.pop_front();
      return zero_depth_child(root, child, poss_types);
    } else {
      //cout << "returning this node from zdc: " << node << "\n";
      return node;
    }
  }
}

parsetree * zero_depth_child(parsetree *root, int child, nodetype type) {
  list<pair<int, nodetype> > search;
  search.push_back(pair<int, nodetype> (child, type));
  return node_search(root, search);
}

parsetree * node_search(parsetree *root, list<pair<int, nodetype> > path) {
    if (path.size() > 0) {
      parsetree *child = root -> children[path.front().first];
      if (child != NULL && child -> type == path.front().second) {
	path.pop_front();
	return node_search(child, path);
      } else {
	return NULL;
      }
    } else {
      return root;
    }
}  

list<nodetype> operator_types() {
  //please figure out how to do constants
  list<nodetype> op_types;
  op_types.push_back(node_ADD);
  op_types.push_back(node_MULT);
  op_types.push_back(node_DIVIDE);
  op_types.push_back(node_SUBTRACT);
  return op_types;
}

list<nodetype> expression_types() {
  //please figure out how to do constants
  list<nodetype> exp_types;
  exp_types.push_back(node_additive_expression);
  exp_types.push_back(node_multiplicative_expression);
  return exp_types;
}

list<nodetype> ground_expr_types() {
  list<nodetype> ground_expr_nt;
  ground_expr_nt.push_back(node_primary_expression);
  return ground_expr_nt;
}

parsetree * get_assign(parsetree *ae) {
  list<pair<int, nodetype> > search;
  search.push_back(pair<int, nodetype> (1, node_ASSIGNMENT));
  return node_search(ae, search);
}

parsetree * get_const(parsetree *ae, int child) {
  list<pair<int, nodetype> > search;
  search.push_back(pair<int, nodetype> (child, node_primary_expression)); 
  search.push_back(pair<int, nodetype> (0, node_CONSTANT));
  return node_search(ae, search);
}


list<quad> ground_expression(parsetree *root, set<string> *regs_avail, 
					      set<pair<string, string> > *regs_used) {
  parsetree *left_child = get_id_or_const(root, 0);
  parsetree *right_child = get_id_or_const(root, 2);
  if (left_child == NULL || right_child == NULL) {
    //do a warn or something
    list<quad> res;
    return res;
  } else {
    quad left = store_leaf(left_child, regs_avail, regs_used);
    quad right = store_leaf(right_child, regs_avail, regs_used);
    nodetype op_type = zero_depth_child(root, 1, operator_types()) -> type;

    release_reg(left.dest, regs_avail, regs_used);
    release_reg(right.dest, regs_avail, regs_used);
    string expr_dest = first(regs_avail);
    quad expr = four_arity_quad(op_type, expr_dest, left.dest, right.dest);
    
    assoc_id_reg(quad_to_string(expr), expr_dest, regs_avail, regs_used);

    list<quad> res;
    res.push_back(left);
    res.push_back(right);
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

list<quad> nested_expression(parsetree *root, set<string> *regs_avail, 
					      set<pair<string, string> > *regs_used, list<nodetype> exp_types) {
  parsetree *left_child = zero_depth_child(root, 0, exp_types);
  parsetree *mid_child = zero_depth_child(root, 1, operator_types());
  parsetree *right_child = zero_depth_child(root, 2, exp_types);

  if (root == NULL || mid_child == NULL) {
    //do debug or something
    cout << "not an eggsicutable eggspression, mid_child: " << mid_child << "\n";
    list<quad> res;
    res.push_back(two_arity_quad(node_ERROR, "nested_expression saw a null input"));
    return res;
  } else {
    if (left_child == NULL && right_child == NULL) {
      return ground_expression(root, regs_avail, regs_used);
    } else {
      if (left_child != NULL && right_child != NULL) {
	list<quad> l1 = nested_expression(left_child, regs_avail, regs_used, exp_types);
	list<quad> l2 = nested_expression(right_child, regs_avail, regs_used, exp_types);

	pair<string, string> l1_reg = lookup_str(quad_to_string(l1.back()), regs_used);
	pair<string, string> l2_reg = lookup_str(quad_to_string(l2.back()), regs_used);
	release_reg(l1_reg.second, regs_avail, regs_used);
	release_reg(l2_reg.second, regs_avail, regs_used);
	string expr_dest = first(regs_avail);
	quad expr = four_arity_quad(mid_child -> type, expr_dest, l1_reg.first, l2_reg.first);

	l1.insert(l1.end(), l2.begin(), l2.end());
	l1.push_back(expr);
	//register expr
	assoc_id_reg(quad_to_string(expr), expr_dest, regs_avail, regs_used);
	return l1;
      } else {
	list<quad> l1 = nested_expression(left_child == NULL ? right_child : left_child, regs_avail, 
							   regs_used, exp_types);
	parsetree *ground_node = get_id_or_const(root, left_child == NULL ? 0 : 2);
	quad reg_load = store_leaf(ground_node, regs_avail, regs_used);
	string expr_dest = first(regs_avail);
	quad expr = four_arity_quad(mid_child -> type, expr_dest, reg_load.dest, l1.back().dest);

	l1.push_back(reg_load);	
	l1.push_back(expr);

	//register expr and release l1 reg
	string l1_expr = quad_to_string(l1.back());
	release_reg(l1_expr, regs_avail, regs_used);
	assoc_id_reg(l1_expr, expr_dest, regs_avail, regs_used);
	return l1;
      }
    }
  }
}

list<quad> handle_assignment(parsetree *root, set<string> *regs_avail, set<pair<string, string> > *regs_used) {
  list<quad> res;

  list<nodetype> expr_types = expression_types();
  parsetree *right_child = zero_depth_child(root, 2, expr_types);
  quad sa = simple_assignment(get_ident(root, 0), get_assign(root), get_const(root, 2), regs_avail, regs_used);
  if (sa.type == node_ERROR) {
    list<quad> lis = nested_expression(right_child, regs_avail, regs_used, expr_types);
    if (lis.back().type == node_ERROR) {
      
    } else {
      res.insert(res.end(), lis.begin(), lis.end());
      
    }
  } else {
    res.push_back(sa);
  }
  return res;
}


list<quad> arm_output_new(parsetree *root, set<string> *regs_avail, set<pair<string, string> > *regs_used, 
			  list<quad> res) {
  switch(root -> type) {
    case node_assignment_expression: {
      list<quad> assign = handle_assignment(root, regs_avail, regs_used);
      return assign;
    }
    break;
    default:
      list<quad> recur_ret;
      for (int i = 0; i < 10 && root -> children[i] != NULL; i++) {
	list<quad> child_quads = arm_output_new(root -> children[i], regs_avail, regs_used, res);
	recur_ret.insert(recur_ret.end(), child_quads.begin(), child_quads.end());
      }
      res.insert(res.begin(), recur_ret.begin(), recur_ret.end());
      break;
  }
  return res;
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

string print_register(string reg) {
  return three_arity(MOV, "r1", reg)
    + three_arity(MOV, "r0", arm_small_constant("1"))
    + two_arity(SWI, SEEK);
}

void assign_to_ident(parsetree *ident_node, parsetree *const_node) {
  ident_node -> symbol_table_ptr -> value = to_int(const_node -> str_ptr);
}

string assoc_if_not_used(string id, set<string> *regs_avail, set<pair<string, string> > *regs_used) {
  pair<string, string> id_reg = lookup_str(id, regs_used);
  return id_reg.first.empty() ? assoc_id_reg(id, regs_avail, regs_used) : id_reg.first;
}

string assoc_id_reg(string id, set<string> *regs_avail, set<pair<string, string> > *regs_used) {
  string reg = first(regs_avail);
  return assoc_id_reg(id, reg, regs_avail, regs_used);
}

string assoc_id_reg(string id, string reg, set<string> *regs_avail, set<pair<string, string> > *regs_used) {
  if (reg.empty() || id.empty()) {
    error("assoc_id_reg", "reg or id was empty");
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

string arm_constant(string val) {
  return "=" + val;
}

string arm_small_constant(string val) {
  return "#" + val;
}
