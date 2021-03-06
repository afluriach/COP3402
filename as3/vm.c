#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define MAX_STACK_HEIGHT 2000
#define MAX_CODE_LENGTH 500
#define MAX_LEXI_LEVEL 3

//stack frame:
//bp+0 = static link
//bp+1 = dynamic link
//bp+2 = return address

typedef struct 
{
    int op, lex, mod;
} Instruction;

typedef struct
{
    int sp, bp, pc, haltFlag;
    Instruction ir;
} Cpu;

typedef struct
{
    int * data;
    int * frames;
    int frameDepth;
} Stack;

//typedef of function pointer to be used for all instructions in the instruction set
typedef void (*IsaFunc) (Stack * stack, Cpu * cpu);

void lit (Stack * stack, Cpu * cpu)
{
    if(spCheck(cpu->sp+1)) return;
    stack->data[++cpu->sp] = cpu->ir.mod;
}

void lod(Stack * stack, Cpu* cpu)
{
    if(spCheck(cpu->sp+1)) return;
    stack->data[++cpu->sp] = stack->data[findArBase(cpu->ir.lex, cpu->bp, stack->data) + cpu->ir.mod];
}

void sto(Stack * stack, Cpu* cpu)
{
    if(spCheck(cpu->sp-1)) return;
    stack->data[findArBase(cpu->ir.lex, cpu->bp, stack->data)+cpu->ir.mod] = stack->data[cpu->sp--];
}

void jmp (Stack * stack, Cpu * cpu)
{
    cpu->pc = cpu->ir.mod;
}

void jpc (Stack * stack, Cpu * cpu)
{
    if(spCheck(cpu->sp -1)) return;
    if(stack->data[cpu->sp] == 0)
    {
        cpu->pc = cpu->ir.mod;
    }
    cpu->sp--;
}

void sio (Stack * stack, Cpu * cpu)
{
    if(cpu->ir.mod ==2)
    {
        if(spCheck(cpu->sp+1)) return;
        char buf[12];
        printf("Input an integer: ");
        fgets(buf, 12, stdin);
        stack->data[++cpu->sp] = atoi(buf);
    }
    else if(cpu->ir.mod ==0 || cpu->ir.mod ==1)
    {
        if(spCheck(cpu->sp-1)) return;
        printf("output from VM: %d\n", stack->data[cpu->sp--]);
    }
    else
    {
        printf("Error: invalid SIO mod %d.\n", cpu->ir.mod);
        cpu->haltFlag = 1;
    }
}

void inc (Stack * stack, Cpu * cpu)
{
    if(spCheck(cpu->sp + cpu->ir.mod)) return;
    cpu->sp += cpu->ir.mod;
}

void cal(Stack * stack, Cpu* cpu)
{
    if(stack->frameDepth >= MAX_LEXI_LEVEL)
    {
        printf("Error: max call depth exceeeded.\n");
        cpu->haltFlag=1;
        return;
    }
    if(spCheck(cpu->sp + 3)) return;
    stack->frames[stack->frameDepth++] = cpu->sp;
    stack->data[cpu->sp+1] = findArBase(cpu->ir.lex, cpu->bp, stack->data); //static link
    stack->data[cpu->sp+2] = cpu->bp; //dynamic link
    stack->data[cpu->sp+3] = cpu->pc; //return address
    cpu->bp = cpu->sp+1;
    cpu->pc = cpu->ir.mod;
}

