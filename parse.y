%{

/* ===========================================================================

Parser template, supplied as a starting point for compiler
construction. Use and abuse as you wish.

Author:  Bill Mahoney
For:     CSCI 4700

=========================================================================== */

#include        <iostream>       // Just plain needed...
#include        <fstream>
#include        <iomanip>        // Needed for setw, hex, ...
#include        <fstream>        // Needed for ofstream type
#include        <stdio.h>        // Possibly needed
#include        <unistd.h>       // Possibly needed
#include        <stdlib.h>       // Possibly needed
#include        <ctype.h>        // Possibly needed
#include        <string.h>       // Possibly needed
#include        "symbolTable.h"
#include        "quad_gen.h"
#include        "arm_output.h"
#include        <stack>
#include        <map>

using namespace std;

extern int  yydebug; // Use if you want the Bison generated debugging

set<nodetype> set_expr_types();
void negate_operands(parsetree *root);
bool negate_present(parsetree *root);
parsetree* new_unary_expr();
void negate_operands(parsetree *root, set<nodetype> expr_types, set<nodetype> ground_types, set<nodetype> recur_gt);
int usage( void );
int yyerror( const char *msg );
int my_input( char *buf, int max_size );
int sequential_name(parsetree *root, set<nodetype> node_types, int seq);
void sequential_name(parsetree *root);
 void check_names(parsetree *root, set<nodetype> node_types);
void detab( char *line );
int yylex( void );
void dumpit( parsetree *root, int level );
void dotit( parsetree *root, int level, ofstream *out_file);
void take_out_nodes(parsetree *root);
void take_out_nodes(parsetree *root, set<nodetype> remove_nodes);
void postfix_to_assign(parsetree *root, set<nodetype> post_ops);
void postfix_to_assign(parsetree *root);
extern "C" int yywrap( void );
extern char *yytext; // In the scanner

char		listing_line[ 132 ];	// for listings.	
int		err_count;		// # of errors.		
short		listing;		// true gives listing	
short           echo;                   // true echos input     
short           lex_debug;              // true debugs scanner  
short           line = 0;       	// For listings		
ifstream        infile;

parsetree *root;
%}

/* ===========================================================================

Here's the grammar part. The %expect says to expect one conflict
(shift/reduce). In this case it is the dangling else problem and we
know about that one. Change the %token to %type as needed when
implementing the parse tree generation.

=========================================================================== */

//Terminals
%token <treeptr> INT READ WRITE IDENTIFIER CONSTANT STRING_LITERAL
%token <treeptr> INC_OP DEC_OP LE_OP GE_OP EQ_OP NE_OP AND_OP OR_OP
%token <treeptr> IF ELSE WHILE FOR RETURN

//Non-terminals
%type <treeptr> translation_unit external_declaration formal_list formal block decl psubs subs statement statement_list
%type <treeptr> decl_list i_list expression_stmt selection_stmt iteration_stmt return_stmt expression
%type <treeptr> primary_expression postfix_expression unary_expression unary_operator multiplicative_expression
%type <treeptr> additive_expression relational_expression equality_expression and_expression exclusive_or_expression
%type <treeptr> inclusive_or_expression logical_and_expression logical_or_expression assignment_expression
%type <treeptr> identifier_list
%expect 1
%locations

%%

translation_unit
         : external_declaration
         {
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_translation_unit;
	  $$ -> children[0] = $1;
	  root = $$;
  	 }
       | translation_unit external_declaration
	  {
	    $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	    $$ -> type = node_translation_unit;
	    $$ -> children[0] = $1;
	    $$ -> children[1] = $2;
	    root = $$;
	  }
	;

external_declaration
	: int_ident '(' formal_list ')' block
	{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_external_declaration;
	  $$ -> children[0] = $<treeptr>1;
	  $$ -> children[1] = $3;
	  $$ -> children[2] = $5;
	}
        | decl
	;

formal_list
        : formal_list ',' formal
	{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_formal_list;
	  $$ -> children[0] = $1;
	  $$ -> children[1] = $3;
	}
        | formal
        |
	 {
	   $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $$ -> type = node_formal_list;
	 } 
        ;

formal  
        : int_ident psubs
	{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_formal;
	  $$ -> children[0] = $<treeptr>1;
	  $$ -> children[1] = $2;
	}
        | int_ident
	 {
          $<treeptr>$ = $<treeptr>1;
         }
        ;

