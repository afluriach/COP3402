#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#define MAX_LINE_SIZE 256
#define MAX_IDENT_LEN 11
#define MAX_INT_LEN 5
#define N_RESERVED_WORDS 14
#define MAX_LIST_SIZE 1024
#define MAX_RESERVED_LEN 11
//the offset can be added to a reserved word's position in reservedWords to get the integer value of its TokeType enumeration. 
//the only exception is the odd symbol whose value is among the symbol values; see below:
#define RESERVED_OFFSET 21
#define N_SYMBOLS 17
//same idea for symbols
#define SYMBOL_OFFSET 4

typedef union
{
    int value;
    char name[MAX_IDENT_LEN+1];
} TokenData;

char reservedWords[N_RESERVED_WORDS][MAX_RESERVED_LEN] = {"begin", "end", "if", "then", "while","do", "call", "const", "int", "procedure", "out", "in", "else", "odd"};

//symbols[4] should never be detected: this is to reserve sybmols[4] (tokentype value 8) for the odd keyword
char symbols[N_SYMBOLS][3] = {"+", "-", "*", "/", "!!", "=", "<>", "<", "<=", ">", ">=", "(", ")", ",", ";" , ".", ":=" };


typedef enum { 
nulsym = 1, identsym, numbersym, plussym, minussym,
multsym,  slashsym, oddsym, eqsym, neqsym, lessym, leqsym,
gtrsym, geqsym, lparentsym, rparentsym, commasym, semicolonsym,
periodsym, becomessym, beginsym, endsym, ifsym, thensym, 
whilesym, dosym, callsym, constsym, intsym, procsym, outsym,
insym , elsesym } TokenType;

int getSymbolEnum(int pos)
{
    assert(pos >= 0), assert( pos < N_SYMBOLS);
    return pos+SYMBOL_OFFSET;
}

int getReservedEnum(int pos)
{
    //oddsym is the special case
    if(pos==13)
        return oddsym;
    else return pos+ RESERVED_OFFSET;
}


typedef struct
{
    TokenData data;
    TokenType type;
	short int len;
} Token;

typedef struct
{
    int size, len;
    Token * list;
} TokenList;



TokenList * initList(int size)
{
    TokenList * tl = malloc(sizeof(TokenList));
    assert (tl != NULL);
    tl->size = size;
    tl->list = malloc(sizeof(Token)*size);
    assert(tl->list != NULL);
    tl->len = 0;
    return tl;
}

void destroyList(TokenList * tl)
{
    free(tl->list);
    free(tl);
}

void addToken(TokenList * tl, Token t)
{
    assert(tl->len+1 < tl->size);
    tl->list[tl->len++] = t;
}

Token getToken(TokenList * tl, int pos)
{
    assert(pos >=0 && pos < tl->len);
    return tl->list[pos];
}

int isValidWhitespace(char ch)
{
    switch(ch)
	{
        case ' ':
        case '\n':
        case '\t':
		case '\r':
            return 1;
        default:
            return 0;
    }
}

//Determine if a is a prefix substring of b;
//i.e., the first strlen(a) characters of a are also the first strlen(a) characters of
//b. both must be nul-terminated. 

int isPrefix(char * a, char * b)
{
	int i,len;
	for(i=0, len=strlen(a); i < len; i++)
	{
		//if EOS is reached on b first, then a can't be a prefix substring of b
		if(b[i] == '\0')
			return 0;
		else if (a[i] != b[i])
			return 0;
		
	}
	return 1;
}

int isOpenComment(char * buf, int pos)
{
   return isPrefix("/*", buf+pos); 
}

int isCloseComment(char * buf, int pos)
{
    return isPrefix("*/", buf+pos);
}

//parse integer; if integer overflows the value stored will be incorrect. token len will need to be checked
//returns NULL if it is an illegal identifier.

Token parseInt(char * buf, int pos)
{
    int i, n=0;
    for(i=0;isdigit(buf[pos]); i++, pos++)
    {
        n *= 10;
        n += buf[pos] - '0'; 
    }
	
	//if a digit is immediately followed by an alphabetic character, then this is an illegal identifer.
	
	Token t;
	if(isalpha(buf[pos]))
		t.type = nulsym;
	else
		t.type = numbersym;
	t.data.value = n;
	t.len = i;
    return t; 
}

int isReservedWord(char * buf, int pos)
{
	int i;
	for(i=0;i<N_RESERVED_WORDS; i++)
		if(isPrefix(reservedWords[i], buf+pos))
			return 1;
	return 0;
}

int getIdentLen(char * buf, int pos)
{
	int i;
	for(i=0; isalpha(buf[pos]); i++, pos++)
		;
	return i;
}

//copy n bytes from source to dest, plus null terminatopr
void substrcpy(char * dest, char * src, int n)
{
	int i;
	for(i=0;i<n;i++)
		dest[i] = src[i];
	dest[i] = '\0';
}



Token parseSymbol(char * buf, int pos)
{
	//if there are multiple symbols that can match at the current position, 
	//only consider the longest one
	
	int i, maxSym;
	TokenType symbolType=nulsym;
	for(i=0, maxSym=0; i < N_SYMBOLS; i++)
	{
		if(isPrefix(symbols[i], buf+pos))
		{
			if(strlen(symbols[i]) > maxSym)
			{
				maxSym = strlen(symbols[i]);
				symbolType = getSymbolEnum(i);
			}
		}
	}
	Token t;
	t.type = symbolType;
	t.len = maxSym;
	return t;
}

Token parseIdentifier(char * buf, int pos)
{
	int len = getIdentLen(buf, pos);
	Token t;
	t.type = identsym;
	t.len = len;
	if(len <= MAX_IDENT_LEN)
		substrcpy(t.data.name, buf+pos, len);
	return t;
}

