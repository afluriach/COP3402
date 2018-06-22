#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define MAX_IDENT_LEN 11
#define MAX_CODE_LEN 1000
#define TOKEN_LIST_LEN 1000
#define TABLE_SIZE 100

#define LIT 1
#define OPR 2
#define LOD 3
#define STO 4
#define CAL 5
#define INC 6
#define JMP 7
#define JPC 8
#define SIO 9

#define RET 0
#define NEG 1
#define ADD 2
#define SUB 3
#define MUL 4
#define DIV 5
#define ODD 6
#define MOD 7
#define EQL 8
#define NEQ 9
#define LSS 10
#define LEQ 11
#define GTR 12
#define GEQ 13

typedef enum { 
nulsym = 1, identsym, numbersym, plussym, minussym,
multsym,  slashsym, oddsym, eqsym, neqsym, lessym, leqsym,
gtrsym, geqsym, lparentsym, rparentsym, commasym, semicolonsym,
periodsym, becomessym, beginsym, endsym, ifsym, thensym, 
whilesym, dosym, callsym, constsym, intsym, procsym, outsym,
insym , elsesym } TokenType;

typedef struct
{
    char name[MAX_IDENT_LEN+1];
	int value;
    TokenType type;
} Token;

typedef struct
{
    int size, len, pos;
    Token * list;
} TokenList;

typedef enum
{
    nulsymbol=0, variable, constant, procedure
} SymbolType;


typedef struct 
{
	SymbolType type;
	char name[MAX_IDENT_LEN+1];
	//value can be either the value of a constant, a code address for procedures, or a stack frame address for variables.  
	int value;
}Symbol;

typedef struct 
{
	Symbol * arr;
	int size, len;
} SymbolTable;

typedef struct 
{
	int op, l, m;
}Instruction;

typedef struct
{
	int l, m;
} Address;

void expression();
void block();
void varDeclaration();
void constDeclaration();
void statement();
Token crntToken();

void condition();

TokenList * mainList;
SymbolTable * st;
Instruction * code;
int cx;
int stackOffset;

SymbolTable * initTable(int size)
{
	SymbolTable * st = malloc(sizeof(SymbolTable));
	assert(st != NULL);
	st->size = size;
	st->arr = malloc(sizeof(Symbol)*size);
	assert(st->arr != NULL);
	st->len = 0;
}



void addSymbol(	int type, char * name, int value)
{
	st->arr[st->len].value = value;
	st->arr[st->len].type = type;
	strcpy(st->arr[st->len].name, name);
	st->len++;
}

void error(int errorNumber, char * errorString)
{
	printf("Error %d: %s\n", errorNumber, errorString);
	printf("Found token %d\n at token number %d.\n", crntToken().type, mainList->pos);
	exit(1);
}

Symbol getSymbol(char * name)
{
	int i;
	for(i=0;i<st->len; i++)
	{
		if(strcmp(name, st->arr[i].name)==0)
		{
			return st->arr[i];
		}
	}
	Symbol s;
	s.type = nulsymbol;
	return s;
}


TokenList * initList(int size)
{
    TokenList * tl = malloc(sizeof(TokenList));
    assert (tl != NULL);
    tl->size = size;
    tl->list = malloc(sizeof(Token)*size);
    assert(tl->list != NULL);
    tl->len = 0;
	tl->pos = 0;
    return tl;
}

void resizeList(TokenList * tl)
{
	tl-> size *= 2;
	tl->list = realloc(tl->list, tl->size);
	assert(tl->list != NULL);
}

Token crntToken()
{
	return mainList->list[mainList->pos];
}

Token nextToken()
{
	assert(mainList->pos+1 < mainList->len);
	return mainList->list[++mainList->pos];
}

void emit(int op, int l, int m)
{
	 assert(cx < MAX_CODE_LEN);
	 assert(cx >= 0);
	 code[cx].op = op;
	 code[cx].l = l;
	 code[cx].m = m;
	 ++cx;
}

void exportCode(FILE * fout, Instruction * arr)
{
	int i;
	for(i=0;i<cx; i++)
	{
		fprintf(fout, "%d %d %d ", arr[i].op, arr[i].l, arr[i].m);
	}
}

void factor()
{
       
	switch(crntToken().type)
	{
		case identsym:
            ; //minGW bug
            Token t = crntToken();
             Symbol s = getSymbol(t.name);
            if(s.type == nulsymbol)
                error(11, "undeclared identifier.");
            else if(s.type == procedure)
                error(21, "Expression must not contain procedure identifier.");
			if(s.type == variable)
				emit(LOD, 0, s.value);
			else if(s.type == constant)
				emit(LIT, 0, s.value);
			nextToken();
            break;
		case numbersym:
            emit(LIT, 0, crntToken().value);
			nextToken();
			break;
		case lparentsym: 
			nextToken();
			expression();
			if(crntToken().type != rparentsym)
				error(22, "right parenthesis missing.");
			nextToken();
			break;
		default:
            error(26, "expected expression or open parenthesis.");
	}
}

void term()
{
	int multop;
	factor();
	while(crntToken().type == multsym || crntToken().type == slashsym)
	{
		multop = crntToken().type;
		nextToken();
		factor();
		if(multop == multsym)
			emit(OPR, 0, MUL);
		else
			emit(OPR, 0, DIV);
	}
	
}

void expression()
{
	TokenType addop;
	
	if(crntToken().type == plussym || crntToken().type == minussym)
	{
		addop = crntToken().type;
		nextToken();
		term();
		if(addop == minussym){
			(OPR, 0, NEG);
		}
	}
	else
		term();
	while(crntToken().type == plussym || crntToken().type == minussym)
	{
		addop = crntToken().type;
		nextToken();
		term();
		if(addop == plussym)
			emit(OPR, 0, ADD); 
		else
			emit(OPR, 0, SUB);
	}
}