int_ident
         : INT IDENTIFIER
	  {
  	    $<treeptr>$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
  	    $<treeptr>$ -> type = node_int_ident;
	    $<treeptr>$ -> children[0] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	    $<treeptr>$ -> children[0] -> type = node_INT;
	    $<treeptr>$ -> children[1] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	    $<treeptr>$ -> children[1] -> type = node_IDENTIFIER;
	    $<treeptr>$ -> children[1] -> str_ptr = strdup( yytext );	  	    
	    $<treeptr>$ -> children[1] -> line = yylloc.first_line;
	  } 


psubs   // In parameters, [] is legal if first. In decl's it is not.
        : '[' ']' subs
	 {
	   $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $$ -> type = node_psubs;
	   $$ -> children[0] = $3;
	 }
        | '[' ']'
	  {
	   $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $$ -> type = node_psubs;
	  }
        | subs
        ;

block
	: '{' '}'
	 {
	   $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $$ -> type = node_block;
	 } 
	| '{' statement_list '}'
	 {
	   $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $$ -> type = node_block;
	   $$ -> children[0] = $2;
	 } 
	| '{' decl_list '}'
	 {
	   $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $$ -> type = node_block;
	   $$ -> children[0] = $2;
	 }
	| '{' decl_list statement_list '}'
	 {
	   $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $$ -> type = node_block;
	   $$ -> children[0] = $2;
	   $$ -> children[1] = $3;
	 }
	;

statement_list
	: statement
	| statement_list statement
	 {
	   $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $$ -> type = node_statement_list;
	   $$ -> children[0] = $1;
	   $$ -> children[1] = $2;
	 }
	;

decl_list
        : decl
        | decl_list decl
	 {
	   $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $$ -> type = node_decl_list;
	   $$ -> children[0] = $1;
	   $$ -> children[1] = $2;
	 }
        ;

decl    // Only type is integer here
         : int_ident i_list ';'
	 {
	   $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $$ -> type = node_decl;
	   $$ -> children[0] = $<treeptr>1;
	   $$ -> children[1] = $2;
	 }
        ;

i_list  // Could factor IDENTIFIER above if you like
        : subs ',' IDENTIFIER
	{
	  $<treeptr>$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $<treeptr>$ -> type = node_i_list;
	  $<treeptr>$ -> children[0] = $1;
	  $<treeptr>$ -> children[1] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $<treeptr>$ -> children[1] -> type = node_IDENTIFIER;
	  $<treeptr>$ -> children[1] -> str_ptr = strdup(yytext);
	  $<treeptr>$ -> children[1] -> line = yylloc.first_line;
	} i_list
	 {
	   $<treeptr>$ = $<treeptr>4;
	   $<treeptr>$ -> children[2] = $5;
	 }
        | subs
        | ',' IDENTIFIER
	 {
 	   $<treeptr>$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $<treeptr>$ -> type = node_i_list;
	   $<treeptr>$ -> children[0] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $<treeptr>$ -> children[0] -> type = node_IDENTIFIER;
	   $<treeptr>$ -> children[0] -> str_ptr = strdup(yytext);
	   $<treeptr>$ -> children[0] -> line = yylloc.first_line;
	 } i_list
	  {
	    $<treeptr>$ = $<treeptr>3;
	    $<treeptr>$ -> children[1] = $4;
	  }
        |
	 {
	   $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $$ -> type = node_i_list;
	 } 
        ;

subs
        : '[' CONSTANT
	 {
	   $<treeptr>$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $<treeptr>$ -> type = node_subs;
	   $<treeptr>$ -> children[0] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $<treeptr>$ -> children[0] -> type = node_CONSTANT;
	   $<treeptr>$ -> children[0] -> str_ptr = strdup(yytext);
	 } ']'
	 {
	   $<treeptr>$ = $<treeptr>3;
	 }
        | subs '[' CONSTANT
	 {
	   $<treeptr>$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $<treeptr>$ -> type = node_subs;
	   $<treeptr>$ -> children[0] = $1;
	   $<treeptr>$ -> children[1] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $<treeptr>$ -> children[1] -> type = node_CONSTANT;
	   $<treeptr>$ -> children[1] -> str_ptr = strdup(yytext);
	 } ']' 
	  {
	    $<treeptr>$ = $<treeptr>4;
	  }
        ;

