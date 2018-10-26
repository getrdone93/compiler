#include <string>
#include <sstream>
#include <iostream>
#include <list>
#include <set>

using namespace std;

string to_string(int num);
string list_pairs_to_string(list<pair<string, string> > lis);
int to_int(string str);
void output_pair(pair<string, string> p, string var_name);
void output_reg_sets(set<string> *regs_avail, set<pair<string, string> > *regs_used);
