#include "arm_output.h"

string error(string func, string error) {
  return "ERROR in function " + func + ": " + error + "\n";
}

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

quad load_leaf_new(parsetree *node, set<string> *regs_avail, set<pair<string, string> > *regs_used) {
  quad reg_expr;
  switch (node -> type) {
     case node_IDENTIFIER:
       reg_expr = load_into_reg(node -> symbol_table_ptr -> id_name, to_string(node -> symbol_table_ptr -> value),
		     regs_avail, regs_used);
       break;
    case node_CONSTANT:
       reg_expr = load_into_reg(node -> str_ptr, node -> str_ptr, regs_avail, regs_used);
       break;
    default:
      break;
  }
  return reg_expr;
}

pair<string, string> release_reg(string id, set<string> *regs_avail, set<pair<string, string> > *regs_used) {
    pair<string, string> up = lookup_str(id, regs_used);
    if (!up.first.empty()) {
      regs_avail -> insert(up.first);
      regs_used -> erase(up);
    }
    return up;
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

quad simple_assignment(parsetree *ident, parsetree *assign, parsetree *constant, set<string> *regs_avail, 
			 set<pair<string, string> > *regs_used) {
  quad res;
  if (ident == NULL || assign == NULL || constant == NULL) {
    cout << "simple assignment, null node\n";
    //do debug or something
  } else {
    assign_to_ident(ident, constant);
    string reg = assoc_if_not_used(ident -> symbol_table_ptr -> id_name, regs_avail, regs_used);
    res = three_arity_quad(node_STOR, reg, arm_constant(constant -> str_ptr));
  }
  return res;
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


string list_pairs_to_string(list<pair<string, string> > lis) {
   string res;
   for (list<pair<string, string> >::iterator it = lis.begin(); it != lis.end(); it++) {
	res += it -> second;
   }
   return res;
}

void output_pair(pair<string, string> p, string var_name) {
  cout << var_name << ".first " << p.first << "\t" << var_name << ".second " << p.second << "\n";
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
    quad left = load_leaf_new(left_child, regs_avail, regs_used);
    quad right = load_leaf_new(right_child, regs_avail, regs_used);
    nodetype op_type = zero_depth_child(root, 1, operator_types()) -> type;
    //reuse register of left leaf for expression, could reuse right leaf's register as well
    quad expr = four_arity_quad(op_type, left.dest, left.dest, right.dest);
    release_reg(left.dest, regs_avail, regs_used);
    release_reg(right.dest, regs_avail, regs_used);

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

  output_node(left_child, "left_child");
  output_node(mid_child, "mid_child");
  output_node(right_child, "right_child");

  if (root == NULL || mid_child == NULL) {
    //do debug or something
    cout << "not an eggsicutable eggspression\n";
    list<quad> res;
    return res;
  } else {
    if (left_child == NULL && right_child == NULL) {
      return ground_expression(root, regs_avail, regs_used);
    } else {
      if (left_child != NULL && right_child != NULL) {
	list<quad> l1 = nested_expression(left_child, regs_avail, regs_used, exp_types);
	list<quad> l2 = nested_expression(right_child, regs_avail, regs_used, exp_types);
	l1.insert(l1.end(), l2.begin(), l2.end());
	return l1;
      } else {
	parsetree *id_node = get_id_or_const(root, left_child == NULL ? 0 : 2);
	quad reg_load = load_leaf_new(id_node, regs_avail, regs_used);
	list<quad> l1 = nested_expression(left_child == NULL ? right_child : left_child, regs_avail, 
							   regs_used, exp_types);
	l1.push_front(reg_load);	
	
	//release_reg(reg_load.first, regs_avail, regs_used);
	return l1;
      }
    }
  }
}

list<quad> handle_assignment(parsetree *root, set<string> *regs_avail, set<pair<string, string> > *regs_used) {
  list<quad> res;
  parsetree *id = get_ident(root, 0);
  parsetree *assign = get_assign(root);
  parsetree *con = get_const(root, 2);
  if (id == NULL) {
    cout << "id is null\n";
  }
  if (assign == NULL) {
    cout << "assign is null\n";
  }

  if (con == NULL) {
    cout << "con is null\n";
  }

  quad sa = simple_assignment(get_ident(root, 0), get_assign(root), get_const(root, 2), regs_avail, regs_used);
  res.push_back(sa);
  if (res.empty()) {
    list<quad> lis = nested_expression(zero_depth_child(root, 2, expression_types()), 
							    regs_avail, regs_used, expression_types());
    res.insert(res.end(), lis.begin(), lis.end());
  }
  return res;
}


list<quad> arm_output_new(parsetree *root, set<string> *regs_avail, set<pair<string, string> > *regs_used, 
			  list<quad> res) {
  switch(root -> type) {
  case node_assignment_expression:
      return handle_assignment(root, regs_avail, regs_used);
      break;
    case node_statement:
      // if (write_exp(root)) {
      // 	const string id = root -> children[1] -> children[0] -> str_ptr;
      // 	print_register(lookup_str(id, regs_used).first);	
      // 	release_reg(id, regs_avail, regs_used);
      // }
      break;
    default:
      for (int i = 0; i < 10 && root -> children[i] != NULL; i++) {
	return arm_output_new(root -> children[i], regs_avail, regs_used, res);
      }      
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

string to_string(int num) {
  stringstream ss;
  ss << num;
  return ss.str();
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

quad four_arity_quad(nodetype op, string dest, string opd2, string opd3) {
  quad q = {
    op,
    dest,
    opd2,
    opd3
  };
  return q;
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

quad three_arity_quad(nodetype op, string opd1, string opd2) {
  return four_arity_quad(op, opd1, opd2, "");
}

string two_arity(string op, string opd1) {
  if (op.empty()) {
    error(__FUNCTION__, "op was empty");
  } else if (opd1.empty()) {
    error(__FUNCTION__, "opd1 was empty");
  }
  return op + "\t" + opd1 + "\n";
}

string dash_if_empty(string s) {
  return s.empty() ? "-" : s;
}

void print_quad_list(list<quad> quads) {
  for (list<quad>::iterator it = quads.begin(); it != quads.end(); it++) {
    cout << quad_to_string(*it);
  }
}

string quad_to_string(quad q) {
  return "(" + string(nodenames[q.type]) + ", " + string(dash_if_empty(q.dest)) + ", " 
    + string(dash_if_empty(q.opd2)) + ", " + string(dash_if_empty(q.opd3)) + ", " + string(dash_if_empty(q.opd4)) + ")\n";
}

quad two_arity_quad(nodetype op, string opd1) {
  return three_arity_quad(op, opd1, "");
}

string arm_constant(string val) {
  return "=" + val;
}

string arm_small_constant(string val) {
  return "#" + val;
}