statement
	: block
	| expression_stmt
	| selection_stmt
	| iteration_stmt
	| return_stmt
        | READ '(' IDENTIFIER
	 {
	   $<treeptr>$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $<treeptr>$ -> type = node_statement;
	   $<treeptr>$ -> children[0] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $<treeptr>$ -> children[0] -> type = node_READ;
           $<treeptr>$ -> children[1] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
           $<treeptr>$ -> children[1] -> type = node_IDENTIFIER;
	   $<treeptr>$ -> children[1] -> str_ptr = strdup(yytext);
	   $<treeptr>$ -> children[1] -> line = yylloc.first_line;
	 } ')' ';'
	  {
	    $<treeptr>$ = $<treeptr>4;
	  }
        | WRITE 
	  {
	    $<treeptr>$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	    $<treeptr>$ -> type = node_statement;
	    $<treeptr>$ -> children[0] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	    $<treeptr>$ -> children[0] -> type = node_WRITE;
	    $<treeptr>$ -> children[0] -> str_ptr = strdup(yytext);
	  } '(' primary_expression ')' ';'
	   {
	     $<treeptr>$ = $<treeptr>2;
	     $<treeptr>$ -> children[1] = $4;
	   }
	;

expression_stmt
	: ';'
	{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_expression_stmt;
	}
	| expression ';'
	 {
	   $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $$ -> type = node_expression_stmt;
	   $$ -> children[0] = $1;
	 }
	;

selection_stmt
	: IF '(' expression ')' statement 
	 {
	   $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $$ -> type = node_selection_stmt;
	   $$ -> children[0] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $$ -> children[0] -> type = node_IF;
	   $$ -> children[0] -> str_ptr = "if";
	   $$ -> children[1] = $3;
	   $$ -> children[2] = $5;
 	 }
	| IF '(' expression ')' statement ELSE statement
	 {
  	   $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $$ -> type = node_selection_stmt;
	   $$ -> children[0] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $$ -> children[0] -> type = node_IF;
	   $$ -> children[0] -> str_ptr = "if";
	   $$ -> children[1] = $3;
	   $$ -> children[2] = $5;
	   $$ -> children[3] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $$ -> children[3] -> type = node_ELSE;
	   $$ -> children[3] -> str_ptr = "else";
	   $$ -> children[3] -> children[0] = $7;
	 }
	;

iteration_stmt
	: WHILE '(' expression ')' statement
	{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_iteration_stmt;
	  $$ -> children[0] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> children[0] -> type = node_WHILE;
	  $$ -> children[0] -> str_ptr = "while";
	  $$ -> children[1] = $3;
	  $$ -> children[2] = $5;
	}
	| FOR '(' expression_stmt expression_stmt ')' statement
	 {
	   $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $$ -> type = node_iteration_stmt;
	   $$ -> children[0] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $$ -> children[0] -> type = node_FOR;
	   $$ -> children[0] -> str_ptr = "for";
	   $$ -> children[1] = $3;
	   $$ -> children[2] = $4;
	   $$ -> children[3] = $6;
	 } 
	| FOR '(' expression_stmt expression_stmt expression ')' statement
	 {
	   $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $$ -> type = node_iteration_stmt;
	   $$ -> children[0] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $$ -> children[0] -> type = node_FOR;
	   $$ -> children[0] -> str_ptr = "for";
	   $$ -> children[1] = $3;
	   $$ -> children[2] = $4;
	   $$ -> children[3] = $5;
	   $$ -> children[4] = $7;
	 } 
	;

return_stmt
	: RETURN ';'
	{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_return_stmt;
	  $$ -> children[0] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> children[0] -> type = node_RETURN;
	  $$ -> children[0] -> str_ptr = "return";
	}
	| RETURN expression ';'
	 {
	   $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $$ -> type= node_return_stmt;
	   $$ -> children[0] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $$ -> children[0] -> type = node_RETURN;
	   $$ -> children[0] -> str_ptr = "return";
	   $$ -> children[1] = $1;
	 }
	;

expression
	: assignment_expression
	| expression ',' assignment_expression
	 {
	   $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $$ -> type = node_expression;
	   $$ -> children[0] = $1;
	   $$ -> children[1] = $3;
	 }
	;

primary_expression
	: IDENTIFIER
	{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_primary_expression;
	  $$ -> children[0] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> children[0] -> type = node_IDENTIFIER;
	  $$ -> children[0] -> str_ptr = strdup(yytext);  
 	  $$ -> children[0] -> line = yylloc.first_line;
	}
	| CONSTANT
	{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_primary_expression;
	  $$ -> children[0] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> children[0] -> type = node_CONSTANT;
	  $$ -> children[0] -> str_ptr = strdup(yytext);  
	}
	| STRING_LITERAL
	{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_primary_expression;
	  $$ -> children[0] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> children[0] -> type = node_STRING_LITERAL;
	  $$ -> children[0] -> str_ptr = strdup(yytext);  
	}
	| '(' expression ')'
	{
  	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_primary_expression;
	  $$ -> children[0] = $2;
	}
	;