void opr(Stack * stack, Cpu* cpu)
{
    switch(cpu->ir.mod)
    {
        case 0:  //ret
            if(stack->frameDepth ==0)
            {
                //return  from main
                cpu->haltFlag = 1;
                return;
            }
            if(spCheck(cpu->bp-1)) return;
            cpu->sp = cpu->bp - 1;
            cpu->pc = stack->data[cpu->sp+3];
            cpu->bp = stack->data[cpu->sp +2];
            stack-> frameDepth--;
            break;
        case 1: //neg
            stack->data[cpu->sp] *= -1;
            break;
        case 2: //add
            if(spCheck(--cpu->sp)) return;
            stack->data[cpu->sp] += stack->data[cpu->sp +1];
            break;
        case 3: //sub
            if(spCheck(--cpu->sp)) return;
            stack->data[cpu->sp] -= stack->data[cpu->sp +1];
            break;
        case 4: //mul
            if(spCheck(--cpu->sp)) return;
            stack->data[cpu->sp] *= stack->data[cpu->sp +1];
            break;
        case 5: //div
            if(spCheck(--cpu->sp)) return;
            stack->data[cpu->sp] /= stack->data[cpu->sp +1];
            break;
       case 6: //odd
            stack->data[cpu->sp] %= 2;
            break;
      case 7: //mod
            if(spCheck(--cpu->sp)) return;
            stack->data[cpu->sp] %= stack->data[cpu->sp +1];
            break;
      case 8: //eql
           if(spCheck(--cpu->sp)) return;
            stack->data[cpu->sp] = (stack->data[cpu->sp] == stack->data[cpu->sp +1]);
            break;
      case 9: //neq
            if(spCheck(--cpu->sp)) return;
            stack->data[cpu->sp] = (stack->data[cpu->sp] != stack->data[cpu->sp +1]);
            break;
      case 10: //lss
            if(spCheck(--cpu->sp)) return;
            stack->data[cpu->sp] = (stack->data[cpu->sp] < stack->data[cpu->sp +1]);
            break;
       case 11: //leq
            if(spCheck(--cpu->sp)) return;
            stack->data[cpu->sp] = (stack->data[cpu->sp] <= stack->data[cpu->sp +1]);
            break;
      case 12: //gtr
            if(spCheck(--cpu->sp)) return;
            stack->data[cpu->sp] = (stack->data[cpu->sp] > stack->data[cpu->sp +1]);
            break;
       case 13: //geq
            if(spCheck(--cpu->sp)) return;
            stack->data[cpu->sp] = (stack->data[cpu->sp] >= stack->data[cpu->sp +1]);
            break;
       default:
            printf("Error: invalid OPR modifier %d.\n", cpu->ir.mod);
    }
}

IsaFunc operations[11] = {NULL, lit, opr, lod, sto, cal, inc, jmp, jpc, sio, sio};

char opName [][4]  = { "", "LIT", "OPR", "LOD", "STO", "CAL", "INC", "JMP", "JPC", "SIO", "SIO"};

void init(Cpu ** cpu, Stack ** stack, Instruction ** instrArr)
{
    *cpu = malloc(sizeof(Cpu));
    *stack = malloc(sizeof(Stack));
    *instrArr = malloc(sizeof(Instruction)*MAX_CODE_LENGTH);

    if(cpu==NULL || stack == NULL || instrArr == NULL)
    {
        printf("Error allocating memory. Program will now terminate.\n");
        exit(0);
    }
    
    (*stack)->data = malloc(sizeof(int)*MAX_STACK_HEIGHT);
    (*stack)->frames = malloc(sizeof(int)*MAX_LEXI_LEVEL);
    (*stack)->frameDepth=0;
    
    if((*stack)->data==NULL || (*stack)->frames == NULL)
    {
        printf("Error allocating memory. Program will now terminate.\n");
        exit(0);
    }

    (*cpu)->sp = 0;
    (*cpu)->bp = 1;
    (*cpu)-> pc = 0;
    (*cpu)->haltFlag = 0;
    (*cpu)->ir.op = 0; 
    (*cpu)->ir.mod=0;
    (*cpu)->ir.lex = 0;
    
    (*stack)->data[0]= 0;
    (*stack)->data[1] = 0;
    (*stack)->data[2] = 0;
    (*stack)->data[3] = 0;
}