void program()
{
	
	block();
	if(crntToken().type != periodsym)
		error(9, "period expected at end of program.");
	else
		emit(OPR, 0, RET);
}

void block()
{
    //allocate space for control information. Even without procedures
    //the VM still expects valid control information. 
	int ctemp = cx;
	emit(INC, 0, 0);
    stackOffset = 3;
	constDeclaration();
	varDeclaration();
	//it will need to parse declaractions before it can know how many spaces
	//to allocate
	code[ctemp].m = stackOffset;
	statement();
}

void constDeclaration()
{
	if(crntToken().type != constsym)
		return;
	nextToken();
	while(crntToken().type == identsym)
	{
		char symbolName[MAX_IDENT_LEN+1];
        Token t=crntToken();
		strcpy(symbolName, t.name);
		nextToken();
		if(crntToken().type == becomessym)
	        error(1, "use = instead of :=.");
		else if(crntToken().type != eqsym)
			error(3, "identifier must be followed by =.");
		if(nextToken().type != numbersym)
			error(2, "= must be followed by a number.");
		else
		      addSymbol(constant, symbolName, crntToken().value);
		nextToken();
		if(crntToken().type != commasym)
		    break;
		else
		    nextToken();
	}
	if(crntToken().type != semicolonsym)
	   error(5, "semicolon or comma missing.");
	else
		nextToken();
}

void varDeclaration()
{
	if(crntToken().type != intsym)
		return;
	nextToken();
	while(crntToken().type == identsym)
	{
		Token t = crntToken();
		addSymbol(variable, t.name,  stackOffset++);
		nextToken();
		if(crntToken().type != commasym)
			break;
		else
			nextToken();
	}
	if(crntToken().type != semicolonsym)
		 error(5, "semicolon or comma missing.");
	else
		nextToken();
}

void statement()
{
    int counterTemp;
	TokenType crnt = crntToken().type;
	
	switch(crnt)
	{
		case identsym:
            ;
            Token t = crntToken();
			Symbol s = getSymbol(t.name);
			if(s.type == nulsymbol)
			     error(11, "undeclared identifier.");
			else if(s.type != variable)
				error(27, "attemp to use const or procedure as l-value.");
			if(nextToken().type != becomessym)
				error(28, "error, := expected after variable.");
			nextToken();
			expression();
			emit(STO, 0, s.value);
			
			break;
		case beginsym:
			nextToken();
			statement();
			while(crntToken().type == semicolonsym)
			{
				nextToken();
				statement();
			}
			if(crntToken().type != endsym)
				error(17, "semicolon or end expected.");
			else{
				nextToken();
				break;
			}
		case ifsym:
			nextToken();
			condition();
			if(crntToken().type != thensym)
				error(16, "then expected.");
			else
				nextToken();
			counterTemp = cx;
			emit(JPC, 0, 0);
			statement();
			code[counterTemp].m = cx;
			break;
		case whilesym:
			nextToken();
			int condAdd = cx;
			condition();
			counterTemp = cx;
			emit(JPC, 0, 0);
			if(crntToken().type != dosym)
				error(18, "do expected.");
			else
				nextToken();
			statement();
			emit(JMP, 0, condAdd);
			code[counterTemp].m = cx;
		default:
			return;
	}
}

void condition()
{
	if(crntToken().type == oddsym)
	{
		nextToken();
		expression();
		emit(OPR, 0, ODD);
	}
	else
	{
		expression();
		TokenType relOp = crntToken().type;
		nextToken();
		expression();
		switch(relOp)
		{
			case eqsym:

				emit(OPR, 0, EQL);
				break;
			case neqsym:
				emit(OPR, 0, NEQ);
				break;
			case lessym:
				emit(OPR, 0, LSS);
				break;
			case leqsym:
				emit(OPR, 0, LEQ);
				break;
			case gtrsym:
				emit(OPR, 0, GTR);
				break;
			case geqsym:
				emit(OPR, 0, GEQ);
				break;
			default:
				error(20, "relational operator expected.");
		}
	}
}

int main(int argc, char ** argv)
{
	if(argc != 2 && argc != 3)
	{
		printf("Usage: codegen source-file [dest-file]\n"
			   "Source file should be a tokenized PL/0 program. If dest file is not included, "
			   "the PM/0 integer \"binary\" will be printed to stdout.\n");
		exit(1);
	}
	
	FILE *fin, *fout;
	
	fin = fopen(argv[1], "r"), fout = (argc ==3) ? fopen(argv[2], "w") : stdout;
	if(fin == NULL)
	{
		printf("Error opening input file: \"%s\".\n", argv[1]);
		exit(1);
	}
	if(fout == NULL)
	{
		printf("Error opening output file: \"%s\".\n", argv[2]);
		exit(1);
	}
	
	mainList = initList(TOKEN_LIST_LEN);
	assert(mainList != NULL);
	cx=0;
	int i;
	code = malloc(sizeof(Instruction)*MAX_CODE_LEN);
	assert(code != 0);

	
	for(i=0; !feof(fin); i++, mainList->len++)
	{
		
		if(i == mainList->size)
			resizeList(mainList);
			
		int temp;
		fscanf(fin, "%d", &temp);
		mainList->list[i].type = temp;
		
		if(temp == identsym)
		{
			char name[MAX_IDENT_LEN];
			fscanf(fin, "%s", name);
			strcpy(mainList->list[i].name, name);
		}
		else if(temp == numbersym)
		{
			int n;
			fscanf(fin, "%d", &n);
			mainList->list[i].value = n;
		}
	}
	
	st = initTable(TABLE_SIZE);
	
	program();
	exportCode(fout, code);
	exit(0);
}