Token parseReservedWord(char * buf, int pos)
{
	Token t;
	t.type = nulsym;
	t.len = 0;
	int i;
	
	for(i=0;i<N_RESERVED_WORDS;i++)
		if(isPrefix(reservedWords[i], buf+pos))
		{
			t.type =  getReservedEnum(i);
			t.len = strlen(reservedWords[i]);
			break;
		}
	return t;
}

int alphaNumLen(char * a)
{
	int i=0;
	for(;isdigit(*a) || isalpha(*a); a++, i++)
		;
	return i;
}

void throwError(int errNum, int lineNum, char * str)
{
	printf("Error %d, line %d: ", errNum, lineNum);
	
	switch(errNum)
	{
		case 1:
			printf("indentifier \"%.*s\" does not start with an letter.\n", alphaNumLen(str), str);
			break;
		case 2:
			printf("integer \"%.*s\" is too large.\n",alphaNumLen(str), str);
			break;
		case 3:
			printf("identifier \"%.*s\" is too large.\n", alphaNumLen(str), str);
			break;
		case 4:
			printf("illegal symbol \"%.*s\".\n", 1, str);
			break;
		default:
			assert(0);
	}
	exit(0);
}

void printLexeme(FILE * fout, Token crnt)
{
		if(crnt.type == identsym)
			fprintf(fout, "%s", crnt.data.name);
		else if(crnt.type == numbersym)
			fprintf(fout, "%d", crnt.data.value);
		//print symbol
		else if(crnt.type >= SYMBOL_OFFSET && crnt.type < N_SYMBOLS+SYMBOL_OFFSET)
			fprintf(fout, "%s", symbols[crnt.type - SYMBOL_OFFSET]); 
		//print reserved word
		else
			fprintf(fout, "%s", reservedWords[crnt.type-RESERVED_OFFSET]);
}

void printList(FILE * fout, TokenList * tl)
{
	int i;
	Token t;
	for(i=0; i < tl->len; i++)
	{
		Token t = getToken(tl, i);
		fprintf(fout, "%d ", (int) t.type);
		if(t.type == identsym)
			fprintf(fout, "%s ", t.data.name);
		if(t.type == numbersym)
			fprintf(fout, "%d ", t.data.value);
	}
}

void printTable(FILE * fout, TokenList * tl)
{
	fprintf(fout, "lexeme\ttoken type\n");
	int i;
	Token t;
	
	for(i=0; i < tl->len; i++)
	{
		Token t = getToken(tl, i);
		printLexeme(fout, t);
		fprintf(fout, "\t%d", t.type);
		fprintf(fout, "\n");
		
	}
}


void printProgram(FILE * fout, TokenList * tl)
{
	int i;
	Token crnt;
	for(i=0;i<tl->len; i++)
	{
		crnt = getToken(tl, i);
		printLexeme(fout, crnt);
		if(crnt.type == beginsym || crnt.type==endsym || crnt.type==semicolonsym)
			fprintf(fout, "\n");
		else
			fprintf(fout, " ");
	}
	fprintf(fout, "\n");
}

int main(int argc, char** argv)
{
	if(argc != 3 && argc != 2)
	{
		printf("Input PL/0 file as first argument and output file as second argument. "
			   "If no second argument is specified program will output to stdout.\n");
		exit(0);
	}
	FILE *fin=fopen(argv[1], "r"), *fout= (argc==2) ? stdout : fopen(argv[2], "w");
	if(fin == NULL)
	{
		printf("Error opening input file \"%s\".\n", argv[1]);
		exit(0);
	}
	if(fout == NULL)
	{
		printf("Error opening output file \"%s\".\n", (argc==2) ? "stdout" : argv[2]);
		exit(0);
	}
	//fgets will read up to n-1 plus a null terminator. 
	char line[MAX_LINE_SIZE+1];
	TokenList * tl = initList(MAX_LIST_SIZE);
	
	int lineNumber, inComment=0;
	for(lineNumber=1; !feof(fin); lineNumber++)
	{
		if(fgets(line, MAX_LINE_SIZE+1, fin)==NULL)
			break;
		int lineSize  = strlen(line);
		int i;
		Token crnt;
		for(i=0; i < lineSize;)
		{
            if(inComment)
            {
                if(isCloseComment(line, i))
                {
                    i+= 2;
                    inComment = 0;
                    continue;
                }
                else
                {
                    i += 1;
                    continue;
                }
            }
            
            if(!inComment)
            {
                if(isOpenComment(line, i))
                {
                    i += 2;
                    inComment = 1;
                    continue;
                }
            }
            
			if(isValidWhitespace(line[i])){
				i++;
				continue;
			}
			if(isalpha(line[i]))
			{
				if(isReservedWord(line, i))
					crnt = parseReservedWord(line, i);
				else{
					crnt = parseIdentifier(line, i);
					if(crnt.len > MAX_IDENT_LEN)
						throwError(3, lineNumber, line+i);
				}
			}
			else if(isdigit(line[i])){
				crnt= parseInt(line, i);
				if(crnt.type==nulsym)
					throwError(1, lineNumber, line+1);
				if(crnt.len > MAX_INT_LEN)
					throwError(2, lineNumber, line+i);
			}
			else
			{
				crnt= parseSymbol(line, i);
				if(crnt.type == nulsym)
					throwError(4, lineNumber, line+i);
			}
			
			//add symbol to table and advance i as appropriate.
			
			addToken(tl, crnt);
			i += crnt.len;
		}
	}
	
	fprintf(fout, "Source program:\n");
	printProgram(fout, tl);
	fprintf(fout, "\n\nLexeme table:\n");
	printTable(fout, tl);
	fprintf(fout, "\n\nLexeme list:\n");
	printList(fout, tl);
	exit(0);
}
