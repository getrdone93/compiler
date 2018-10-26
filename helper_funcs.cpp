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