postfix_expression
	: primary_expression
	| postfix_expression '[' expression ']'
	{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_postfix_expression;
	  $$ -> children[0] = $1;
	  $$ -> children[0] = $3;
	}
	| postfix_expression INC_OP
	{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_postfix_expression;
	  $$ -> children[0] = $1;
	  $$ -> children[1] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> children[1] -> type = node_INC_OP;
	  $$ -> children[1] -> str_ptr = "++";
	}
	| postfix_expression DEC_OP
	{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_postfix_expression;
	  $$ -> children[0] = $1;
	  $$ -> children[1] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> children[1] -> type = node_DEC_OP;
	  $$ -> children[1] -> str_ptr = "--";	  
	}
	;

unary_expression
	: postfix_expression
	| INC_OP unary_expression
	{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_unary_expression;
	  $$ -> children[0] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );	  
	  $$ -> children[0] -> type = node_INC_OP;
	  $$ -> children[0] -> str_ptr = "++";
	  $$ -> children[1] = $2;
	}
	| DEC_OP unary_expression
	{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_unary_expression;
	  $$ -> children[0] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );	  
	  $$ -> children[0] -> type = node_DEC_OP;
	  $$ -> children[0] -> str_ptr = "--";
	  $$ -> children[1] = $2;
	}
	| unary_operator unary_expression
	{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_unary_expression;
	  $$ -> children[0] = $1;
	  $$ -> children[1] = $2;
	}
	;

unary_operator
	: '&'
	{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_UNARY_AND;
	  $$ -> str_ptr = "&";
	}
	| '*'
	{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
 	  $$ -> type = node_DEREF;
	  $$ -> str_ptr = "*";
	}
	| '+'
	{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_UNARY_PLUS;
	  $$ -> str_ptr = "+";
	}
	| '-'
	{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_UNARY_MINUS;
	  $$ -> str_ptr = "-";
	}
	| '!'
	{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_NEGATE;
	  $$ -> str_ptr = "!";
	}
	;

// Here we have the operator heirarchy in C++ but spelled out.
// Normally in a bison grammar you would list all of the operations
// in the same place, and use %left and %right and the bison precedence
// rules to keep the grammar file shorter. You certainly can if you want.
multiplicative_expression
	: unary_expression
	| multiplicative_expression '*' unary_expression
	{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_multiplicative_expression;
	  $$ -> children[0] = $1;
	  $$ -> children[1] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> children[1] -> type = node_MULT;
	  $$ -> children[1] -> str_ptr = "*";
	  $$ -> children[2] = $3;
	}
	| multiplicative_expression '/' unary_expression
	{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_multiplicative_expression;
	  $$ -> children[0] = $1;
	  $$ -> children[1] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> children[1] -> type = node_DIVIDE;
	  $$ -> children[1] -> str_ptr = "/";
	  $$ -> children[2] = $3;
	}
	| multiplicative_expression '%' unary_expression
	{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_multiplicative_expression;
	  $$ -> children[0] = $1;
	  $$ -> children[1] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> children[1] -> type = node_MOD;
	  $$ -> children[1] -> str_ptr = "%";
	  $$ -> children[2] = $3;
	}
	;

additive_expression
	: multiplicative_expression
	| additive_expression '+' multiplicative_expression
	{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_additive_expression;
	  $$ -> children[0] = $1;
	  $$ -> children[1] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> children[1] -> type = node_ADD;
	  $$ -> children[1] -> str_ptr = "+";
	  $$ -> children[2] = $3;
	}
	| additive_expression '-' multiplicative_expression
	{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_additive_expression;
	  $$ -> children[0] = $1;
	  $$ -> children[1] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> children[1] -> type = node_SUBTRACT;
	  $$ -> children[1] -> str_ptr = "-";
	  $$ -> children[2] = $3;
	}
	;

