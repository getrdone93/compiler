#include "quad_gen.h"

string next_reg() {
  static int reg_seq = 0;
  return "R" + to_string(reg_seq++);
}

string error(string func, string error) {
  return "ERROR in function " + func + ": " + error + "\n";
}

quad store_leaf(parsetree *node) {
  quad reg_expr;
  switch (node -> type) {
     case node_IDENTIFIER:
       reg_expr = three_arity_quad(node_LOAD, next_reg(), node -> symbol_table_ptr -> id_name);
       break;
    case node_CONSTANT:
       reg_expr = three_arity_quad(node_LOAD, next_reg(), node -> str_ptr);
       break;
    default:
        cout << "hit default uh oh\n";
      break;
  }
  return reg_expr;
}

quad simple_assignment(parsetree *ident, parsetree *assign, parsetree *constant) {
  return ident == NULL || assign == NULL || constant == NULL ? 
    two_arity_quad(node_ERROR, "simple_assignment saw a null input") 
    : three_arity_quad(node_STOR, ident -> symbol_table_ptr -> id_name, constant -> str_ptr);
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


list<quad> ground_expression(parsetree *root) {
  parsetree *lc = root -> children[0];
  parsetree *lcc = lc == NULL ? NULL : lc -> children[0];
  parsetree *left_child = lc != NULL && lc -> type == node_primary_expression && lcc != NULL
    && (lcc -> type == node_IDENTIFIER || lcc -> type == node_CONSTANT) ? lcc : NULL;


  parsetree *rc = root -> children[2];
  parsetree *rcc = rc == NULL ? NULL : rc -> children[0];
  parsetree *right_child = rc != NULL && rc -> type == node_primary_expression && rcc != NULL
    && (rcc -> type == node_IDENTIFIER || rcc -> type == node_CONSTANT) ? rcc : NULL;

  if (left_child == NULL || right_child == NULL) {
    //do a warn or something
    cout << "ground expression sees nulls\n";
    list<quad> res;
    return res;
  } else {
    quad left = store_leaf(left_child);
    quad right = store_leaf(right_child);
    nodetype op_type = zero_depth_child(root, 1, operator_types()) -> type;
    quad expr = four_arity_quad(op_type, next_reg(), left.dest, right.dest);

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

list<nodetype> expression_types() {
  //please figure out how to do constants
  list<nodetype> exp_types;
  exp_types.push_back(node_additive_expression);
  exp_types.push_back(node_multiplicative_expression);
  return exp_types;
}

set<nodetype> set_expression_types() {
  set<nodetype> exp_types;
  exp_types.insert(node_additive_expression);
  exp_types.insert(node_multiplicative_expression);
  return exp_types;
}

bool contains(set<nodetype> types, nodetype type) {
  return types.find(type) != types.end();
}

list<nodetype> operator_types() {
  //please figure out how to do constants
  list<nodetype> op_types;
  op_types.push_back(node_ADD);
  op_types.push_back(node_MULT);
  op_types.push_back(node_DIVIDE);
  op_types.push_back(node_SUBTRACT);
  op_types.push_back(node_MOD);
  return op_types;
}

list<quad> nested_expression(parsetree *root, list<nodetype> exp_types) {
    set<nodetype> set_exp = set_expression_types();
    parsetree *lc = root -> children[0];
    parsetree *left_child = NULL;
    if (lc == NULL) {
      left_child = NULL;
    } else if (contains(set_exp, lc -> type)) {
      left_child = lc;
    } else if (lc -> type == node_primary_expression && contains(set_exp, lc -> children[0] -> type)) {
      left_child = lc -> children[0];
    }

    parsetree *mc = root -> children[1];
    parsetree *mid_child = mc -> type == node_ADD || mc -> type == node_MULT || mc -> type == node_DIVIDE 
    || mc -> type == node_SUBTRACT || mc -> type == node_MOD ? mc : NULL;

    parsetree *rc = root -> children[2];
    parsetree *right_child = NULL;
    if (rc == NULL) {
      right_child = NULL;
    } else if (contains(set_exp, rc -> type)) {
      right_child = rc;
    } else if (rc -> type == node_primary_expression && contains(set_exp, rc -> children[0] -> type)) {
      right_child = rc -> children[0];
    }

    if (root == NULL || mid_child == NULL) {
    //do debug or something
    cout << "not an eggsicutable eggspression, mid_child: " << mid_child << "\n";
    list<quad> res;
    res.push_back(two_arity_quad(node_ERROR, "nested_expression saw a null input"));
    return res;
  } else {
    if (left_child == NULL && right_child == NULL) {
      return ground_expression(root);
    } else {
      if (left_child != NULL && right_child != NULL) {
	list<quad> l1 = nested_expression(left_child, exp_types);
	list<quad> l2 = nested_expression(right_child, exp_types);
	quad expr = four_arity_quad(mid_child -> type, next_reg(), l1.back().dest, l2.back().dest);

	l1.insert(l1.end(), l2.begin(), l2.end());
	l1.push_back(expr);
	return l1;
      } else {
	list<quad> l1 = nested_expression(left_child == NULL ? right_child : left_child, exp_types);

	int child = left_child == NULL ? 0 : 2;
	parsetree *ground_node = left_child == NULL ? root -> children[0] -> children[0]
	  : root -> children[2] -> children[0];

	quad reg_load = store_leaf(ground_node);
	quad expr = four_arity_quad(mid_child -> type, next_reg(), l1.back().dest, reg_load.dest);

	l1.push_back(reg_load);	
	l1.push_back(expr);
	return l1;
      }
    }
  }
}

list<quad> handle_assignment(parsetree *root) {
  list<quad> res;

  list<nodetype> expr_types = expression_types();
  parsetree *lc = root -> children[0];
  parsetree *rc = root -> children[2];
  quad sa = simple_assignment(lc -> children[0], root -> children[1], rc -> children[0] -> type == node_CONSTANT ? 
			      rc -> children[0] : NULL);
  if (sa.type == node_ERROR) {
    list<quad> lis = nested_expression(root -> children[2], expr_types);
    if (lis.back().type == node_ERROR) {
      
    } else {
      lis.push_back(three_arity_quad(node_STOR, lc -> children[0] -> symbol_table_ptr -> id_name, lis.back().dest));
      res.insert(res.end(), lis.begin(), lis.end());      
    }
  } else {
    res.push_back(sa);
  }
  return res;
}

quad write_exp_quad(parsetree *write_node, parsetree *ident) {
  quad res;
  if (write_node == NULL || ident == NULL) {
    //do a warn or something
    res.type = node_ERROR;
    res.dest = "write_expression saw a null input";
  } else {
    res = two_arity_quad(node_WRITE, ident -> symbol_table_ptr -> id_name);
  }
  return res;
}

list<quad> make_quads(parsetree *root, list<quad> res) {
  cout << "at node: " << nodenames[root -> type] << "\n";
  switch(root -> type) {
    case node_assignment_expression: {
      list<quad> assign = handle_assignment(root);
      cout << "done with handle_assignment, size: " << assign.size() << "\n";
      return assign;
    }
    break;
  case node_statement:
    cout << "made it to node_statement\n";
    res.push_back(write_exp_quad(root -> children[0], root -> children[1] -> children[0]));
    cout << "here now\n";
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

// parsetree * node_search(parsetree *root, list<pair<int, nodetype> > path) {
//   parsetree *recur = root;
//   for (list<pair<int, nodetype> >::iterator it = path.begin(); it != path.end(); it++) {
//     if (recur -> children[it -> first] == NULL || recur -> children[it -> first] -> type != it -> second) {
//       return NULL;
//     } else {
//       recur = recur -> children[it -> first];
//     }
//   }
//   return recur;
// }

parsetree * node_search(parsetree *root, list<pair<int, nodetype> > path) {
    if (path.size() > 0) {
      int child = path.front().first;
      if (root -> children[child] != NULL 
	  && root -> children[child] -> type == path.front().second) {
	path.pop_front();
	return node_search(root -> children[child], path);
      } else {
	return NULL;
      }
    } else {
       //try a copy here or rewrite other funcs to traverse tree
      return root;
    }
}  
