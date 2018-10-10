#include <stack>
#include <map>
using namespace std;

bool idArray(parsetree *node, int startChild);
void pushScope(stack<map<string, id_type> > *symTable);
void outputMap(map<string, id_type> m);
void process_formal_list(parsetree *root, stack<map<string, id_type> > *symTable);
void insertSymTable(string ident, id_type idt, stack<map<string, id_type> > *symTable, int line);
void process_decl_list(parsetree *root, stack<map<string, id_type> > *symTable);
void symbolTable(parsetree *root, stack<map<string, id_type> > *symTable);
bool inScope(string id, stack<map<string, id_type> > *symTable);
bool inTopScope(string id, stack<map<string, id_type> > *symTable);
bool inGlobalScope(string id, stack<map<string, id_type> > *symTable);
string getIdTypeName(id_type idt);
