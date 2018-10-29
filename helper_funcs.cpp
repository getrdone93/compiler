#include "helper_funcs.h"

using namespace std;

string list_to_string(list<string> str_list) {
  string res;
  for (list<string>::iterator it = str_list.begin(); it != str_list.end(); it++) {
    res += *it;
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
