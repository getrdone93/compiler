/* ===========================================================================

header file for sample Flex/Bison parser.

=========================================================================== */

#define	TRUE		1
#define	FALSE		0

#define SYM_LEN         32

#include <string>

using namespace std;

#ifndef NODE_NAMES_H
#define NODE_NAMES_H
#include "nodeNames.h"
#endif // NODE_NAMES_H

enum id_type {
  integer,
  integer_array,
  function
};

struct id_attrs {
  id_type it;
  int line;
  char* id_name;
  int value;
};

struct parsetree {
  enum nodetype type;
  int int_val;
  int line;
  const char *str_ptr;
  struct id_attrs *symbol_table_ptr;
  struct parsetree *children[ 10 ];
};

struct yystype {
  struct parsetree *treeptr;
};

#ifdef  YYSTYPE
#undef  YYSTYPE
#endif
#define YYSTYPE struct yystype