int findArBase(int lex, int bp, int * stack)
{
    //note, this assumes the dynamic link is the second thing on the AR

    for(;lex > 0; bp = stack[bp], lex--);
    return bp;
}

void fetch(Cpu * cpu, Instruction * code)
{
    cpu->ir = code[cpu->pc];
    cpu->pc++;
} 

void execute(Cpu * cpu, Stack * stack)
{
    assert(cpu->ir.op <= 10);
    assert(cpu->ir.op >= 1);
    (operations[cpu->ir.op])(stack, cpu);
}

int readBin(Instruction * code, FILE* fin)
{
    int op, lex, mod, i;
    for(i=0; fscanf(fin, "%d %d %d", &op, &lex, &mod) != EOF; i++)
        code[i].op = op, code[i].lex = lex, code[i].mod = mod;
    return i;
}

void printAsm(Instruction * code, int len)
{
    int i;
    printf("Line\tOP\tL\tM\n");
    for(i=0;i<len;i++)
    {
        printf("%d\t%s\t%d\t%d\n",i, opName[code[i].op], code[i].lex, code[i].mod);
    }
}

void printCpu(Cpu * cpu)
{
    printf("%d\t%d\t%d", cpu->pc, cpu->bp, cpu->sp);
}

void printStack(Stack * stack, int sp)
{
    int i, j;
    if(sp ==0 || sp ==1)
        sp = 3;
    //even if the sp is zero, assume that the MAIN stack frame has been initialized and print it out.    
    
    for(i=1, j=0;i<=sp; i++)
    {
        printf("%d ", stack->data[i]);
        if(i==stack->frames[j] && j < stack->frameDepth && i < sp-1)
        {
            printf("|");
            j++;
        }
    }
            
}

//checks the bounds of sp. Only an integer is passed for sp
//so the result of sp after an operation would be performed can be checked

//returns true if error is detected; the calling function should return without performing any operation on the stack if this function returns true
int spCheck(int sp, Cpu * cpu)
{
    if(sp < 0)
    {
        printf("Error: Attempt to decrement sp on empty stack.\n");
        cpu->haltFlag = 1;
        return 1;
    }
    if(sp >= MAX_STACK_HEIGHT)
    {
        printf("Error: Stack overflow.\n");
        cpu->haltFlag = 1;
        return 1;
    }
    return 0;
}

int pcCheck(int pc, Cpu * cpu)
{
    if(pc < 0 || pc >= MAX_CODE_LENGTH)
    {
        printf("Error: program counter out of bounds; pc=%d\n", pc);
        cpu->haltFlag = 1;
        return 1;
    }
    else return 0;
}

void pCheck(Cpu * cpu)
{
    spCheck(cpu->sp, cpu);
    pcCheck(cpu->pc, cpu);
}

void printTraceHeader()
{
    printf("pc\tbp\tsp\tstack\n");
}

int main(int argc, char **argv)
{
    FILE * asmFin;
    //determines whether binary or assembler instructions will be read in
    if(argc !=2)
    {
        printf("Pass the name of the PM/0 \"binary\".\n");
        exit(0);
    }
    asmFin = fopen(argv[1], "r");
        
    if(asmFin == NULL)
    {
        printf("Error openining file.\n");
        exit(0);
    }
    
    Cpu * cpu;
    Instruction * code;
    Stack * stack;
    init(&cpu, &stack, &code);
    
    int codeLen = readBin(code, asmFin);
    printAsm(code, codeLen);
    printTraceHeader();
    
    while(!cpu->haltFlag)
    {
        printCpu(cpu);
        printf("\t");
        printStack(stack, cpu->sp > cpu->bp ? cpu->sp : cpu-> bp);
        printf("\n");
    
        fetch(cpu, code);
        pCheck(cpu);
        execute(cpu, stack);
        pCheck(cpu);
    }
    printf("CPU execution halted.\n");
    exit(0);
}

