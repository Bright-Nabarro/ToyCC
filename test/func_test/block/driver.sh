#!/usr/bin/env bash

source ../func.sh

check_toycc $1

program=bin/a.out
mkdir -p bin

function test_if_success {
	if [[ $? -eq 0 ]]; then
		echo $1
		exit 1
	fi
}

for n in {1..2}; do
	$1 fail/${n}.c --filetype=obj
	test_if_success "file/${n}: Scope cannot be blocked"
done


$1 test.c -o bin/cp.o --filetype=obj
exit_if_failure "toycc compile failed"

gcc main.c bin/cp.o -o $program
exit_if_failure "gcc compile failed"

$program
exit_if_failure "$program exit"

