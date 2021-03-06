#!/bin/sh

if [ $# -ne 1 ]
then
echo input PL/0 program as argument.
exit
fi

rm -f tokens
rm -f binary
./lex $1 tokens && cat tokens && ./codegen tokens binary && 
echo "No errors, program is syntactically correct" && ./vm binary
rm -f tokens
rm -f binary
