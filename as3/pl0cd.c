#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COMMAND_LEN 255
#define INTERMEDIATE_FILE_LEN 8

int call(char * name, char * input, char * output)
{
	char command [MAX_COMMAND_LEN+1] = "";
	strncat(command, name, MAX_COMMAND_LEN);
	strncat(command, " ", MAX_COMMAND_LEN);
	strncat(command, input, MAX_COMMAND_LEN);
	strncat(command, " ", MAX_COMMAND_LEN);
	strncat(command, output, MAX_COMMAND_LEN);
	strncat(command, "\n", MAX_COMMAND_LEN);
	return system(command);
}

void randHex(int nDigits, char * dest)
{
	int i;
	srand(time(0));
	
	for(i=0;i<nDigits; i++)
	{
		int n = rand()%16;
		if(n < 10)
			dest[i] = '0' + n;
		else
			dest[i] = 'a' + n - 10;
	}
	dest[i] = '\0';
}

int main(int argc, char ** argv)
{
	/*
	if(argc != 3)
		printf("PL/0 compiler driver.\n
			   "pl0cd operation-level input-file
			   "operation level:\n"
			   "1. Tokenize only
	*/
	
	char tokens[INTERMEDIATE_FILE_LEN];
	char code[INTERMEDIATE_FILE_LEN];
	randHex(INTERMEDIATE_FILE_LEN, tokens);
	randHex(INTERMEDIATE_FILE_LEN, code);
	
	if(call("./lex", argv[1], tokens) != 0)
	{
		printf("Lexical scanner error (error tokenizing file).\n");
		call("rm", tokens,"");
		exit(1);
	}
	
	if(call("./codegen", tokens, code) != 0)
	{
		call("rm", tokens, "");
		printf("Error compiling code.\n");
		exit(1);
	}
	
	call("./vm", code, argc == 3 ? argv[2] : "");
	call("rm", tokens, "");
	call("rm", code, "");
}