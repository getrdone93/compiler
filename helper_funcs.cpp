#include "helper_funcs.h"

using namespace std;

string to_string(int num) {
  stringstream ss;
  ss << num;
  return ss.str();
}

string list_pairs_to_string(list<pair<string, string> > lis) {
   string res;
   for (list<pair<string, string> >::iterator it = lis.begin(); it != lis.end(); it++) {
	res += it -> second;
   }
   return res;
}

int to_int(string str) {
  int num;
  stringstream ss(str);
  ss >> num;
  return num;
}

void output_pair(pair<string, string> p, string var_name) {
  cout << var_name << ".first " << p.first << "\t" << var_name << ".second " << p.second << "\n";
}

void output_reg_sets(set<string> *regs_avail, set<pair<string, string> > *regs_used) {
  cout << "regs_used:\n";
    for (set<pair<string, string> >::iterator it = regs_used -> begin(); it != regs_used -> end(); it++) {
      output_pair(*it, "entry");
    }

    cout << "\nregs_avail:\n";
    for (set<string>::iterator it = regs_avail -> begin(); it != regs_avail -> end(); it++) {
      cout << *it << " ";
    }
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

string assoc_if_not_used(string id, set<string> *regs_avail, set<pair<string, string> > *regs_used) {
  pair<string, string> id_reg = lookup_str(id, regs_used);
  return id_reg.first.empty() ? assoc_id_reg(id, regs_avail, regs_used) : id_reg.first;
}

string assoc_id_reg(string id, set<string> *regs_avail, set<pair<string, string> > *regs_used) {
  string reg = first(regs_avail);
  return assoc_id_reg(id, reg, regs_avail, regs_used);
}

string assoc_id_reg(string id, string reg, set<string> *regs_avail, set<pair<string, string> > *regs_used) {
  if (reg.empty() || id.empty()) {
    //    error("assoc_id_reg", "reg or id was empty");
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


pair<string, string> release_reg(string id, set<string> *regs_avail, set<pair<string, string> > *regs_used) {
    pair<string, string> up = lookup_str(id, regs_used);
    if (!up.first.empty()) {
      regs_avail -> insert(up.first);
      regs_used -> erase(up);
    }
    return up;
}
