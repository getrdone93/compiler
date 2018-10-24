#include "quad.h"

quad three_arity_quad(nodetype op, string opd1, string opd2) {
  return four_arity_quad(op, opd1, opd2, "");
}

quad two_arity_quad(nodetype op, string opd1) {
  return three_arity_quad(op, opd1, "");
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


string dash_if_empty(string s) {
  return s.empty() ? "-" : s;
}

void print_quad_list(list<quad> quads) {
  for (list<quad>::iterator it = quads.begin(); it != quads.end(); it++) {
    cout << quad_to_string(*it);
  }
}

// string quad_to_string(quad q) {
//   return "(" + string(nodenames[q.type]) + ", " + string(dash_if_empty(q.dest)) + ", " 
//     + string(dash_if_empty(q.opd2)) + ", " + string(dash_if_empty(q.opd3)) + ", " + string(dash_if_empty(q.opd4)) + ")\n";
// }

string to_str(int num) {
  stringstream ss;
  ss << num;
  return ss.str();
}

nodetype test_for_error(nodetype type, int nodenames_size) {
  return type > nodenames_size || type < 0 ? node_ERROR : type;
}

string quad_to_string(quad q) {
  return "(" + string(nodenames[test_for_error(q.type, 300)]) + ", " + string(dash_if_empty(q.dest)) + ", " 
    + string(dash_if_empty(q.opd2)) + ", " + string(dash_if_empty(q.opd3)) + ")\n";
}

