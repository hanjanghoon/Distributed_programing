/*
 * This is sample code generated by rpcgen.
 * These are only templates and you can use them
 * as a guideline for developing your own functions.
 */

#include "oper.h"
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

int *
addition_1_svc(input *argp, struct svc_req *rqstp)
{
	static int  result;

	/*
	 * insert server code here
	 */

  
   
	result = argp->a + argp->b;
	 

	return &result;
}

int *
subtraction_1_svc(input *argp, struct svc_req *rqstp)
{
	static int  result;

	/*
	 * insert server code here
	 */

	result = argp->a - argp->b;
	
	return &result;
}

int *
multiplication_1_svc(input *argp, struct svc_req *rqstp)
{
	static int  result;

	/*
	 * insert server code here
	 */

	result = argp->a * argp->b;

//	printf("****************");
//	printf("server: %d",argp->a);
//	result =40;
	return &result;
}

int *
division_1_svc(input *argp, struct svc_req *rqstp)
{
	static int  result;

	/*
	 * insert server code here
	 */

	result = argp->a / argp->b;

	return &result;
}
