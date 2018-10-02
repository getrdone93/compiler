%{

/* ===========================================================================

Parser template, supplied as a starting point for compiler
construction. Use and abuse as you wish.

Author:  Bill Mahoney
For:     CSCI 4700

=========================================================================== */

#include        <iostream>       // Just plain needed...
#include        <iomanip>        // Needed for setw, hex, ...
#include        <fstream>        // Needed for ofstream type
#include        <stdio.h>        // Possibly needed
#include        <unistd.h>       // Possibly needed
#include        <stdlib.h>       // Possibly needed
#include        <ctype.h>        // Possibly needed
#include        <string.h>       // Possibly needed
#include        "parse.h"
#include        <stack>
#include        <map>

using namespace std;

extern int  yydebug; // Use if you want the Bison generated debugging

int usage( void );
int yyerror( const char *msg );
int my_input( char *buf, int max_size );
void detab( char *line );
int yylex( void );
void dumpit( parsetree *root, int level );
void dotit( parsetree *root, int level );
bool idArray(parsetree *node, int startChild);
void pushScope(stack<map<string, id_type> > *symTable);
void outputMap(map<string, id_type> m);
void process_formal_list(parsetree *root, stack<map<string, id_type> > *symTable);
void insertSymTable(string ident, id_type idt, stack<map<string, id_type> > *symTable);
void process_decl_list(parsetree *root, stack<map<string, id_type> > *symTable);
void symbolTable(parsetree *root, stack<map<string, id_type> > *symTable);
bool inScope(string id, stack<map<string, id_type> > *symTable);
bool inTopScope(string id, stack<map<string, id_type> > *symTable);
bool inGlobalScope(string id, stack<map<string, id_type> > *symTable);
string getIdTypeName(id_type idt);
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
	  /*{
	    $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	    $$ -> type = node_external_declaration;
	    $$ -> children[0] = $1;
	    }*/
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
	  /*{
	    $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	    $$ -> type = node_formal_list;
	    $$ -> children[0] = $1;
	    }*/
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
	  /*{
	   $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $$ -> type = node_psubs;
	   $$ -> children[0] = $1;
	   }*/
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
	  /*{
	   $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $$ -> type = node_statement_list;
	   $$ -> children[0] = $1;
	   }*/ 
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
	  /*{
	   $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $$ -> type = node_decl_list;
	   $$ -> children[0] = $1;
	   }*/
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
	} i_list
	 {
	   $<treeptr>$ = $<treeptr>4;
	   $<treeptr>$ -> children[2] = $5;
	 }
        | subs
	  /*{
	   $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $$ -> type = node_i_list;
	   $$ -> children[0] = $1;
	   }*/
        | ',' IDENTIFIER
	 {
 	   $<treeptr>$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $<treeptr>$ -> type = node_i_list;
	   $<treeptr>$ -> children[0] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $<treeptr>$ -> children[0] -> type = node_IDENTIFIER;
	   $<treeptr>$ -> children[0] -> str_ptr = strdup(yytext);
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
	  /*{
	   $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $$ -> type = node_statement;
	   $$ -> children[0] = $1;
	   }*/
	| expression_stmt
	  /*{
	   $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $$ -> type = node_statement;
	   $$ -> children[0] = $1;
	   }*/
	| selection_stmt
	  /*{
	   $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $$ -> type = node_statement;
	   $$ -> children[0] = $1;
	   }*/
	| iteration_stmt
	  /*{
	   $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $$ -> type = node_statement;
	   $$ -> children[0] = $1;
	   }*/
	| return_stmt
	  /*{
	    $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	    $$ -> type = node_statement;
	    $$ -> children[0] = $1;
	    }*/
        | READ '(' IDENTIFIER
	 {
	   $<treeptr>$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $<treeptr>$ -> type = node_statement;
	   $<treeptr>$ -> children[0] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $<treeptr>$ -> children[0] -> type = node_READ;
           $<treeptr>$ -> children[1] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
           $<treeptr>$ -> children[1] -> type = node_IDENTIFIER;
	   $<treeptr>$ -> children[1] -> str_ptr = strdup(yytext);
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
	   $$ -> children[2] = $7;
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
	  /*{
	   $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $$ -> type = node_expression;
	   $$ -> children[0] = $1;
	   }*/
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
	  /*{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_postfix_expression;
	  $$ -> children[0] = $1;
	  }*/
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
	  /*{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_unary_expression;
	  $$ -> children[0] = $1;
	  }*/
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
	  $$ -> type = node_unary_operator;
	  $$ -> children[0] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> children[0] -> type = node_UNARY_AND;
	  $$ -> children[0] -> str_ptr = "&";
	}
	| '*'
	{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_unary_operator;
	  $$ -> children[0] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> children[0] -> type = node_DEREF;
	  $$ -> children[0] -> str_ptr = "*";
	}
	| '+'
	{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_unary_operator;
	  $$ -> children[0] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> children[0] -> type = node_UNARY_PLUS;
	  $$ -> children[0] -> str_ptr = "+";
	}
	| '-'
	{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_unary_operator;
	  $$ -> children[0] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> children[0] -> type = node_UNARY_MINUS;
	  $$ -> children[0] -> str_ptr = "-";
	}
	| '!'
	{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_unary_operator;
	  $$ -> children[0] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> children[0] -> type = node_NEGATE;
	  $$ -> children[0] -> str_ptr = "!";
	}
	;

// Here we have the operator heirarchy in C++ but spelled out.
// Normally in a bison grammar you would list all of the operations
// in the same place, and use %left and %right and the bison precedence
// rules to keep the grammar file shorter. You certainly can if you want.
multiplicative_expression
	: unary_expression
	  /*{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_multiplicative_expression;
	  $$ -> children[0] = $1;
	  }*/
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
	  /*{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_additive_expression;
	  $$ -> children[0] = $1;
	  }*/
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
	  /*{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_relational_expression;
	  $$ -> children[0] = $1;
	  }*/
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
	  /*{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_equality_expression;
	  $$ -> children[0] = $1;
	  }*/
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
	  /*{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_and_expression;
	  $$ -> children[0] = $1;	  
	  }*/
	| and_expression '&' equality_expression
	{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_and_expression;
	  $$ -> children[0] = $1;
	  $$ -> children[1] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> children[1] -> type = node_BITWISE_OR;
	  $$ -> children[1] -> str_ptr = "&";
	  $$ -> children[2] = $3;
	}
	;

exclusive_or_expression
	: and_expression
	  /*{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_exclusive_or_expression;
	  $$ -> children[0] = $1;
	  }*/
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
	  /*{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_inclusive_or_expression;
	  $$ -> children[0] = $1;
	  }*/
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
	  /*{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_logical_and_expression;
	  $$ -> children[0] = $1;	  
	  }*/
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
	  /*{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_logical_or_expression;
	  $$ -> children[0] = $1;
	  }*/
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
	  /*{
	  $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	  $$ -> type = node_assignment_expression;
	  $$ -> children[0] = $1;
	  }*/
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
	 }
	| identifier_list ',' IDENTIFIER
	 {
	   $$ = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $$ -> type = node_identifier_list;
	   $$ -> children[0] = $1;
	   $$ -> children[1] = (struct parsetree *) calloc( sizeof( struct parsetree ), 1 );
	   $$ -> children[1] -> type = node_IDENTIFIER;
	   $$ -> children[1] -> str_ptr = strdup(yytext);
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
        //cout << "Compiled OK\n";
      //dotit(root, 0);

        stack<map<string, id_type> > symTable;
	map<string, id_type> global_scope;
	symTable.push(global_scope);
	symbolTable(root, &symTable);
        return( 0 );
    }
    else
    {
	cerr << "Completed with " << err_count << "errors.\n";
	return( 1 );
    }
    
}

