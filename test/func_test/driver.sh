#!/usr/bin/env bash

if [[ ! $1 =~ .*toycc$ ]]; then
 	echo "Missing toycc compiler path"
	exit 1
fi

if [[ -z $2 ]]; then
	echo "Missing directory"
	exit 1
fi

if [[ ! $3 =~ .*\.c$ ]]; then
	echo "Missing input file"
	exit 1;
fi

function exit_if_failure {
	if [[ $? -ne 0 ]]; then
		echo "Compile failed"
		exit 1;
	fi
}

output_dir=${2}/bin
program=$output_dir/$2.out
mkdir -p $output_dir

$1 $3 -o ${output_dir}/cp.o --filetype=obj
exit_if_failure

gcc $2/main.c ${output_dir}/cp.o -o $program
exit_if_failure

$program
exit_if_failure

