extern const char *nodenames[];
enum nodetype {
  node_translation_unit = 1,
  node_external_declaration,
  node_formal_list,
  node_formal,
  node_INT,
  node_IDENTIFIER,
  node_psubs,
  node_block,
  node_statement_list,
  node_decl_list,
  node_decl,
  node_i_list,
  node_subs,
  node_CONSTANT,
  node_statement,
  node_READ,
  node_WRITE,
  node_expression_stmt,
  node_selection_stmt,
  node_IF,
  node_ELSE,
  node_WHILE,
  node_FOR,
  node_iteration_stmt,
  node_RETURN,
  node_return_stmt,
  node_expression,
  node_primary_expression,
  node_STRING_LITERAL,
  node_INC_OP,
  node_DEC_OP,
  node_unary_expression,
  node_unary_operator,
  node_UNARY_AND,
  node_DEREF,
  node_UNARY_PLUS,
  node_UNARY_MINUS,
  node_NEGATE,
  node_MULT,
  node_multiplicative_expression,
  node_DIVIDE,
  node_MOD,
  node_additive_expression,
  node_ADD,
  node_SUBTRACT,
  node_relational_expression,
  node_LESS_THAN,
  node_GREATER_THAN,
  node_LESS_EQUAL,
  node_GREATER_EQUAL,
  node_equality_expression,
  node_EQUAL,
  node_NOT_EQUAL,
  node_and_expression,
  node_BITWISE_AND,
  node_BITWISE_XOR,
  node_BITWISE_OR,
  node_exclusive_or_expression,
  node_inclusive_or_expression,
  node_logical_and_expression,
  node_logical_or_expression,
  node_LOGICAL_AND,
  node_LOGICAL_OR,
  node_assignment_expression,
  node_ASSIGNMENT,
  node_identifier_list,
  node_postfix_expression,
  node_int_ident,
  node_STOR,
  node_ERROR,
  node_end_of_nodes
};
