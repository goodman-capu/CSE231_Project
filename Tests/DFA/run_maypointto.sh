#!/bin/bash

# path to clang++, llvm-dis, and opt
LLVM_BIN=/LLVM_ROOT/build/bin
# path to CSE231.so
LLVM_SO=/LLVM_ROOT/build/lib
# path to the test directory
TEST_DIR=/tests/DFA

$LLVM_BIN/clang -c -O0 $TEST_DIR/DFA_MayPointTo.cpp -emit-llvm -S -o /tmp/DFA_MayPointTo.ll
make -C $LLVM_SO/Transforms/CSE231_Project

for test_case in \
"/tmp/DFA_MayPointTo.ll" \
"/LLVM_ROOT/llvm/test/DebugInfo/Generic/block-asan.ll"
do
	echo "$test_case"
	$LLVM_BIN/opt -load $LLVM_SO/CSE231.so -cse231-maypointto < $test_case > /dev/null 2> /tmp/DFA.result1
	/solution/opt -cse231-maypointto < $test_case > /dev/null 2> /tmp/DFA.result2
	my_output=`cat /tmp/DFA.result1`
	opt_output=`cat /tmp/DFA.result2`

	if [ "$my_output" == "$opt_output" ]
	then
		echo "Answers match!"
	else
		echo "My output:"
		echo "$my_output"
		echo "OPT output:"
		echo "$opt_output"
		echo "Answers don't match!"
	fi
done