relational_expression
	: additive_expression
	| relational_expression '<' additive_expression
	{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_relational_expression;
	  $$ -> children[0] = $1;
	  $$ -> children[1] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> children[1] -> type = node_LESS_THAN;
	  $$ -> children[1] -> str_ptr = "<";
	  $$ -> children[2] = $3;
	}
	| relational_expression '>' additive_expression
	{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_relational_expression;
	  $$ -> children[0] = $1;
	  $$ -> children[1] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> children[1] -> type = node_GREATER_THAN;
	  $$ -> children[1] -> str_ptr = ">";
	  $$ -> children[2] = $3;
	}
	| relational_expression LE_OP additive_expression
	{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_relational_expression;
	  $$ -> children[0] = $1;
	  $$ -> children[1] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> children[1] -> type = node_LESS_EQUAL;
	  $$ -> children[1] -> str_ptr = "<=";
	  $$ -> children[2] = $3;
	}
	| relational_expression GE_OP additive_expression
	{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_relational_expression;
	  $$ -> children[0] = $1;
	  $$ -> children[1] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> children[1] -> type = node_GREATER_EQUAL;
	  $$ -> children[1] -> str_ptr = ">=";
	  $$ -> children[2] = $3;
	}
	;

equality_expression
	: relational_expression
	| equality_expression EQ_OP relational_expression
	{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_equality_expression;
	  $$ -> children[0] = $1;
	  $$ -> children[1] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> children[1] -> type = node_EQUAL;
	  $$ -> children[1] -> str_ptr = "==";
	  $$ -> children[2] = $3;
	}
	| equality_expression NE_OP relational_expression
	{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_equality_expression;
	  $$ -> children[0] = $1;
	  $$ -> children[1] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> children[1] -> type = node_NOT_EQUAL;
	  $$ -> children[1] -> str_ptr = "!=";
	  $$ -> children[2] = $3;
	}
	;

and_expression
	: equality_expression
	| and_expression '&' equality_expression
	{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_and_expression;
	  $$ -> children[0] = $1;
	  $$ -> children[1] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> children[1] -> type = node_BITWISE_AND;
	  $$ -> children[1] -> str_ptr = "&";
	  $$ -> children[2] = $3;
	}
	;

exclusive_or_expression
	: and_expression
	| exclusive_or_expression '^' and_expression
	{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_exclusive_or_expression;
	  $$ -> children[0] = $1;
	  $$ -> children[1] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> children[1] -> type = node_BITWISE_XOR;
	  $$ -> children[1] -> str_ptr = "^";
	  $$ -> children[2] = $3;
	}
	;

inclusive_or_expression
	: exclusive_or_expression
	| inclusive_or_expression '|' exclusive_or_expression
	{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_inclusive_or_expression;
	  $$ -> children[0] = $1;
	  $$ -> children[1] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> children[1] -> type = node_BITWISE_OR;
	  $$ -> children[1] -> str_ptr = "|";
	  $$ -> children[2] = $3;
	}
	;

logical_and_expression
	: inclusive_or_expression
	| logical_and_expression AND_OP inclusive_or_expression
	{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_logical_and_expression;
	  $$ -> children[0] = $1;
	  $$ -> children[1] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> children[1] -> type = node_LOGICAL_AND;
	  $$ -> children[1] -> str_ptr = "&&";
	  $$ -> children[2] = $3;
	}
	;

logical_or_expression
	: logical_and_expression
	| logical_or_expression OR_OP logical_and_expression
	{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_logical_or_expression;
	  $$ -> children[0] = $1;
	  $$ -> children[1] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> children[1] -> type = node_LOGICAL_OR;
	  $$ -> children[1] -> str_ptr = "||";
	  $$ -> children[2] = $3;
	}
	;

// Modified this and moved the function calls here from postfix_expression so that one can NOT
// call a function in the middle of an expression; this is harder for students to implement
// because of the live registers in the expression.
assignment_expression
	: logical_or_expression
	| postfix_expression '(' ')'
	{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_assignment_expression;
	  $$ -> children[0] = $1;
	}
	| postfix_expression '(' identifier_list ')'
	{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_assignment_expression;
	  $$ -> children[0] = $1;
	  $$ -> children[1] = $3;
	}
	| unary_expression '=' assignment_expression
	{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_assignment_expression;
	  $$ -> children[0] = $1;
	  $$ -> children[1] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> children[1] -> type = node_ASSIGNMENT;
	  $$ -> children[1] -> str_ptr = "=";
	  $$ -> children[2] = $3;	  
	}
	;

identifier_list
	: IDENTIFIER
	{
	   $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $$ -> type = node_identifier_list;
	   $$ -> children[0] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $$ -> children[0] -> type = node_IDENTIFIER;
	   $$ -> children[0] -> str_ptr = strdup(yytext);
	   $$ -> children[0] -> line = yylloc.first_line;
	 }
	| identifier_list ',' IDENTIFIER
	 {
	   $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $$ -> type = node_identifier_list;
	   $$ -> children[0] = $1;
	   $$ -> children[1] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $$ -> children[1] -> type = node_IDENTIFIER;
	   $$ -> children[1] -> str_ptr = strdup(yytext);
	   $$ -> children[1] -> line = yylloc.first_line;
	 }
	;

