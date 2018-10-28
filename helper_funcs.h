#include <string>
#include <sstream>
#include <iostream>
#include <list>
#include <set>

using namespace std;

string four_arity(string op, string dest, string opd1, string opd2);
string three_arity(string op, string dest, string opd1);
string two_arity(string op, string dest);
string three_arity_nc(string op, string dest, string opd1);
string list_to_string(list<string> str_list);
pair<string, string> release_reg(string id, set<string> *regs_avail, set<pair<string, string> > *regs_used);
string assoc_if_not_used(string id, set<string> *regs_avail, set<pair<string, string> > *regs_used);
pair<string, string> lookup_str(string str, set<pair<string, string> > *regs_used);
string to_string(int num);
string list_pairs_to_string(list<pair<string, string> > lis);
int to_int(string str);
void output_pair(pair<string, string> p, string var_name);
void output_reg_sets(set<string> *regs_avail, set<pair<string, string> > *regs_used);
string assoc_id_reg(string id, set<string> *regs_avail, set<pair<string, string> > *regs_used);
string assoc_id_reg(string id, string reg, set<string> *regs_avail, set<pair<string, string> > *regs_used);
string first(set<string> *regs);
