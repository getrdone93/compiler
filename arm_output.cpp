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

list<string> declare_idents(set<string> idents) {
  list<string> decs;
  for (set<string>::iterator it = idents.begin(); it != idents.end(); it++) {
    string id = *it;
    decs.push_back(three_arity_nc(make_label(id), WORD, DEFAULT_VALUE));
  }

  return decs;
}

list<string> process_quads(list<quad> quads) {
  vector<arm_register> arms = make_registers(10);
  return process_quads(quads, &arms);
}

list<string> stor(quad store, arm_register *id_add, arm_register *value_reg) {
  id_add -> dt = MEM_ADD;

  list<string> res;
  res.push_back(three_arity(LOAD, regify(id_add -> number), arm_constant(store.dest)));
  if (!boost::starts_with(store.opd1, "R")) {
    value_reg -> dt = CONST;
    res.push_back(three_arity(LOAD, regify(value_reg -> number), arm_constant(store.opd1)));
  }
  res.push_back(three_arity(STOR, regify(value_reg -> number), at_address(regify(id_add -> number))));

  //value has been stored in identifier so regs are free
  id_add -> dt = NONE;
  value_reg -> dt = NONE;
  return res;
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

list<string> process_quads(list<quad> quads, vector<arm_register> *regs) {
  list<string> res;
  list<string> decs = declare_idents(get_idents(quads));
  res.insert(res.end(), decs.begin(), decs.end());

  for (list<quad>::iterator it = quads.begin(); it != quads.end(); it++) {
    quad cq = *it;
    switch(cq.type) {
        case node_STOR: {
	  vector<int> free_regs = regs_with_dt(*regs, NONE);
	  list<string> stor_asm = stor(cq, &(regs -> at(free_regs.at(0))), &(regs -> at(free_regs.at(1))));
	  res.insert(res.end(), stor_asm.begin(), stor_asm.end());
	}
	break;
      default:
	cout << "dont have rule for nodetype: " << nodenames[cq.type] << "\n";
	break;
    }
  }
  return res;
}

string four_arity(string op, string dest, string opd1, string opd2) {
  return op + "\t" + dest + ", " + opd1 + ", " + opd2 + "\n";
}

string three_arity(string op, string dest, string opd1) {
  return op + "\t" + dest + ", " + opd1 + "\n";
}

string three_arity_nc(string op, string dest, string opd1) {
  return op + "\t" + dest + " " + opd1 + "\n";
}

string two_arity(string op, string dest) {
  return op + "\t" + dest + "\n";
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




