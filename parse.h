/* ===========================================================================

header file for sample Flex/Bison parser.

=========================================================================== */

#define	TRUE		1
#define	FALSE		0

#define SYM_LEN         32

#include "nodeNames.h"

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
