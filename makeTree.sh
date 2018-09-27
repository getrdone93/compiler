#!/bin/sh

./parse $1 > tree.dat
dot -Tpng tree.dat >~/public_html/newtree.tree
chmod 666 ~/public_html/newtree.tree