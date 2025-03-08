#!/usr/bin/env bash

source ../func.sh

check_toycc $1

program=bin/a.out
mkdir -p bin

$1 test.c -o bin/cp.ll --filetype=asm -emit-llvm
$1 test.c -o bin/cp.s --filetype=asm
$1 test.c -o bin/cp.o --filetype=obj
exit_if_failure "toycc compile failed"

gcc -O0 -g main.c bin/cp.o -o $program
exit_if_failure "gcc compile failed"

$program
exit_if_failure "$program exit"