%%

/* ===========================================================================

MAIN

=========================================================================== */

int main( int ac, char *av[] )
{
    
    char *filename;
    int  i;

    if ( ac < 2 )
	exit( usage() );
    else
	for( i = 1; i < ac; i++ )
	    if ( av[ i ][ 0 ] == '-' )
		switch( av[ i ][ 1 ] )
		{
                    case 'e': // Currently does nothing in lex end of things
                        echo = ! echo;
                        break;
                    case 'l':
                        lex_debug = ! lex_debug;
                        break;
                    case 'y':  
                        yydebug = ! yydebug;
                        break;
                    default:
                        exit( usage() );
		}
	    else
		filename = av[ i ];
    
    infile.open( filename, ios::in );
    if ( ! infile )
    {
        cerr << "Can't open source file!\n";
	exit( 2 );
    }
    
    // We can test the return of yyparse, but I'll go ahead
    // and track an error count internally and use that.

    (void) yyparse();

    infile.close();

    if ( ! err_count )
    {
       //apply tree transformations
      take_out_nodes(root);
      negate_operands(root);
      sequential_name(root);

      ofstream tree_file("tree.dat");
      dotit(root, 0, &tree_file);

      vector<map<string, id_attrs> > sym_table;
      map<string, id_attrs> global_scope;
      sym_table.push_back(global_scope);
      cout << "calling into sym table\n";
      symbol_table(root, &sym_table);
      
      cout << "calling make quads\n";
      ofstream quad_file("output.quad");
      list<quad> quads;
      quads = make_quads(root, quads);
      quad_file << quad_list_to_str(quads);
      quad_file.flush();

      cout << "calling process quads, " << quads.size() << "\n"; 
      set<string> begin;
      set<string> ids = get_identifiers(root, begin);
      ofstream asm_file("program.s");
      list<quad> assembler = quads_to_asm(quads, ids);
      list<string> asm_out = asm_quads_to_asm(assembler);
      asm_file << list_to_string(asm_out);
      asm_file.flush();

      cout << "\n\tquad output can be seen in ./output.quad\n\tassembler output can be seen in ./program.s\n";
      return( 0 );
    }
    else
    {
	cerr << "Completed with " << err_count << "errors.\n";
	return( 1 );
    }
    
}

void sequential_name(parsetree *root) {
  cout << "in " << __FUNCTION__ << "\n";
  set<nodetype> types;
  types.insert(node_IF);
  types.insert(node_ELSE);
  types.insert(node_WHILE);

  sequential_name(root, types, 0);
  cout << __FUNCTION__ << " is done\n";
}

void check_names(parsetree *root, set<nodetype> node_types) {
  if (contains(node_types, root -> type)) {
    cout << "node " << nodenames[root -> type] << " now has str_ptr " << root -> str_ptr << "\n";
    if (root -> type == node_ELSE) {
      check_names(root -> children[0], node_types);
    }
  } else {
    for (int i = 0; i < 10 && root -> children[i] != NULL; i++) {
      check_names(root -> children[i], node_types);
    }
  }
}

int sequential_name(parsetree *root, set<nodetype> node_types, int seq) {
  if (contains(node_types, root -> type)) {
    string seq_end = string(root -> str_ptr) + "_" + to_string(seq);
    root -> str_ptr = (char*) calloc(256 * sizeof(char), 0);
    strcpy((char*) root -> str_ptr, seq_end.c_str());
    int ret = seq + 1;
    if (root -> type == node_ELSE) {
      ret = sequential_name(root -> children[0], node_types, ret);
    }
    return ret;
  } else {
    for (int i = 0; i < 10 && root -> children[i] != NULL; i++) {
      int ret = sequential_name(root -> children[i], node_types, seq);
      seq = ret;
    }
    return seq;
  }
}

bool negate_present(parsetree *root) {  
  return root -> type == node_unary_expression && root -> children[0] != NULL
    && root -> children[0] -> type == node_NEGATE;
}

set<nodetype> set_expr_types() {
  set<nodetype> exp_types;
  exp_types.insert(node_additive_expression);
  exp_types.insert(node_multiplicative_expression);
  exp_types.insert(node_relational_expression);
  exp_types.insert(node_inclusive_or_expression);
  exp_types.insert(node_and_expression);
  exp_types.insert(node_exclusive_or_expression);
  exp_types.insert(node_logical_and_expression);
  exp_types.insert(node_logical_or_expression);
  return exp_types;
}

