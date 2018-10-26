#!/bin/sh

#compiler outputs tree everytime in dot form to tree.dat
dot -Tpng tree.dat >~/public_html/another_tree.tree
chmod 666 ~/public_html/another_tree.tree
