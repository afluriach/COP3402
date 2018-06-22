#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define MAX_IDENT_LEN 11
#define MAX_CODE_LEN 1000
#define MAX_LEX_LEVELS 10
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

#define INPUT 0 
#define OUTPUT 1 

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
	//the lexical level will be stored here before a copy is returned as a symbol.
	int lex;
}Symbol;

typedef struct 
{
	Symbol * arr;
	int size, len;
} SymbolTable;

//each new lexical scope will get its own SymbolTable. if a variable is found in arr[crntLex], then it is in
//lex level 0.

typedef struct
{
	SymbolTable * arr[MAX_LEX_LEVELS];
	int crntLex;
}SymbolTableList;


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
int varDeclaration();
void constDeclaration();
void statement();
Token crntToken();
SymbolTable * initTable(int size);
void condition();
Symbol getSymbolFromSt(char * name, SymbolTable * st);
void destroyTable(SymbolTable * st);

TokenList * mainList;
SymbolTableList * stl;
Instruction * code;
int cx;
int stackOffset;

void addSymbolTable(SymbolTableList * stl)
{
	assert(stl->crntLex + 1 < MAX_LEX_LEVELS);
	stl->crntLex++;
	stl->arr[stl->crntLex] = initTable(TABLE_SIZE);
}

void removeSymbolTable(SymbolTableList * stl)
{
	assert(stl->crntLex >= 0);
	destroyTable(stl->arr[stl->crntLex]);
	stl->crntLex --;
}

SymbolTable * initTable(int size)
{
	SymbolTable * st = malloc(sizeof(SymbolTable));
	assert(st != NULL);
	st->size = size;
	st->arr = malloc(sizeof(Symbol)*size);
	assert(st->arr != NULL);
	st->len = 0;
}

void destroyTable(SymbolTable * st)
{
    free(st->arr);
    free(st);
}



void addSymbol(	int type, char * name, int value)
{
	SymbolTable * st = stl->arr[stl->crntLex];
	st->arr[st->len].value = value;
	st->arr[st->len].type = type;
	strcpy(st->arr[st->len].name, name);
	//printf("Added symbol %s at pos %d, lexical level %d.\n", name, st->len, stl->crntLex);
	st->len++;
}

void error(int errorNumber, char * errorString)
{
    Token t = crntToken();
	printf("Error %d: %s\n", errorNumber, errorString);
	printf("Found token %d\n at token pos %d.%s%s\n", crntToken().type, mainList->pos, (crntToken().type == identsym) ? " it is an identifier: " : "",  (t.type == identsym) ? t.name : "");
	exit(1);
}

Symbol getSymbol(char * name)
{
    Symbol result;
	int lex = stl->crntLex;
	for(; lex >= 0; lex--)
	{
		result = getSymbolFromSt(name, stl->arr[lex]);
		if(result.type != nulsymbol)
		{
            //printf("Found symbol %s at lexical level %d.\n", name, lex);
			result.lex = stl->crntLex - lex;
			return result;
		}
	}
	return result;
}

Symbol getSymbolFromSt(char * name, SymbolTable * st)
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
				emit(LOD, s.lex, s.value);
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
	emit(JMP, 0, 0);
	block(1);
	if(crntToken().type != periodsym)
		error(9, "period expected at end of program.");
}

void block(int isMain)
{
    //allocate space for control information. Even without procedures
    //the VM still expects valid control information. 
	
    constDeclaration();
	int offset = varDeclaration();
	procDeclaration();	
	
	if(isMain)
        code[0].m = cx;
    emit(INC, 0, offset);
	statement();
	emit(OPR, 0, RET);
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

//change to do while loops
//const/var followed by nothing is not valid. (error 4)

int varDeclaration()
{
	if(crntToken().type != intsym)
		return;
	nextToken();
	int offset = 3;
	while(crntToken().type == identsym)
	{
		Token t = crntToken();
		addSymbol(variable, t.name,  offset++);
		nextToken();
		if(crntToken().type != commasym)
			break;
		else
			nextToken();
	}
	if(crntToken().type != semicolonsym)
		 error(5, "semicolon or comma missing.");
	else{
		nextToken();
		return offset;
    }
}

procDeclaration()
{
    char procName[MAX_IDENT_LEN+1];
    while(crntToken().type == procsym)
    {
        if(nextToken().type != identsym)
            error(4, "procedure keyword must be followed by an identifier.");
        else
        {
            Token t= crntToken();
            strcpy(procName, t.name);
        } 
        if(nextToken().type != semicolonsym)
            error(26, "Semicolon expected.");
        nextToken();
         addSymbol(procedure, procName, cx);
        addSymbolTable(stl);
        block(0);
        removeSymbolTable(stl);
        if(crntToken().type != semicolonsym)
            error(6, "Incorrect symbol after procedure declaration (semicolon expected).");
        nextToken();
    }
}

void statement()
{
    int counterTemp;
	TokenType crnt = crntToken().type;
	Token t;
	Symbol s;
	
	switch(crnt)
	{
		case identsym:
            ;
             t = crntToken();
			 s = getSymbol(t.name);
			if(s.type == nulsymbol)
			     error(11, "undeclared identifier.");
			else if(s.type != variable)
				error(12, "assignment to constant or procedure is not allowed.");
			if(nextToken().type != becomessym)
				error(28, "error, := expected after variable.");
			nextToken();
			expression();
			emit(STO, s.lex, s.value);
			
			break;
		case callsym:
            if(nextToken().type != identsym)
                error(14, "call must be followed by a procedure.");
             t = crntToken();
             s = getSymbol(t.name);
            if(s.type != procedure)
               error(32, "attempt to call an identifier that is not a procedure.");
            emit(CAL, s.lex, s.value);
            nextToken();
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
			break;
		case insym:
            if(nextToken().type != lparentsym)
                error(29, "left parenthesis expected.");
            if(nextToken().type != identsym)
                error(31, "variable expected");
            t   = crntToken();
            Symbol s = getSymbol(t.name);
            if(s.type != variable)
               error(31, "variable expected.");
            emit(SIO, 0, INPUT);
            emit(STO, s.lex, s.value);
            if(nextToken().type != rparentsym)
                error(22, "right parenthesis missing.");
            nextToken();
            break;
        case outsym:
            if(nextToken().type != lparentsym)
                error(29, "left parenthesis expected.");
            nextToken();
            expression();
            emit(SIO, 0, OUTPUT);
            if(crntToken().type != rparentsym)
                error(22, "right parenthesis missing.");
            nextToken();
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
	
	stl = malloc(sizeof(SymbolTableList));
	stl-> crntLex = 0;
    stl->arr[0] = initTable(TABLE_SIZE);	
	program();
	exportCode(fout, code);
	exit(0);
}
