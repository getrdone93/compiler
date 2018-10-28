#include "arm_output.h"

using namespace std;

list<arm_register> make_registers(int num_regs) {
  list<arm_register> res;
  int i;
  for (i = 1; i <= num_regs; i++) {
    arm_register reg;
    reg.dt = NONE;
    reg.number = i;
    res.push_back(reg);
  }
  return res;
}

list<string> process_quads(list<quad> quads) {
  list<arm_register> arms = make_registers(12);
  return process_quads(quads, &arms);
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
    decs.push_back(three_arity(make_label(id), WORD, DEFAULT_VALUE));
  }

  return decs;
}

string make_label(string id) {
  return id + ":";
}

list<string> process_quads(list<quad> quads, list<arm_register> *regs) {
  list<string> res;
  list<string> decs = declare_idents(get_idents(quads));
  res.insert(res.end(), decs.begin(), decs.end());

  for (list<quad>::iterator it = quads.begin(); it != quads.end(); it++) {
    
  }
  return res;
}

string four_arity(string op, string dest, string opd1, string opd2) {
  if (op.empty()) {
    //error(__FUNCTION__, "op was empty");
  } else if (dest.empty()) {
    //error(__FUNCTION__, "dest was empty");
  } else if (opd1.empty()) {
    //error(__FUNCTION__, "opd1 was empty");
  } else if (opd2.empty()) {
    //error(__FUNCTION__, "opd2 was empty");
  }
  return op + "\t" + dest + ", " + opd1 + ", " + opd2 + "\n";
}

string three_arity(string op, string dest, string opd1) {
  if (op.empty()) {
    //error(__FUNCTION__, "op was empty");
  } else if (dest.empty()) {
    //error(__FUNCTION__, "dest was empty");
  } else if (opd1.empty()) {
    //error(__FUNCTION__, "opd1 was empty");
  }
  return op + "\t" + dest + ", " + opd1 + "\n";
}

string two_arity(string op, string dest) {
  if (op.empty()) {
    //error(__FUNCTION__, "op was empty");
  } else if (dest.empty()) {
    //error(__FUNCTION__, "dest was empty");
  }
  return op + "\t" + dest + "\n";
}

string arm_constant(string val) {
  return "=" + val;
}

string arm_small_constant(string val) {
  return "#" + val;
}

