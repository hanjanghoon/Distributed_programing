/*
 * This is sample code generated by rpcgen.
 * These are only templates and you can use them
 * as a guideline for developing your own functions.
 */

#include "oper.h"
#include "string.h"
#define MAX_STACK_SIZE 100

typedef int element;
typedef struct {
    element stack[MAX_STACK_SIZE];
	int top;
}StackType;

void init(StackType* s) {
    s->top = -1;
}

int is_empty(StackType* s) {
    return (s->top == -1);
}

int is_full(StackType* s) {
    return (s->top == MAX_STACK_SIZE - 1);
}

void push(StackType* s, element item) {
    if (is_full(s)) {
	fprintf(stderr, "full error\n");
	    return;
    }
    
    else
	s->stack[++(s->top)] = item;
}

element pop(StackType* s) {
    if (is_empty(s)) {
	fprintf(stderr, "empty error\n");
	    exit(1);
    }
    else
	return s->stack[(s->top)--];
}

element peek(StackType* s) {
    if (is_empty(s)) {
	fprintf(stderr, "full error\n");
	    return;
    }
    else
	return s->stack[s->stack[s->top]];
}

int prec(char op) {
    switch (op) {
	case'(': case')':return 0;
	case'+': case'-': return 1;
	case'*': case'/': return 2;
    }
    return -1;
}



void infix_to_postfix(char postfix[],char exp[]) {
    int i = 0;
	char ch, top_op;
	int len = strlen(exp);
	//char postfix[50];
	StackType s;
	
	init(&s);
	int k=0;
    
	for (i = 0; i < len; i++) {
	    ch = exp[i];
		switch (ch) {
		    case '+': case '-': case '*': case '/':
			
							   while (!is_empty(&s) && (prec(ch) <= prec(peek(&s))))
							       postfix[k++]=pop(&s);
								   push(&s, ch);
								   break;
								   
		    case '(':
			     push(&s, ch);
				 break;
		    case ')':
			     top_op = pop(&s);
				 while (top_op != '(') {

				     postfix[k++]=pop(&s);

					 top_op = pop(&s);
				 }
			     break;
		    default:
			    postfix[k++]=ch;

				break;
		}
	}
    

	while (!is_empty(&s))
	{
	    postfix[k++] = pop(&s);
	}
	postfix[k]='\0';
	return;
	
}



    void
oper_prog_1(char *host)
{
    CLIENT *clnt;
    int  *result_1;
    input  addition_1_arg;
    int  *result_2;
    input  subtraction_1_arg;
    int  *result_3;
    input  multiplication_1_arg;
    int  *result_4;
    input  division_1_arg;
    FILE *fp;
    int op1, op2 = 0;
    char filename[20];
    int i=0;
    int value=0;
    char instr[50];
    StackType s;
    char ch;
    char post[50];
#ifndef	DEBUG
    clnt = clnt_create (host, OPER_PROG, OPERS_VERS, "udp");
    if (clnt == NULL) {
	clnt_pcreateerror (host);
	exit (1);
    }
#endif	/* DEBUG */
    printf("assignment > ");
    scanf("%s",filename);
    //	printf("%s",filename);

    fp = fopen(filename, "r");
    if (fp == NULL) {
	printf("fileopne error!\n");
	return;
    }

    fscanf(fp, "%s", instr);
    printf("%s\n",instr);
	
    fclose(fp);
    infix_to_postfix(post, instr);
	
	 
	//	printf("gkjdkgjdkjgk%s",post);
	
	
	//return;
//	printf("%s",post);
	//printf("\n%d\n",strlen(post));
	init(&s);
	for (i = 0; i < strlen(post); i++) {
	    ch = post[i];
		//printf("loop!\n");
	     //printf("iter%c\n",ch);
	//    return ;
		if (ch != '+' && ch != '-' && ch != '*' && ch != '/') {
		    value = ch - '0';
			push(&s, value);
		}
		else {
		    op2 = pop(&s);
			op1 = pop(&s);
			
			switch (ch) {
			    case '+': 
				      {
					  addition_1_arg.a = op1;
					      addition_1_arg.b = op2;
					      result_1 = addition_1(&addition_1_arg, clnt);
					  if (result_1 == (int *)NULL) {
					      clnt_perror(clnt, "+ call failed");
					  }
					 // return;
					// return;
			//		printf("add result=%d\n",*result_1);
					  push(&s, *result_1); break;
				      }
			    case '-':
				     {
					 subtraction_1_arg.a = op1;
					     subtraction_1_arg.b = op2;
					     result_2 = subtraction_1(&subtraction_1_arg, clnt);
					 if (result_2 == (int *)NULL) {
					     clnt_perror(clnt, "- call failed");
					 }
					 push(&s, *result_2); break;
				     }
			    case '*':
				     {
				//	 printf("DFDDFFDDF");
					 multiplication_1_arg.a = op1;
					 multiplication_1_arg.b = op2;
					// printf("%d,%d",multiplication_1_arg.a,multiplication_1_arg.b);
					 //return;   
					 result_3 = multiplication_1(&multiplication_1_arg, clnt);
					 if(result_3==NULL)
					     printf("thersnoaswer\n\n");
					 //return;
				//		printf("mult :result=%d\n",*result_3);
						//return;
					 if (result_3 == (int *)NULL) {
					     clnt_perror(clnt, "* call failed");
					 }
					 
					 
					 push(&s, *result_3); break;
				     }
			    case '/':
				     {
					 division_1_arg.a = op1;
					     division_1_arg.b = op2;
					     result_4 = division_1(&division_1_arg, clnt);
					 if (result_4 == (int *)NULL) {
					     clnt_perror(clnt, "/call failed");
					 }
					 push(&s, *result_4); break;
				     }
			}
		}
	}
    //return;
    printf("anwser :%d\n",pop(&s));
	
	
	/*
	   result_1 = addition_1(&addition_1_arg, clnt);
	   if (result_1 == (int *) NULL) {
	   clnt_perror (clnt, "call failed");
	   }
	   result_2 = subtraction_1(&subtraction_1_arg, clnt);
	   if (result_2 == (int *) NULL) {
	   clnt_perror (clnt, "call failed");
	   }
	   result_3 = multiplication_1(&multiplication_1_arg, clnt);
	   if (result_3 == (int *) NULL) {
	   clnt_perror (clnt, "call failed");
	   }
	   result_4 = division_1(&division_1_arg, clnt);
	   if (result_4 == (int *) NULL) {
	   clnt_perror (clnt, "call failed");
	   }*/
#ifndef	DEBUG
	clnt_destroy (clnt);
#endif	 /* DEBUG */
}


    int
main (int argc, char *argv[])
{
    char *host;

    if (argc < 2) {
	printf ("usage: %s server_host\n", argv[0]);
	exit (1);
    }
    host = argv[1];
    oper_prog_1 (host);
    exit (0);
}