parsetree* new_unary_expr() {
  parsetree *top_child = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
  top_child -> type = node_unary_expression;
  top_child -> children[0] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
  top_child -> children[0] -> type = node_NEGATE;
  top_child -> children[1] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
  top_child -> children[1] -> type = node_unary_expression;
  top_child -> children[1] -> children[0] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
  top_child -> children[1] -> children[0] -> type = node_NEGATE;
  top_child -> children[1] -> children[1] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
  return top_child;
}

void negate_operands(parsetree *root) {
  cout << "in " << __FUNCTION__ << "\n";
  set<nodetype> recur_gt = set_expr_types();

  set<nodetype> ground_types;
  ground_types.insert(node_IDENTIFIER);
  ground_types.insert(node_CONSTANT);
  ground_types.insert(node_unary_expression);
  ground_types.insert(node_postfix_expression);
  ground_types.insert(recur_gt.begin(), recur_gt.end());
  ground_types.erase(node_logical_and_expression);
  ground_types.erase(node_logical_or_expression);

  set<nodetype> expr_types;
  expr_types.insert(node_logical_and_expression);
  expr_types.insert(node_logical_or_expression);

  negate_operands(root, expr_types, ground_types, recur_gt);
  cout << __FUNCTION__ << " is done\n";
}

void negate_operands(parsetree *root, set<nodetype> expr_types, set<nodetype> ground_types, 
		     set<nodetype> recur_gt) {
  if (contains(expr_types, root -> type)) {
    for (int i = 0; i < 3 && root -> children[i] != NULL; i++) {
      if (contains(ground_types, root -> children[i] -> type)) {
	parsetree *new_child = new_unary_expr();
	new_child -> children[1] -> children[1] = root -> children[i];
	root -> children[i] = new_child;
	if (contains(recur_gt, root -> children[i] -> children[1] -> children[1] -> type)) {
	  negate_operands(root -> children[i] -> children[1] -> children[1], expr_types, ground_types, recur_gt);
	}
      } else if (contains(expr_types, root -> children[i] -> type)) {
	negate_operands(root -> children[i], expr_types, ground_types, recur_gt);
      }
    }
  } else {
    if (root -> type == node_selection_stmt && contains(ground_types, root -> children[1] -> type)) {
      parsetree *new_child = new_unary_expr();
      new_child -> children[1] -> children[1] = root -> children[1];
      root -> children[1] = new_child;    
    }
    for (int i = 0; i < 10 && root -> children[i] != NULL; i++) {
      negate_operands(root -> children[i], expr_types, ground_types, recur_gt);
    }
  }
}

void take_out_nodes(parsetree *root) {
  cout << "in " << __FUNCTION__ << "\n";
  set<nodetype> remove_nodes;
  remove_nodes.insert(node_primary_expression);
  remove_nodes.insert(node_expression_stmt);
  take_out_nodes(root, remove_nodes);
  cout << __FUNCTION__ << " is done\n";
}

void take_out_nodes(parsetree *root, set<nodetype> remove_nodes) {
  if (root == NULL) {
    return;
  }
  parsetree *child;
  int stop;
  for (int i = 0; i < 10 && root -> children[i] != NULL; i++) {
    child = root -> children[i];
    while (child != NULL && contains(remove_nodes, child -> type)) {
      child = child -> children[0];
    }
    if (child != NULL) {
      root -> children[i] = child;
    }
    take_out_nodes(child, remove_nodes);
  }
}

/*===========================================================================

usage

Print usage message, curl up, and die. Basically we just return 1
always, as we are usually called as "exit( usage() )". So there.

Inputs:  None
Outputs: None
Returns: 1 always

=========================================================================== */

int usage( void )
{
    cout << "usage: whatever <file> [-e] [-l] [-m size ] [-p] [-s] [-y]\n"
      //         << "-e == turns on echo of input file\n"
      //         << "-l == turns on lex debug flag\n"
         << "-y == turns on yydebug flag\n";
    return( 1 );
}

/* ===========================================================================

yyerror

This is called from within the parser when something is not matching a
grammar rule. It can also be called manually (see de_reference) to
generate an error for some other reason. 

Inputs:  None
Outputs: None
Returns: int?

=========================================================================== */

int yyerror( const char *msg )
{
    cerr << "Error: line " << line << ": " << listing_line << endl;
    cerr << msg << endl;
    err_count++;
    return( 0 );
}

