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

list<quad> declare_idents(set<string> idents) {
  list<quad> decs;
  for (set<string>::iterator it = idents.begin(); it != idents.end(); it++) {
    string id = *it;
    decs.push_back(four_arity_quad(node_LABEL, make_label(id), WORD, DEFAULT_VALUE));
  }

  return decs;
}

list<quad> quads_to_asm(list<quad> quads, set<string> idents) {
  vector<arm_register> arms = make_registers(11);
  return quads_to_asm(quads, idents, &arms);
}

list<quad> stor(quad store, vector<arm_register> *regs, map<string, int> *fake_to_real) {
  list<quad> res;
  int ari = regs_with_dt(regs, NONE).front();
  arm_register *address_reg = &(regs -> at(ari));
  address_reg -> dt = MEM_ADD;
  string add_reg = regify(address_reg -> number);
  if (boost::starts_with(store.opd1, "R")) {
    int vr = -1;
    if (fake_to_real -> find(store.opd1) == fake_to_real -> end()) {
      cout << "ERROR: tried looking this up and got nothing back: " << store.opd1 << "\n";
      list<quad> err;
      return err;
    } else {
      vr = fake_to_real -> find(store.opd1) -> second;
    }
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
  if (load.dest.compare(load.opd1) != 0) {
    value_reg -> dt = DATA;
    string reg = regify(value_reg -> number);
    fake_to_real -> insert(pair<string, int>(load.dest, value_reg -> number));
    if (contains(idents, load.opd1)) {
      res.push_back(three_arity_quad(node_LOAD, reg, arm_constant(load.opd1)));
      res.push_back(three_arity_quad(node_LOAD, reg, at_address(reg)));
    } else {
      res.push_back(three_arity_quad(node_LOAD, reg, arm_constant(load.opd1)));
    }
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

nodetype post_to_regular(nodetype type) {
  nodetype res;
  switch(type) {
  case node_POST_ADD:
    res = node_ADD;
    break;
  case node_POST_SUB:
    res = node_SUBTRACT;
    break;
  default:
    res = type;
    break;
  }
  return res;
}

list<quad> binary_operator(quad binary, arm_register *dest_reg, vector<arm_register> *regs, 
			   map<string, int> *fake_to_real) {
  int rr1 = fake_to_real -> find(binary.opd1) -> second;
  int rr2 = fake_to_real -> find(binary.opd2) -> second;
  if (binary.type != node_POST_ADD && binary.type != node_POST_SUB) {
    regs -> at(rr1).dt = NONE;
  }
  regs -> at(rr2).dt = NONE;
  if (binary.type != node_POST_ADD && binary.type != node_POST_SUB) {
    fake_to_real -> erase(binary.opd1);
  }
  fake_to_real -> erase(binary.opd2);
  string opd1 = regify(rr1);
  string opd2 = regify(rr2);

  list<quad> res;
  string dr = regify(dest_reg -> number);
  dest_reg -> dt = DATA;
  fake_to_real -> insert(pair<string, int>(binary.dest, dest_reg -> number));
  
  res.push_back(four_arity_quad(post_to_regular(binary.type), dr, opd1, opd2));
  return res;
}

list<quad> write_to_quads(quad write, vector<arm_register> *regs, set<string> idents, map<string, int> *fake_to_real) {
  list<quad> res;
  arm_register *r1 = &(regs -> at(1));
  //  if (r1 -> dt == NONE && r1 -> dt == NONE) {
    quad la = {node_LOAD, regify(1), write.dest};
    list<quad> load_asm = load(la, r1, idents, fake_to_real);
    res.insert(res.end(), load_asm.begin(), load_asm.end());
    res.push_back(three_arity_quad(node_MOV, regify(0), arm_small_constant("1")));
    res.push_back(two_arity_quad(node_SWI, SEEK));
    res.push_back(two_arity_quad(node_SWI, HALT));
  // } else {
  //   cout << "r1 or r0 is being used. Need to save their values\n";
  // }

  return res;
}

list<quad> quads_to_asm(list<quad> quads, set<string> idents, vector<arm_register> *regs) {
  list<quad> res;
  map<string, int> fake_to_real;
  list<quad> decs = declare_idents(idents);
  res.insert(res.end(), decs.begin(), decs.end());

  for (list<quad>::iterator it = quads.begin(); it != quads.end(); it++) {
    quad cq = *it;
    cout << "on this quad: " << quad_to_string(cq);
    switch(cq.type) {
        case node_STOR: {
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
    case node_LOGICAL_OR:
    case node_LOGICAL_AND:
    case node_POST_ADD:
    case node_POST_SUB:
    case node_BITWISE_XOR:
    case node_BITWISE_AND:
    case node_BITWISE_OR:
    case node_ADD:
    case node_MULT:
    case node_SUBTRACT: {
      int fr = regs_with_dt(regs, NONE).at(0);
      list<quad> bin_asm = binary_operator(cq, &(regs -> at(fr)), regs, &fake_to_real);
      res.insert(res.end(), bin_asm.begin(), bin_asm.end());
    }
      break;
    case node_DIVIDE: {
      list<quad> div = handle_divide(cq, regs, &fake_to_real);
      res.insert(res.end(), div.begin(), div.end());
    }
      break;
    case node_MOD: {
      list<quad> mod = handle_mod(cq, regs, &fake_to_real);
      res.insert(res.end(), mod.begin(), mod.end());
    }
      break;
    case node_WRITE: {
      list<quad> write_asm = write_to_quads(cq, regs, idents, &fake_to_real);
      res.insert(res.end(), write_asm.begin(), write_asm.end());
    }
      break;
    case node_NEGATE: {
      list<quad> negate = handle_negate(cq, regs, &fake_to_real);
      res.insert(res.end(), negate.begin(), negate.end());
    }
	break;
    case node_LESS_EQUAL: {
      list<quad> relation = handle_relational(cq, regs, &fake_to_real);
      res.insert(res.end(), relation.begin(), relation.end());
    }
      default:
  	cout << "dont have rule for nodetype: " << nodenames[cq.type] << "\n";
  	break;
    }
  }

  list<quad> af = arm_funcs();
  res.insert(res.end(), af.begin(), af.end());
  res.push_back(two_arity_quad(node_FUNC_LABEL, ".end"));
  return res;
}

void free_pair(pair<string, int> free, vector<arm_register> *regs, map<string, int> *fake_to_real) {
  if (pair_exists(free.first, fake_to_real)) {
    arm_register *f = &(regs -> at(free.second));
    f -> dt = NONE;
    fake_to_real -> erase(free.first);
  } else {
    cout << "WARN " << __FUNCTION__ << " free is not in map\n";
  }
}

pair<string, int> pair_exists(int real_reg, map<string, int> *fake_to_real) {
  pair<string, int> res;
  res.first = "not_found";
  res.second = -1;
  for (map<string, int>::iterator entry = fake_to_real -> begin(); entry != fake_to_real -> end(); entry++) {
    if (entry -> second == real_reg) {
      res.first = entry -> first;
      res.second = entry -> second;
      break;
    }
  }

  return res;
}

bool pair_exists(string fake_reg, map<string, int> *fake_to_real) {
  return fake_to_real -> find(fake_reg) != fake_to_real -> end();
}

void reg_pair(pair<string, int> reg,  data_type type, vector<arm_register> *regs, map<string, int> *fake_to_real) {
  if (pair_exists(reg.first, fake_to_real)) {
    cout << "WARN " << __FUNCTION__ << " reg is in map\n";
  } else {
    arm_register *t = &(regs -> at(reg.second));
    t -> dt = type;
    fake_to_real -> insert(reg);
  }
}

quad move_to_first_unused(pair<string, int> from, vector<arm_register> *regs, map<string, int> *fake_to_real) {
  int fr = regs_with_dt(regs, NONE).front();
  pair<string, int> to = pair<string, int>(from.first, fr);
  return move_to(from, to, regs, fake_to_real);
}

quad move_to(pair<string, int> from, pair<string, int> to, vector<arm_register> *regs, map<string, int> *fake_to_real) {
  free_pair(from, regs, fake_to_real);
  reg_pair(to, DATA, regs, fake_to_real);
  return three_arity_quad(node_MOV, regify(to.second), regify(from.second));
}

list<quad> handle_relational(quad rel, vector<arm_register> *regs, map<string, int> *fake_to_real) {
  list<quad> res;
  if (pair_exists(rel.opd1, fake_to_real) && pair_exists(rel.opd2, fake_to_real)) {
    int real_1 = fake_to_real -> find(rel.opd1) -> second;
    int real_2 = fake_to_real -> find(rel.opd2) -> second;
    res.push_back(three_arity_quad(node_CMP, regify(real_1), regify(real_2)));

    free_pair(pair<string, int>(rel.opd1, real_1), regs, fake_to_real);
    free_pair(pair<string, int>(rel.opd2, real_2), regs, fake_to_real);

    list<quad> dest_prep = prepare_dest(rel.dest, real_1, regs, fake_to_real);
    res.insert(res.end(), dest_prep.begin(), dest_prep.end());

    //do first move to assume false. next instruction will mov true if condition is true
    res.push_back(three_arity_quad(node_MOV, regify(real_1), arm_small_constant("0")));
    res.push_back(three_arity_quad(rel.type, regify(real_1), arm_small_constant("1")));
  } else {
    cout << "ERROR: " << __FUNCTION__ << " op(s) were not mapped properly\n";
  }

  return res;
}

list<quad> call_divide(quad div, int dest_reg, vector<arm_register> *regs, map<string, int> *fake_to_real) {
  list<quad> res;
  if (pair_exists(div.opd1, fake_to_real) && pair_exists(div.opd2, fake_to_real)) {
    list<quad> opd1_prep = prepare_operand(div.opd1, 0, regs, fake_to_real);
    res.insert(res.end(), opd1_prep.begin(), opd1_prep.end());

    list<quad> opd2_prep = prepare_operand(div.opd2, 1, regs, fake_to_real);
    res.insert(res.end(), opd2_prep.begin(), opd2_prep.end());

    free_pair(pair<string, int>(div.opd1, fake_to_real -> find(div.opd1) -> second), regs, fake_to_real);
    free_pair(pair<string, int>(div.opd2, fake_to_real -> find(div.opd2) -> second), regs, fake_to_real);

    list<quad> dest_prep = prepare_dest(div.dest, dest_reg, regs, fake_to_real);
    res.insert(res.end(), dest_prep.begin(), dest_prep.end());

    res.push_back(two_arity_quad(node_BL, "sdiv"));
  } else {
    cout << "ERROR: " << __FUNCTION__ << " op(s) were not mapped properly\n";
  }

  return res;
}

list<quad> prepare_operand(string fake_reg, int dest_real, vector<arm_register> *regs, map<string, int> *fake_to_real) {
  list<quad> res;
  if (pair_exists(fake_reg, fake_to_real)) {
    pair<string, int> func_opd = pair<string, int>(fake_reg, fake_to_real -> find(fake_reg) -> second);
    pair<string, int> curr_reg = pair_exists(dest_real, fake_to_real);
    if (curr_reg.second == -1) {
      //nothing is in desired register
      pair<string, int> to_pair = pair<string, int>(func_opd.first, dest_real);
      res.push_back(move_to(func_opd, to_pair, regs, fake_to_real));
    } else {
      //something is in desired register
      if (func_opd.second == curr_reg.second) {
	//do nothing becase function operand is in desired register
      } else {
	//move whatever is in desired register away and operand into desired register
	res.push_back(move_to_first_unused(curr_reg, regs, fake_to_real));
	pair<string, int> to_pair = pair<string, int>(func_opd.first, 0);
	res.push_back(move_to(func_opd, to_pair, regs, fake_to_real));
      }
    }
  } else {
    cout << __FUNCTION__ << " has no effect because fake_reg " << fake_reg << " has no mapping\n";
  }

  return res;
}

list<quad> prepare_dest(string fake_reg, int dest_real, vector<arm_register> *regs, map<string, int> *fake_to_real) {
  list<quad> res;
  pair<string, int> dest_pair = pair_exists(dest_real, fake_to_real);
  if (dest_pair.second == -1) {
    //do nothing because nothing is in destination
  } else {
    //move whatever is there somewhere else
    res.push_back(move_to_first_unused(dest_pair, regs, fake_to_real));
  }
  
  //take dest register
  reg_pair(pair<string, int>(fake_reg, dest_real), DATA, regs, fake_to_real); 
  return res;
}

list<quad> handle_mod(quad div, vector<arm_register> *regs, map<string, int> *fake_to_real) {
  return call_divide(div, 1, regs, fake_to_real);
}

list<quad> handle_divide(quad div, vector<arm_register> *regs, map<string, int> *fake_to_real) {
  return call_divide(div, 0, regs, fake_to_real);
}

list<quad> handle_negate(quad negate, vector<arm_register> *regs, map<string, int> *fake_to_real) {
  list<quad> res;
  if (fake_to_real -> find(negate.opd1) == fake_to_real -> end()) {
    cout << "ERROR: " << __FUNCTION__ << " opd1 was not in map\n";
  } else {
    list<quad> opd_prep = prepare_operand(negate.opd1, 0, regs, fake_to_real);
    res.insert(res.end(), opd_prep.begin(), opd_prep.end());

    free_pair(pair<string, int>(negate.opd1, fake_to_real -> find(negate.opd1) -> second), regs, fake_to_real);

    list<quad> dest_prep = prepare_dest(negate.dest, 1, regs, fake_to_real);
    res.insert(res.end(), dest_prep.begin(), dest_prep.end());

    res.push_back(two_arity_quad(node_BL, "negate"));
  }
  return res;
}

list<quad> arm_funcs() {
  list<quad> res;
  list<quad> afn = arm_func_negate(regify(0), regify(1));
  res.insert(res.end(), afn.begin(), afn.end());
  list<quad> sdiv = arm_func_sdiv();
  res.insert(res.end(), sdiv.begin(), sdiv.end());
  return res;
}

list<quad> arm_func_sdiv() {
  list<quad> res;
  res.push_back(two_arity_quad(node_FUNC_LABEL, ""));
  res.push_back(two_arity_quad(node_FUNC_LABEL, "sdiv:"));
  res.push_back(three_arity_quad(node_STM_FD, "sp!", "{r2-r5}"));
  res.push_back(three_arity_quad(node_MOV, regify(4), regify(0)));
  res.push_back(four_arity_quad(node_MULT, regify(5), regify(0), regify(1)));
  res.push_back(three_arity_quad(node_CMP, regify(0), arm_small_constant("0")));
  res.push_back(four_arity_quad(node_RSBLT, regify(0), regify(0), arm_small_constant("0")));
  res.push_back(three_arity_quad(node_CMP, regify(1), arm_small_constant("0")));
  res.push_back(four_arity_quad(node_RSBLT, regify(1), regify(1), arm_small_constant("0")));
  res.push_back(three_arity_quad(node_MOV, regify(3), regify(1)));
  res.push_back(four_arity_quad(node_CMP_4, regify(3), regify(0), "lsr #1"));

  res.push_back(two_arity_quad(node_FUNC_LABEL, ".Div1:"));
  res.push_back(four_arity_quad(node_MOVLS, regify(3), regify(3), "lsl #1"));
  res.push_back(four_arity_quad(node_CMP_4, regify(3), regify(0), "lsr #1"));
  res.push_back(two_arity_quad(node_BLS, ".Div1"));
  res.push_back(three_arity_quad(node_MOV, regify(2), arm_small_constant("0")));

  res.push_back(two_arity_quad(node_FUNC_LABEL, ".Div2:"));
  res.push_back(three_arity_quad(node_CMP, regify(0), regify(3)));
  res.push_back(four_arity_quad(node_SUBCS, regify(0), regify(0), regify(3)));
  res.push_back(four_arity_quad(node_ADC, regify(2), regify(2), regify(2)));
  res.push_back(four_arity_quad(node_MOV_4, regify(3), regify(3), "lsr #1"));
  res.push_back(three_arity_quad(node_CMP, regify(3), regify(1)));
  res.push_back(two_arity_quad(node_BHS, ".Div2"));
  res.push_back(three_arity_quad(node_CMP, regify(5), arm_small_constant("0")));
  res.push_back(four_arity_quad(node_RSBLT, regify(2), regify(2), arm_small_constant("0")));
  res.push_back(three_arity_quad(node_CMP, regify(4), arm_small_constant("0")));
  res.push_back(four_arity_quad(node_RSBLT, regify(0), regify(0), arm_small_constant("0")));
  res.push_back(three_arity_quad(node_MOV, regify(1), regify(0)));
  res.push_back(three_arity_quad(node_MOV, regify(0), regify(2)));
  res.push_back(three_arity_quad(node_LDM_FD, "sp!", "{r2-r5}"));
  res.push_back(two_arity_quad(node_BX, regify(14)));
  return res;
}

list<quad> arm_func_negate(string in_reg, string ret_reg) {
  list<quad> res;
  res.push_back(two_arity_quad(node_FUNC_LABEL, ""));
  res.push_back(two_arity_quad(node_FUNC_LABEL, "negate:"));
  res.push_back(three_arity_quad(node_CMP, in_reg, arm_small_constant("0")));
  res.push_back(three_arity_quad(node_MOV, ret_reg, arm_small_constant("0")));
  res.push_back(three_arity_quad(node_MOV_EQ, ret_reg, arm_small_constant("1")));
  res.push_back(two_arity_quad(node_BX, nodenames[node_LR]));
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
    case node_LABEL:
      res = three_arity_nc(q.dest, q.opd1, q.opd2);
      break;
    case node_LOGICAL_AND:
    case node_LOGICAL_OR:
    case node_BITWISE_AND:
    case node_BITWISE_XOR:
    case node_BITWISE_OR:
    case node_ADD:
    case node_MULT:
    case node_SUBTRACT:
      res = four_arity(nt_to_arm(q.type), q.dest, q.opd1, q.opd2);
      break;
    case node_ADC:
    case node_SUBCS:
    case node_MOVLS:
    case node_RSBLT:
    case node_CMP_4:
    case node_MOV_4:
      res = four_arity(nodenames[q.type], q.dest, q.opd1, q.opd2);
      break;
    case node_BHS:
    case node_BLS:
    case node_BL:
    case node_BX: 
    case node_SWI:
      res = two_arity(nodenames[q.type], q.dest);
      break;
    case node_LDM_FD:
    case node_STM_FD:
    case node_STOR: 
    case node_LOAD:
    case node_MOV_EQ:  
    case node_CMP:
    case node_MOV:
      res = three_arity(nodenames[q.type], q.dest, q.opd1);
      break;
    case node_FUNC_LABEL:
      res = two_arity(q.dest, "");
      break;
    case node_LESS_EQUAL:
      res = three_arity(rel_to_arm(q.type), q.dest, q.opd1);
      break;
    default:
      res = "default";
      break;
  }
  return res;
}

string rel_to_arm(nodetype t) {
  string res;
  switch(t) {
  case node_LESS_EQUAL:
    res = "MOVLE";
    break;
  default:
    break;
  }
  return res;
}

string nt_to_arm(nodetype t) {
  string res;
  switch(t) {
    case node_SUBTRACT:
      res = "SUB";
      break;
  case node_MULT:
     res = "MUL";
     break;
  case node_ADD:
    res = "ADD";
    break;
  case node_LOGICAL_OR:
  case node_BITWISE_OR:
    res = "ORR";
    break;
  case node_LOGICAL_AND:
  case node_BITWISE_AND:
    res = "AND";
    break;
  case node_BITWISE_XOR:
    res = "EOR";
    break;
    default:
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


