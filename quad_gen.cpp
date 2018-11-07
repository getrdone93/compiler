#include "quad_gen.h"
#include <boost/algorithm/string/predicate.hpp>

string next_reg() {
  static int reg_seq = 0;
  return "R" + to_string(reg_seq++);
}

string error(string func, string error) {
  return "ERROR in function " + func + ": " + error + "\n";
}

quad load_leaf(parsetree *node) {
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

set<nodetype> set_ground_exp() {
  set<nodetype> exp_types;
  exp_types.insert(node_IDENTIFIER);
  exp_types.insert(node_CONSTANT);
  return exp_types;
}

list<quad> ground_expression(parsetree *root, set<nodetype> nested_exp, set<nodetype> accepted_exp) {
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
    return res;
  } else {
    list<quad> left = unary_post_pre_exp(lc, nested_exp, accepted_exp);
    list<quad> right = unary_post_pre_exp(rc, nested_exp, accepted_exp);
    nodetype op_type = root -> children[1] -> type;
    quad expr = four_arity_quad(op_type, next_reg(), 
				left.back().type == node_STOR ? left.back().opd1 : left.back().dest,
				right.back().type == node_STOR ? right.back().opd1 : right.back().dest);

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
  res.insert(res.end(), right_side.begin(), right_side.end());
  string last_loc = right_side.back().dest;
  if (boost::starts_with(last_loc, "R")) {
    res.push_back(three_arity_quad(node_STOR, lc -> symbol_table_ptr -> id_name, last_loc));
  } else {
    res.push_back(three_arity_quad(node_LOAD, next_reg(), last_loc));
    res.push_back(three_arity_quad(node_STOR, lc -> symbol_table_ptr -> id_name, res.back().dest));
  }
  return res;
}

list<quad> write_exp_quad(parsetree *write_node, parsetree *ident) {
  list<quad> res;
  if (write_node == NULL || ident == NULL) {
    //do a warn or something
    quad err;
    err.type = node_ERROR;
    err.dest = "write_expression saw a null input";
    res.push_back(err);
  } else {
    quad ll = load_leaf(ident);
    res.push_back(ll);
    res.push_back(three_arity_quad(node_LOAD, next_reg(), "1"));
    res.push_back(three_arity_quad(node_WRITE, res.back().dest, ll.dest));
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
    quad ll = load_leaf(leaf);
    res.push_back(ll);
    res.push_back(three_arity_quad(node_LOAD, next_reg(), "1"));
    res.push_back(four_arity_quad(op -> type == node_INC_OP ? node_POST_ADD : node_POST_SUB, 
				  next_reg(), ll.dest, res.back().dest));
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
    res.push_back(load_leaf(node));
    return res;
  } else if (contains(nested_exp, node -> type)) {
    return nested_expression(node, nested_exp, ge);
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

list<quad> make_quads(parsetree *root, list<quad> res) {
  set<nodetype> expr_types = set_expression_types();
  set<nodetype> ge = set_ground_exp();
  ge.insert(node_unary_expression);
  ge.insert(node_postfix_expression);
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
    list<quad> write_quads = write_exp_quad(root -> children[0], root -> children[1]);
    res.insert(res.end(), write_quads.begin(), write_quads.end());
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

