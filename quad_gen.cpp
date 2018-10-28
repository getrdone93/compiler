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

list<nodetype> ground_expr_types() {
  list<nodetype> ground_expr_nt;
  ground_expr_nt.push_back(node_primary_expression);
  return ground_expr_nt;
}

set<nodetype> set_ground_exp() {
  set<nodetype> exp_types;
  exp_types.insert(node_IDENTIFIER);
  exp_types.insert(node_CONSTANT);
  return exp_types;
}

list<quad> ground_expression(parsetree *root, set<nodetype> ground_exp) {
  parsetree *lc = root -> children[0];
  parsetree *lcc = lc == NULL ? NULL : lc -> children[0];
  parsetree *left_child = NULL;
  if (lc == NULL) {
    left_child = NULL;
  } else if (lc -> type == node_primary_expression && lcc != NULL && contains(ground_exp, lcc -> type)) {
    left_child = lcc;
  } else if (lc -> type == node_unary_expression) {
    left_child = lc;
  }

  parsetree *rc = root -> children[2];
  parsetree *rcc = rc == NULL ? NULL : rc -> children[0];
  parsetree *right_child = NULL;
  if (rc == NULL) {
    right_child = NULL;
  } else if (rc -> type == node_primary_expression && rcc != NULL && contains(ground_exp, rcc -> type)) {
    right_child = rcc;
  } else if (rc -> type == node_unary_expression) {
    right_child = rc;
  }

  if (left_child == NULL || right_child == NULL) {
    //do a warn or something
    cout << "ground expression sees nulls\n";
    list<quad> res;
    return res;
  } else {
    list<quad> left = handle_ground_node(lc);
    list<quad> right = handle_ground_node(rc);
    nodetype op_type = root -> children[1] -> type;
    quad expr = four_arity_quad(op_type, next_reg(), left.back().dest, right.back().dest);

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
  exp_types.insert(node_relational_expression);
  exp_types.insert(node_inclusive_or_expression);
  exp_types.insert(node_and_expression);
  exp_types.insert(node_exclusive_or_expression);
  exp_types.insert(node_logical_and_expression);
  return exp_types;
}

bool contains(set<nodetype> types, nodetype type) {
  return types.find(type) != types.end();
}

set<nodetype> set_op_types() {
  //please figure out how to do constants
  set<nodetype> op_types;
  op_types.insert(node_ADD);
  op_types.insert(node_MULT);
  op_types.insert(node_DIVIDE);
  op_types.insert(node_SUBTRACT);
  op_types.insert(node_MOD);
  op_types.insert(node_LESS_EQUAL);
  op_types.insert(node_LESS_THAN);
  op_types.insert(node_GREATER_EQUAL);
  op_types.insert(node_GREATER_THAN);
  op_types.insert(node_EQUAL);
  op_types.insert(node_NOT_EQUAL);
  op_types.insert(node_BITWISE_OR);
  op_types.insert(node_BITWISE_AND);
  op_types.insert(node_BITWISE_XOR);
  op_types.insert(node_LOGICAL_AND);
  return op_types;
}

list<quad> nested_expression(parsetree *root, set<nodetype> set_exp, set<nodetype> ops, set<nodetype> ge) {
  //    cout << "at node: " << nodenames[root -> type] << "\n";
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
    parsetree *mid_child = contains(ops, mc -> type) ? mc : NULL;

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
      return ground_expression(root, ge);
    } else {
      if (left_child != NULL && right_child != NULL) {
	list<quad> l1 = nested_expression(left_child, set_exp, ops, ge);
	list<quad> l2 = nested_expression(right_child, set_exp, ops, ge);
	quad expr = four_arity_quad(mid_child -> type, next_reg(), l1.back().dest, l2.back().dest);

	l1.insert(l1.end(), l2.begin(), l2.end());
	l1.push_back(expr);
	return l1;
      } else {
	list<quad> l1 = nested_expression(left_child == NULL ? right_child : left_child, set_exp, ops, ge);

	parsetree *ground_node = left_child == NULL ? root -> children[0] : root -> children[2];
	list<quad> ge = handle_ground_node(ground_node); 

	quad expr = four_arity_quad(mid_child -> type, next_reg(), l1.back().dest, ge.back().dest);
	
	l1.insert(l1.end(), ge.begin(), ge.end());
	l1.push_back(expr);
	return l1;
      }
    }
  }
}

list<quad> handle_ground_node(parsetree *node) {
  list<quad> res;
  if (node -> type == node_unary_expression) {
      parsetree *lc = node -> children[0];
      parsetree *rc = node -> children[1];
      res.push_back(store_leaf(rc -> children[0]));
      res.push_back(three_arity_quad(lc -> children[0] -> type, next_reg(), 
				     rc -> children[0] -> symbol_table_ptr -> id_name));
      //need another instruction here for increments
  } else {
    res.push_back(store_leaf(node -> children[0]));
  }
  return res;
}

list<quad> handle_assignment(parsetree *root) {
  list<quad> res;

  set<nodetype> expr_types = set_expression_types();
  set<nodetype> ops = set_op_types();
  set<nodetype> ge = set_ground_exp();
  parsetree *lc = root -> children[0];
  parsetree *rc = root -> children[2];
  quad sa = simple_assignment(lc -> children[0], root -> children[1], rc -> children[0] -> type == node_CONSTANT ? 
			      rc -> children[0] : NULL);
  if (sa.type == node_ERROR) {
    list<quad> lis = nested_expression(root -> children[2], expr_types, ops, ge);
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
  //  cout << "at node: " << nodenames[root -> type] << "\n";
  switch(root -> type) {
    case node_assignment_expression: {
      list<quad> assign = handle_assignment(root);
      return assign;
    }
    break;
  case node_statement:
    res.push_back(write_exp_quad(root -> children[0], root -> children[1] -> children[0]));
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

