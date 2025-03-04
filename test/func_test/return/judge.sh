#!/usr/bin/env bash

if [[ ! $1 =~ .*toycc$ ]]; then
 	echo "Missing toycc compiler path"
	exit 1
fi

if [[ ! $2 =~ .*\.c$ ]]; then
	echo "Missing input file"
	exit 1;
fi

function exit_if_failure {
	if [[ $? -ne 0 ]]; then
		echo "Compile failed"
		exit 1;
	fi
}

mkdir -p bin

$1 $2 -o bin/cp.o --filetype=obj
exit_if_failure

gcc main.c bin/cp.o -o bin/a.out
exit_if_failure

./a.out
exit_if_failure

