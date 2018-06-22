#include <stdio.h>
#include <stdlib.h>


typedef enum {
nulsym = 1, identsym, numbersym, plussym, minussym,
multsym,  slashsym, oddsym, eqsym, neqsym, lessym, leqsym,
gtrsym, geqsym, lparentsym, rparentsym, commasym, semicolonsym,
periodsym, becomessym, beginsym, endsym, ifsym, thensym,
whilesym, dosym, callsym, constsym, intsym, procsym, outsym,
insym , elsesym } tokenType;

struct tokenNode
{
       tokenType type;
       struct tokenNode *next;
};

tokenType token = nulsym;

struct tokenNode *tokenList = NULL;

void getToken()
{
    token = tokenList->type;

    if(tokenList->next != NULL)
    {
        tokenList = tokenList->next;
    }
    else
    {
        tokenList = malloc(sizeof(struct tokenNode));

        tokenList->type = nulsym;
        tokenList->next = NULL;
    }
}

void error(int code)
{
    switch(code)
    {
        case 0:
            printf("const, int, procedure must be followed by"
"identifier.\n");
            break;
        case 1:
            printf("Identifier must be followed by =.\n");
            break;
        case 2:
            printf("= must be followed by a number.\n");
            break;
        case 3:
            printf("Semicolon or comma expected.\n");
            break;
        case 4:
            printf("Assignment operator expected.\n");
            break;
        case 5:
            printf("call must be followed by an identifier.\n");
            break;
        case 6:
            printf("end expected.\n");
            break;
        case 7:
            printf("then expected.\n");
            break;
        case 8:
            printf("do expected.\n");
            break;
        case 9:
            printf("Relational operator expected.\n");
            break;
        case 10:
            printf("Right parenthesis missing.\n");
            break;
        case 11:
            printf("Invalid expression.\n");
            break;
        case 12:
            printf("Unexpected end of file.\n");
            break;
        case 13:
            printf("Semicolon expected.");
        default:
            printf("An error has been detected.\n");
    }

    exit(EXIT_FAILURE);
}

void factor();

void term()
{
    factor();

    while(token == multsym || token == slashsym)
    {
        getToken();

        factor();
    }
}

void expression()
{
    if(token == plussym || token == minussym)
        getToken();

    term();

    while(token == plussym || token == slashsym)
    {
        getToken();

        term();
    }
}

void factor()
{
    if(token == identsym)
        getToken();
    else if(token == numbersym)
        getToken();
    else if(token == lparentsym)
    {
        getToken();

        expression();

        if(token != rparentsym)
            error(10);

        getToken();
    }
    else
        error(11);
}

void condition()
{
    if(token == oddsym)
    {
        getToken();

        expression();
    }
    else
    {
        expression();

        if(isRelation() != 1)
            error(9);

        getToken();

        expression();
    }
}

void statement()
{
    if(token == identsym)
    {
        getToken();

        if(token != becomessym)
            error(4);

        getToken();

        expression();
    }
    else if(token == callsym)
    {
        getToken();

        if(token != identsym)
            error(5);

        getToken();
    }
    else if(token == beginsym)
    {
        getToken();

        statement();

        while(token == semicolonsym)
        {
            getToken();

            statement();
        }

        if(token != endsym)
            error(6);

        getToken();
    }
    else if(token == ifsym)
    {
        getToken();

        condition();

        if(token != thensym)
            error(7);

        getToken();

        statement();
    }
    else if(token == whilesym)
    {
        getToken();

        condition();

        if(token != dosym)
            error(8);

        getToken();

        statement();
    }
}

void block()
{
    if(token == constsym)
    {
        do
        {
            getToken();

            if(token != identsym)
                error(0);

            getToken();

            if(token != eqsym)
                error(1);

            getToken();

            if(token != numbersym)
                error(2);

            getToken();
        } while(token == commasym);

        if(token != semicolonsym)
            error(3);

        getToken();
    }

    if(token == intsym)
    {
        do
        {
            getToken();

            if(token != identsym)
                error(0);

            getToken();
        } while(token == commasym);

        if(token != semicolonsym)
            error(3);

        getToken();
    }

    while(token == procsym)
    {
        getToken();

        if(token != identsym)
            error(0);

        getToken();

        if(token != semicolonsym)
            error(13);

        getToken();

        block();

        if(token != semicolonsym)
            error(13);

        getToken();
    }

    statement();
}

int isRelation()
{
    if(token == eqsym || token == neqsym || token == lessym || token ==
leqsym || token == gtrsym || token == geqsym)
        return 1;
    else
        return 0;
}

void addToken()
{
    struct tokenNode *temp = tokenList;
    if(temp != NULL)
    {
        while(temp->next != NULL)
            temp = temp->next;

        temp->next = malloc(sizeof(struct tokenNode));

        temp = temp->next;

        temp->type = token;
        temp->next = NULL;
    }
    else
    {
        tokenList = malloc(sizeof(struct tokenNode));

        tokenList->type = token;
        tokenList->next = NULL;
    }
}


int main(int argc, char *argv[])
{
    FILE *file;

    file = fopen(argv[1], "rt");

    while(fscanf(file, "%d", &token) != EOF)
        addToken();

    getToken();

    block();

    if(token != periodsym)
        error(12);

    return 0;
}

