#!/usr/bin/perl
use strict;
use warnings;

my $quadFile = "output.quad";
my $programFile = "a_test";
my $suffix = ".c-";
my $numFiles = 11;
my $progDir = "input_progs";
my $outDir = "output";

foreach (my $i = 1; $i < $numFiles; $i++) {
    my $inputProg = $programFile . $i . $suffix;
    my $command = "./parse testFiles/$progDir/$inputProg";
    my $out = `$command`;
    #print $out;
    my $outputFile = "testFiles/" . $outDir . "/" . $inputProg . "_" . "quad";
    `cp $quadFile $outputFile`;
    `echo >> $outputFile`;
    `cat testFiles/$progDir/$inputProg >> $outputFile`;
    $out = `cat $outputFile`;
    print "-" x 40 . $inputProg . "-" x 40 . "\n";
    print $out;
}

