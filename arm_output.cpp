#include "arm_output.h"
#include <boost/algorithm/string/predicate.hpp>

using namespace std;

vector<arm_register> make_registers(int num_regs) {
  vector<arm_register> res;
  int i;
  for (i = 0; i < num_regs; i++) {
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
  vector<arm_register> arms = make_registers(11);
  return quads_to_asm(quads, &arms);
}

list<quad> stor(quad store, vector<arm_register> *regs, map<string, int> *fake_to_real) {
  list<quad> res;
  int ari = regs_with_dt(regs, NONE).front();
  arm_register *address_reg = &(regs -> at(ari));
  address_reg -> dt = MEM_ADD;
  string add_reg = regify(address_reg -> number);
  if (boost::starts_with(store.opd1, "R")) {
    int vr = fake_to_real -> find(store.opd1) -> second;
    arm_register *rv_reg = &(regs -> at(vr));
    string value_reg = regify(rv_reg -> number);
    res.push_back(three_arity_quad(node_LOAD, add_reg, arm_constant(store.dest)));
    res.push_back(three_arity_quad(node_STOR, value_reg, at_address(add_reg)));

    rv_reg -> dt = NONE;
    fake_to_real -> erase(store.opd1);
  } else {
    int fr = regs_with_dt(regs, NONE).front();
    arm_register *rv_reg = &(regs -> at(fr));
    string val_reg = regify(rv_reg -> number);
    res.push_back(three_arity_quad(node_LOAD, add_reg, arm_constant(store.dest)));
    res.push_back(three_arity_quad(node_LOAD, val_reg, arm_constant(store.opd1)));
    res.push_back(three_arity_quad(node_STOR, val_reg, at_address(add_reg)));

    rv_reg -> dt = NONE;
  }
  
  address_reg -> dt = NONE;
  return res;
}

list<quad> load(quad load, arm_register *value_reg, set<string> idents, map<string, int> *fake_to_real) {
  list<quad> res;
  value_reg -> dt = DATA;
  string reg = regify(value_reg -> number);
  fake_to_real -> insert(pair<string, int>(load.dest, value_reg -> number));
  if (contains(idents, load.opd1)) {
    res.push_back(three_arity_quad(node_LOAD, reg, load.opd1));
    res.push_back(three_arity_quad(node_LOAD, reg, at_address(reg)));
  } else {
    res.push_back(three_arity_quad(node_LOAD, reg, arm_constant(load.opd1)));
  }
  return res;
}

bool contains(set<string> strs, string s) {
  return strs.find(s) != strs.end();
}

vector<int> regs_with_dt(vector<arm_register> *regs, data_type filter) {
  vector<int> res;
  int i;
  for (i = 0; i < regs -> size(); i++) {
    if ((regs -> at(i)).dt == filter) {
      res.push_back(i);
    }
  }
  return res;
}

list<quad> binary_operator(quad binary, arm_register *dest_reg, vector<arm_register> *regs, 
			   map<string, int> *fake_to_real) {
  //maybe check if find gets it
  int rr1 = fake_to_real -> find(binary.opd1) -> second;
  int rr2 = fake_to_real -> find(binary.opd2) -> second;
  regs -> at(rr1).dt = NONE;
  regs -> at(rr2).dt = NONE;
  fake_to_real -> erase(binary.opd1);
  fake_to_real -> erase(binary.opd2);
  string opd1 = regify(rr1);
  string opd2 = regify(rr2);

  list<quad> res;
  string dr = regify(dest_reg -> number);
  dest_reg -> dt = DATA;
  fake_to_real -> insert(pair<string, int>(binary.dest, dest_reg -> number));
  
  res.push_back(four_arity_quad(binary.type, dr, opd1, opd2));
  return res;
}

list<quad> quads_to_asm(list<quad> quads, vector<arm_register> *regs) {
  //cout << "quads.size() " << quads.size() << "\n";
  list<quad> res;
  map<string, int> fake_to_real;
  set<string> idents = get_idents(quads);
  list<quad> decs = declare_idents(idents);
  res.insert(res.end(), decs.begin(), decs.end());

  for (list<quad>::iterator it = quads.begin(); it != quads.end(); it++) {
    quad cq = *it;
    switch(cq.type) {
        case node_STOR: {
	  vector<int> free_regs = regs_with_dt(regs, NONE);
	  list<quad> stor_asm = stor(cq, regs, &fake_to_real);
	  res.insert(res.end(), stor_asm.begin(), stor_asm.end());
	}
	 break;
       case node_LOAD: {
	int fr = regs_with_dt(regs, NONE).at(0);
	list<quad> la = load(cq, &(regs -> at(fr)), idents, &fake_to_real);
	res.insert(res.end(), la.begin(), la.end());
       }
	break;
    case node_SUBTRACT: {
      int fr = regs_with_dt(regs, NONE).at(0);
      list<quad> bin_asm = binary_operator(cq, &(regs -> at(fr)), regs, &fake_to_real);
      res.insert(res.end(), bin_asm.begin(), bin_asm.end());
    }
      break;
      default:
	cout << "dont have rule for nodetype: " << nodenames[cq.type] << "\n";
	break;
    }
  }
  //  cout << "res output.size " << res.size() << "\n";
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
    case node_SUBTRACT:
      res = four_arity(SUB, q.dest, q.opd1, q.opd2);
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