void pushScope(stack<map<string, id_type> > *symTable) {
  map<string, id_type> scope;
  symTable -> push(scope);
  cout << "New scope level entered, size of stack is " << symTable -> size() << "\n";
}

void symbolTable(parsetree *root, stack<map<string, id_type> > *symTable) {
  if (root == NULL) {
    cout << "null root, returning\n";
    return;
  }
  switch(root -> type) {
    case node_translation_unit:
      for (int i = 0; i < 10 && root -> children[i] != NULL; i++) {
	if (root -> children[i] -> type == node_decl) {
	  process_decl_list(root -> children[i], symTable);
	} else {
	  symbolTable(root -> children[i], symTable);
	}
      }
      break;
    case node_external_declaration:
        pushScope(symTable);
	insertSymTable(root -> children[0] -> children[1] -> str_ptr, function, symTable);
	process_formal_list(root -> children[1], symTable);

	//run block statements
	if (root -> children[2] -> children[0] != NULL 
	    && (root -> children[2] -> children[0] -> type == node_decl
		|| root -> children[2] -> children[0] -> type == node_decl_list)) {
	  process_decl_list(root -> children[2] -> children[0], symTable); 
	} else {
	  symbolTable(root -> children[2] -> children[0], symTable);
	}
	symbolTable(root -> children[2] -> children[1], symTable);
      break;
    case node_block: 
        pushScope(symTable);
	if (root -> children[0] -> type == node_decl_list || root -> children[0] -> type == node_decl) {
	  process_decl_list(root -> children[0], symTable);
	}
	if (root -> children[1] != NULL) {
	  symbolTable(root -> children[1], symTable);
	}

	outputMap(symTable -> top());
	symTable -> pop();
      break;
    case node_IDENTIFIER:
      if (inScope(root -> str_ptr, symTable)) {
	cout << "I see symbol " << root -> str_ptr << " and its in scope\n";
      } else {
	cout << "ERROR: symbol " << root -> str_ptr << " is out of scope\n";
      }
      break;
    default: 
      for (int i = 0; i < 10 && root -> children[i] != NULL; i++) {
	symbolTable(root -> children[i], symTable);
      }
      break;
  }
}

bool inScope(string id, stack<map<string, id_type> > *symTable) {
  return inTopScope(id, symTable) || inGlobalScope(id, symTable);
}

