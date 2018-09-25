%{

/* ===========================================================================

Scanner for test grammar for CSCI4700

=========================================================================== */

#include        <iostream>
#include        <iomanip>
#include        <stdlib.h>
#include        <string.h>
#include        "parse.h"
#include        "y.tab.h"

using namespace std;

extern YYSTYPE  yylval;
extern short	lex_debug, line;
extern char	listing_line[];

int is_keyword( const char *string );
char debug_char(char is);
int debug_int(const char* str, int retVal);

// Define this so that it will call our input routine (my_input) instead
// of reading from stdin (cin) as a normal flex-generated scanner would.
int my_input( unsigned char *buf, int max_size );
#undef YY_INPUT
#define YY_INPUT(buf,result,max_size) result = my_input( (unsigned char *) buf, max_size );

%}

D			[0-9]
L			[a-zA-Z_]
H			[a-fA-F0-9]
E			[Ee][+-]?{D}+
FS			(f|F|l|L)
IS			(u|U|l|L)*

%%

"\/\/"		{
		    // These are comments.
                    while ( yyinput() != '\n' )
			;
		    if ( lex_debug )
		        cout << "Removed comment from input stream.\n";
                }

"\/\*"          {
                    // The old-school comments
                    while ( 1 )
                        if ( yyinput() == '*' )
                            if ( yyinput() == '/' )
                                break;
		    if ( lex_debug )
		        cout << "Removed old style comment from input stream.\n";
                }

{L}({L}|{D})*		{  return( is_keyword( yytext ) ); }
0[xX]{H}+{IS}?		{  return( debug_int(yytext, CONSTANT) ); }
0{D}+{IS}?		{  return( debug_int(yytext, CONSTANT) ); }
{D}+{IS}?		{  return( debug_int(yytext, CONSTANT) ); }
{D}+{E}{FS}?		{  return( debug_int(yytext, CONSTANT) ); }
L?\"(\\.|[^\\"])*\"	{  return( debug_int(yytext, STRING_LITERAL) ); } /* " <-- de-confuse vim */
"++"			{  return( debug_int("++", INC_OP) ); }
"--"			{  return( debug_int("--", DEC_OP) ); }
"&&"			{  return( debug_int("&&", AND_OP) ); }
"||"			{  return( debug_int("||", OR_OP) ); }
"<="			{  return( debug_int("<=", LE_OP) ); }
">="			{  return( debug_int(">=", GE_OP) ); }
"=="			{  return( debug_int("==", EQ_OP) ); }
"!="			{  return( debug_int("!=", NE_OP) ); }

";"			{  return( debug_char(';') ); }
("{"|"<%")		{  return( debug_char('{') ); }
("}"|"%>")		{  return( debug_char('}') ); }
","			{  return( debug_char(',') ); }
":"			{  return( debug_char(':') ); }
"="			{  return( debug_char('=') ); }
"("			{  return( debug_char('(') ); }
")"			{  return( debug_char(')') ); }
("["|"<:")		{  return( debug_char('[') ); }
("]"|":>")		{  return( debug_char(']') ); }
"."			{  return( debug_char('.') ); }
"&"			{  return( debug_char('&') ); }
"!"			{  return( debug_char('!') ); }
"~"			{  return( debug_char('~') ); }
"-"			{  return( debug_char('-') ); }
"+"			{  return( debug_char('+') ); }
"*"			{  return( debug_char('*') ); }
"/"			{  return( debug_char('/') ); }
"%"			{  return( debug_char('%') ); }
"<"			{  return( debug_char('<') ); }
">"			{  return( debug_char('>') ); }
"^"			{  return( debug_char('^') ); }
"|"			{  return( debug_char('|') ); }
"?"			{  return( debug_char('?') ); }

[ \t\v\n\f]		{  }
.			{ /* ignore bad characters */ }

%%

struct tab {
    char     word[ 9 ];
    int      value;
};

int debug_int(const char* str, int retVal) {
  if (lex_debug) {
    cout << "matched this token ";
    cout << str;
    cout << "\n";
  }
  return retVal;
}

char debug_char(char c) {
  if (lex_debug) {
    cout << "matched this token: ";
    cout << c;
    cout << "\n";
  }
    return c;
}

int is_keyword( const char *string )
{

    int             comp( const void *, const void * );
    struct tab      *ptr;
    static struct   tab     table[] = {
      { "else",	        ELSE },
      { "for",	        FOR },
      { "if",		IF },
      { "int",          INT },
      { "read",         READ },
      { "return",	RETURN },
      { "while",	WHILE },
      { "write",        WRITE }
    };
    int retVal;

    ptr = (struct tab *) bsearch( string, (char *) &table[ 0 ], sizeof( table ) / 
				  sizeof( table[ 0 ] ), sizeof( table[ 0 ] ), comp );

    retVal = ptr ? ptr -> value : IDENTIFIER;
    if (lex_debug) {
      cout << "matched this token ";
      cout << retVal;
      cout << "\n";
    }
    return retVal;
}

int comp( const void *a, const void *b )
{
    return( strcmp( ( ( struct tab *) a ) -> word, 
		    ( ( struct tab *) b ) -> word ) );
}
