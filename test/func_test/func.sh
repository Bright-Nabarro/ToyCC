function check_toycc {
	if [[ ! $1 =~ .*toycc$ ]]; then
	 	echo "Missing toycc compiler path"
		exit 1
	fi
}

function exit_if_failure {
	if [[ $? -ne 0 ]]; then
		echo $1
		exit 1;
	fi
}