bool inGlobalScope(string id, stack<map<string, id_type> > *symTable) {
  stack<map<string, id_type> > copySymTable = *symTable;
  map<string, id_type> global_scope = copySymTable.top();
  while(copySymTable.size() > 1) {
    copySymTable.pop();
    global_scope = copySymTable.top();
  }
  return global_scope.find(id) != global_scope.end();
}

bool inTopScope(string id, stack<map<string, id_type> > *symTable) {
  return symTable -> top().find(id) != symTable -> top().end();
}

void outputMap(map<string, id_type> m) {
  for (map<string, id_type>::const_iterator it = m.begin(); it != m.end(); it++) {
    cout << "id: " << it->first << " type: " << getIdTypeName(it->second) << " is no longer in scope\n";
  }
}

void process_formal_list(parsetree *root, stack<map<string, id_type> > *symTable) {
  switch(root -> type) {
    case node_formal_list:
      for (int i = 0; i < 10 && root -> children[i] != NULL; i++) {
	process_formal_list(root -> children[i], symTable);
      }
      break;
    case node_int_ident:
      insertSymTable(root -> children[1] -> str_ptr, integer, symTable);
      break;
    case node_formal:
      insertSymTable(root -> children[0] -> children[1] -> str_ptr, integer_array, symTable);
      break;
    default:
      cout << "Error process_formal_list, arrived at node " << nodenames[root -> type] << "\n";
      break;    
  }
}

void process_decl_list(parsetree *root, stack<map<string, id_type> > *symTable) {
  if (root == NULL) {
    cout << "null root, returning\n";
    return;
  }
  switch(root -> type) {
    case node_decl_list:
      for (int i = 0; i < 10 && root -> children[i] != NULL; i++) {
	 process_decl_list(root -> children[i], symTable);
      }     
      break;
    case node_decl:
      if (idArray(root, 1)) {
	insertSymTable(root -> children[0] -> children[1] -> str_ptr, integer_array, symTable);
      } else {
	insertSymTable(root -> children[0] -> children[1] -> str_ptr, integer, symTable);
      }

      if (root -> children[1] -> type == node_i_list) {
	process_decl_list(root -> children[1], symTable);
      }
      break;
   case node_i_list:
     if (root -> children[0] == NULL) {
       return;
     }
     if (root -> children[0] -> type == node_subs) {
	if (idArray(root, 2)) {
	  insertSymTable(root -> children[1] -> str_ptr, integer_array, symTable);
	} else {
	  insertSymTable(root -> children[1] -> str_ptr, integer, symTable);
	}

	if (root -> children[2] -> type == node_i_list) {
	  process_decl_list(root -> children[2], symTable);
	}
      } else {
	if (idArray(root, 1)) {
	  insertSymTable(root -> children[0] -> str_ptr, integer_array, symTable);
	} else {
	  insertSymTable(root -> children[0] -> str_ptr, integer, symTable);
	}

	if (root -> children[1] -> type == node_i_list) {
	  process_decl_list(root -> children[1], symTable);
	}
      }
      break;
    default:
      cout << "Error process_decl_list, arrived at node " << nodenames[root -> type] << "\n";
      break;
  }
}

bool idArray(parsetree *node, int startChild) {
  const bool grandChildExists = node -> children[startChild] -> children[0] != NULL;
  return grandChildExists && node -> children[startChild] -> children[0] -> type == node_subs
    || grandChildExists && node -> children[startChild] -> type == node_subs;
}

void insertSymTable(string ident, id_type idt, stack<map<string, id_type> > *symTable) {
  map<string, id_type> tempMap = symTable -> top();
  tempMap.insert(pair<string, id_type>(ident, idt));
  symTable -> pop();
  symTable -> push(tempMap);
  cout << "inserted identifier " << ident << " with type " << getIdTypeName(idt) << " into symbol table\n";  
}

string getIdTypeName(id_type idt) {
  switch (idt) {
    case integer:
      return "integer";
  case integer_array:
    return "integer_array";
  case function:
    return "function";
  default:
    return "";
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

void dotit( parsetree *root, int level )
{
    char name[ 32 ], cname[ 32 ];

    if ( level == 0 )
        cout << "digraph g {" << endl;

    sprintf( name, "%p", root );
    name[ 0 ] = 'x'; // It's a dot thing
    if ( root -> type == node_IDENTIFIER )
        cout << "    " << name << " [label=\"" << nodenames[ root -> type ] << "\\n" << root -> str_ptr << "\"];" << endl;
    else
        cout << "    " << name << " [label=\"" << nodenames[ root -> type ] << "\"];" << endl;
    for( int i = 0; root -> children[ i ]; i++ )
    {
        sprintf( cname, "%p", root -> children[ i ] );
        cname[ 0 ] = 'x'; // It's a dot thing
        cout << "    " << name << " -> " << cname << ";" << endl;
        dotit( root -> children[ i ], level + 1 );
    }

    if ( level == 0 )
        cout << "}" << endl;

}
 
