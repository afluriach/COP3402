#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#define MAX_IDENT_LEN 11

typedef enum { 
nulsym = 1, identsym, numbersym, plussym, minussym,
multsym,  slashsym, oddsym, eqsym, neqsym, lessym, leqsym,
gtrsym, geqsym, lparentsym, rparentsym, commasym, semicolonsym,
periodsym, becomessym, beginsym, endsym, ifsym, thensym, 
whilesym, dosym, callsym, constsym, intsym, procsym, outsym,
insym , elsesym } TokenType;

typedef struct
{
    TokenType type;
	char name[MAX_IDENT_LEN+1];
	int value;
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

void resizeList(TokenList * tl)
{
    tl->size *= 1.5;
    tl->list = realloc(tl->list, siezof(Token)*size);
}

Token getToken(TokenList * tl, int pos)
{
    assert(pos >=0 && pos < tl->len);
    return tl->list[pos];
}

typedef enum
{
    cons=1, var, proc
} SymbolType;


//hash table with separate chaining
typedef struct SymbolNode
{
    SymbolType type;
    char name[12];
    int value;
    int lex;
    int addr;
    SymbolNode * next;
} Symbol;

typedef struct
{
    
}SymbolTable;

//define the hash func in the SymbolTable?

Symbol * getSymbol(char * identifier, SymbolTable * st)
{

}

Symbol * addSymbol(Symbol* s, SymbolTable * st)
{
    
}




parseProgram()
{
    parseBlock();
    if(getToken().type != periodsym)
        error("program must end with a period");
}

TokenList * tl;
SymbolTable * st;
int crntToken=0, nTokens;


int main(int argc, char ** argv)
{
    if(argc != 2 && argc != 3)
        printf("parser input_file [output_file]\n");
    FILE *fin=fopen(argv[1], "r"), *fout = (argc==3) ? fopen(argv[2], "w") : stdout;
    assert(fin != NULL), assert(fout != NULL);
    //change to error opening input/output file
    
    int i, j;
    
    for(i=0;!feof(fin); i++, j++)
    {
        if(tl->list == tl->size)
            resizeList(tl);
        
        Token t;
        fscanf(fin, "%d", &t.type);
        if(t.type == identsym)
            fscanf(fin, "%s", t.name);  
        if(t.type == numsym)
            fscanf(fin, "%d", t.value);
       addSymbol(tl, t);
       nTokens = tl->len;    
    }
    
    
}
