#include "arm_output.h"
#include <boost/algorithm/string/predicate.hpp>

using namespace std;

vector<arm_register> make_registers(int num_regs) {
  vector<arm_register> res;
  int i;
  for (i = 1; i <= num_regs; i++) {
    arm_register reg;
    reg.dt = NONE;
    reg.number = i;
    res.push_back(reg);
  }
  return res;
}

set<string> get_idents(list<quad> quads) {
  set<string> vars;
  for (list<quad>::iterator it = quads.begin(); it != quads.end(); it++) {
    quad q = *it;
    if (q.type == node_STOR) {
      vars.insert(q.dest);
    }
  }
  return vars;
}

list<quad> declare_idents(set<string> idents) {
  list<quad> decs;
  for (set<string>::iterator it = idents.begin(); it != idents.end(); it++) {
    string id = *it;
    decs.push_back(four_arity_quad(node_LABEL, make_label(id), WORD, DEFAULT_VALUE));
  }

  return decs;
}

list<quad> quads_to_asm(list<quad> quads) {
  vector<arm_register> arms = make_registers(10);
  return quads_to_asm(quads, &arms);
}

list<quad> stor(quad store, arm_register *id_add, arm_register *value_reg) {
  id_add -> dt = MEM_ADD;

  list<quad> res;
  res.push_back(three_arity_quad(node_LOAD, regify(id_add -> number), arm_constant(store.dest)));
  if (!boost::starts_with(store.opd1, "R")) {
    value_reg -> dt = CONST;
    res.push_back(three_arity_quad(node_LOAD, regify(value_reg -> number), arm_constant(store.opd1)));
  }
  res.push_back(three_arity_quad(node_STOR, regify(value_reg -> number), at_address(regify(id_add -> number))));

  //value has been stored in identifier so regs are free
  id_add -> dt = NONE;
  value_reg -> dt = NONE;
  return res;
}

quad load(quad load, arm_register *value_reg) {
  value_reg -> dt = DATA;
  return three_arity_quad(node_LOAD, regify(value_reg -> number), arm_constant(load.opd1));
}

vector<int> regs_with_dt(vector<arm_register> regs, data_type filter) {
  vector<int> res;
  for (vector<arm_register>::iterator ar = regs.begin(); ar != regs.end(); ar++) {
    if (ar -> dt == filter) {
      res.push_back(ar -> number - 1);
    }
  }
  return res;
}

list<quad> quads_to_asm(list<quad> quads, vector<arm_register> *regs) {
  list<quad> res;
  list<quad> decs = declare_idents(get_idents(quads));
  res.insert(res.end(), decs.begin(), decs.end());

  for (list<quad>::iterator it = quads.begin(); it != quads.end(); it++) {
    quad cq = *it;
    switch(cq.type) {
        case node_STOR: {
	  vector<int> free_regs = regs_with_dt(*regs, NONE);
	  list<quad> stor_asm = stor(cq, &(regs -> at(free_regs.at(0))), &(regs -> at(free_regs.at(1))));
	  res.insert(res.end(), stor_asm.begin(), stor_asm.end());
	}
       case node_LOAD: {
	int fr = regs_with_dt(*regs, NONE).at(0);
	res.push_back(load(cq, &(regs -> at(fr))));
	break;
       }
      default:
	cout << "dont have rule for nodetype: " << nodenames[cq.type] << "\n";
	break;
    }
  }
  return res;
}

list<string> asm_quads_to_asm(list<quad> asm_quads) {
  list<string> res;
  for (list<quad>::iterator aq = asm_quads.begin(); aq != asm_quads.end(); aq++) {
    res.push_back(quad_to_arm(*aq));
  }
  return res;
}
 
string quad_to_arm(quad q) {
  string res;
  switch (q.type) {
    case node_LOAD:
      res = three_arity(LOAD, q.dest, q.opd1);
      break;
    case node_STOR:
      res = three_arity(STOR, q.dest, q.opd1);
      break;
    case node_LABEL:
      res = three_arity_nc(q.dest, q.opd1, q.opd2);
      break;
    default:
      res = "default";
      break;
  }
  return res;
}

string arm_constant(string val) {
  return "=" + val;
}

string arm_small_constant(string val) {
  return "#" + val;
}

string at_address(string reg) {
  return "[" + reg + "]"; 
}

string regify(string value) {
  return "R" + value;
}

string regify(int num) {
  return regify(to_string(num));
}

string make_label(string id) {
  return id + ":";
}