/* ===========================================================================

yywrap

This function is called automatically when we tell the scanner that
the file is done. The purpose is to let the scanner know if there is
more input coming up (like from an additonal file) or not. In the case
of the assembler, we want to go through the file two times - once to
make the symbol table, once to do the dirty work. So the first time
we're called, rewind to the beginning of the file. Second time, tell
them that we're really done.

Inputs:  None
Outputs: None
Returns: 0 as an indication that there is more input (pass two for us)
         1 on a true end-of-file

=========================================================================== */

extern "C" {

int yywrap( void )
{
    return( 1 ); /* done! */
}

};

/* ===========================================================================

my_input

This function is dropped in in the place of the normal scanner input
function. The reason we do this is to allow us to count input lines,
generate listing output, and so on. To set this up, in the scanner
define YY_INPUT to call here instead if handling it internally. Then
whenever the scanner wants data we call here, read a line, return
it. At the end of file it is necessary to return a 0 to indicate "no
more".

Inputs:  buf - pointer to a place where the scanner wants the data
         max_size - the largest buffer that the scanner will accept
Outputs: buf - filled in with data from the input file (one byte at a
         time using this function, although the data is still buffered
	 internally to us, so it isn't too inefficient).
Returns: 0 on end-of-file
         N - number of bytes read into "buf" (always one for this
	 version)

=========================================================================== */

int my_input( unsigned char *buf, int max_size )
{

    if ( ! infile.eof() )
        infile.getline( listing_line, sizeof( listing_line ) );
    
    if ( infile.eof() )
    {
	listing_line[ 0 ] = '\0';
	*buf = '\0';
	return( 0 ); // A.k.a. YY_NULL 
    }
    else
    {
	char *s;
	// Getline tosses the newline, but we want it on there.
	// Various things depend on it (it is treated as a token).
	strcat( listing_line, "\n" );
	if ( listing )
        {
  	    detab( listing_line );
            cout << listing_line;
        }
	line++;
        // For some reason FLEX wants this as unsigned char, but
        // strcpy wants it as signed char...
        strcpy( (char *) buf, listing_line );
	return( strlen( listing_line ) );
    }
}


/* ===========================================================================

detab

Remove any tab characters from the input line and replace them with
spaces. Basically this is just here to make things line up regardless
of the tab settings on the terminal / printer / whatever. If it is not
needed, you can just nuke it.

Inputs:  line - the line to handle
Outputs: line - with tabs replaced by spaces
Returns: none

=========================================================================== */

void detab( char *line )
{
    
    static char	  temp[ BUFSIZ ];
    register char *s, *d;
    int		  col;

    col = 0; s = line; d = temp;
    while ( *s )
	if ( *s != '\t' )
	    *d++ = *s++, col++;
	else
	{
	    do	{
		*d++ = ' ';
		col++;
	    } while ( col % 8 );
	    s++;
	}
    *d = '\0';
    (void) strcpy( line, temp );
    
}

void dumpit( parsetree *root, int level )
{
    for( int i = 0; i < level; i++ )
        cout << "    ";

    cout << nodenames[ root -> type ];

    if ( root -> type == node_INT )
        cout << " keyword 'int'";
    else if ( root -> type == node_IDENTIFIER )
        cout << " identifier value is '" << root -> str_ptr << "'";
    // More else's as needed.

    if ( root -> children[ 0 ] )
    {
        cout << endl;
        for( int i = 0; root -> children[ i ]; i++ )
            dumpit( root -> children[ i ], level + 1 );
    }
    else
        cout << " - no children" << endl;
}

void dotit( parsetree *root, int level, ofstream *out_file)
{
    char name[ 32 ], cname[ 32 ];

    if ( level == 0 )
        *out_file << "digraph g {" << endl;

    sprintf( name, "%p", root );
    name[ 0 ] = 'x'; // It's a dot thing
    if ( root -> type == node_IDENTIFIER )
        *out_file << "    " << name << " [label=\"" << nodenames[ root -> type ] 
		 << "\\n" << root -> str_ptr << "\"];" << endl;
    else
        *out_file << "    " << name << " [label=\"" << nodenames[ root -> type ] << "\"];" << endl;
    for( int i = 0; root -> children[ i ]; i++ )
    {
        sprintf( cname, "%p", root -> children[ i ] );
        cname[ 0 ] = 'x'; // It's a dot thing
        *out_file << "    " << name << " -> " << cname << ";" << endl;
        dotit( root -> children[ i ], level + 1, out_file);
    }

    if ( level == 0 )
        *out_file << "}" << endl;

}
 
