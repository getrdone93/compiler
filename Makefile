############################################################
#
# Makefile for C++ version of 4700
#
# Author: Bill Mahoney
# For:    CSCI 4700
#
############################################################

YACC=bison
LEX=flex
CC=g++
LN=g++
COPT=-g
LOPT=-g

parse :	parse.o scanner.o symbolTable.o nodeNames.o
	$(CC) $(LOPT) -o parse parse.o scanner.o symbolTable.o nodeNames.o
	rm parse.o scanner.o

# Bison has this annoying habit of sending yydebug to stderr
# so I change that to go to stdout, which I like better.
# Bison puts output in the non-traditional place; move it.
parse.o :	parse.y symbolTable.h
	$(YACC) -d -t parse.y
	sed "s/stderr/stdout/" parse.tab.c >parse.cpp ; rm parse.tab.c
	mv parse.tab.h y.tab.h
	$(CC) $(COPT) -c parse.cpp

scanner.o :	scanner.lex y.tab.h parse.h
	$(LEX) scanner.lex
	mv lex.yy.c scanner.cpp
	$(CC) $(COPT) -c scanner.cpp

symbolTable.o:	symbolTable.cpp symbolTable.h parse.h
	$(CC) $(COPT) -c symbolTable.cpp

#arm_output.o:	arm_output.cpp symbolTable.h
#	$(CC) $(COPT) -c arm_output.cpp

nodeNames.o: nodeNames.cpp nodeNames.h
	$(CC) $(COPT) -c nodeNames.cpp

# Run some tests - all should parse OK
test :	parse
	-./parse ../Tests/phase1test.c-
	-./parse ../Tests/phase2test1.c-
	-./parse ../Tests/phase3test.c-
	-./parse ../Tests/phase4test1.c-
	-./parse ../Tests/phase4test2.c-
	-./parse ../Tests/phase4test3.c-

# Mostly clean (leave executable)
mostly :
	-rm parse.cpp scanner.cpp *.o *~ \#*\# y.tab.h ram.image

clean :	
	-rm parse.cpp scanner.cpp *.o parse *~ \#*\# y.tab.h ram.image
