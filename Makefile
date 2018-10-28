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

parse :	parse.o scanner.o symbolTable.o nodeNames.o quad_gen.o quad.o helper_funcs.o arm_output.o
	$(CC) $(LOPT) -o parse parse.o scanner.o symbolTable.o nodeNames.o quad_gen.o quad.o helper_funcs.o arm_output.o
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

quad_gen.o:	quad.cpp quad.h quad_gen.cpp quad_gen.h
	$(CC) $(COPT) -c quad_gen.cpp

arm_output.o:	arm_output.h arm_output.cpp quad.h
	$(CC) $(COPT) -c arm_output.cpp

helper_funcs.o:	helper_funcs.cpp helper_funcs.h
	$(CC) $(COPT) -c helper_funcs.cpp

nodeNames.o: nodeNames.cpp nodeNames.h
	$(CC) $(COPT) -c nodeNames.cpp

quad.o: quad.cpp quad.h nodeNames.h
	$(CC) $(COPT) -c quad.cpp

# Run some tests - all should parse OK
test :	parse
	echo "\na_test1.c"
	-./parse testFiles/input_progs/a_test1.c- > /dev/null
	cat testFiles/input_progs/a_test1.c-
	cat output.quad
	./makeTree.sh

	# echo "\na_test2.c"
	# -./parse testFiles/input_progs/a_test2.c- > /dev/null
	# 	cat testFiles/input_progs/a_test2.c-
	# 	./makeTree.sh
	# cat output.quad

	# echo "\na_test3.c"
	# -./parse testFiles/input_progs/a_test3.c- > /dev/null
	# 	cat testFiles/input_progs/a_test3.c-
	# ./makeTree.sh
	# cat output.quad

	# echo "\na_test4.c"
	# -./parse testFiles/input_progs/a_test4.c- > /dev/null
	# 	cat testFiles/input_progs/a_test4.c-
	# ./makeTree.sh
	# cat output.quad

	# echo "\na_test5.c"
	# -./parse testFiles/input_progs/a_test5.c- > /dev/null
	# 	cat testFiles/input_progs/a_test5.c-
	# ./makeTree.sh
	# cat output.quad

	# echo "\na_test6.c"
	# -./parse testFiles/input_progs/a_test6.c- > /dev/null
	# cat testFiles/input_progs/a_test6.c-
	# ./makeTree.sh
	# cat output.quad

	# echo "\na_test7.c"
	# -./parse testFiles/input_progs/a_test7.c- > /dev/null
	# 	cat testFiles/input_progs/a_test7.c-
	# ./makeTree.sh
	# cat output.quad

	# echo "\na_test8.c"
	# -./parse testFiles/input_progs/a_test8.c- > /dev/null
	# 	cat testFiles/input_progs/a_test8.c-
	# ./makeTree.sh
	# cat output.quad

	# echo "\na_test9.c"
	# -./parse testFiles/input_progs/a_test9.c- > /dev/null
	# 	cat testFiles/input_progs/a_test9.c-
	# cat output.quad

	# echo "\na_test10.c"
	# -./parse testFiles/input_progs/a_test10.c-
	# 	cat testFiles/input_progs/a_test10.c-
	# cat output.quad
	# ./makeTree.sh




# Mostly clean (leave executable)
mostly :
	-rm parse.cpp scanner.cpp *.o *~ \#*\# y.tab.h ram.image

clean :	
	-rm parse.cpp scanner.cpp *.o parse *~ \#*\# y.tab.h ram.image
