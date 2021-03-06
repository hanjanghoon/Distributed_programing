/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#ifndef _OPER_H_RPCGEN
#define _OPER_H_RPCGEN
#include <pthread.h>
#include <rpc/rpc.h>


#ifdef __cplusplus
extern "C" {
#endif


struct input {
	int a;
	int b;
};
typedef struct input input;

#define OPER_PROG 0x20000000
#define OPERS_VERS 1

#if defined(__STDC__) || defined(__cplusplus)

#define addition 1
extern  int * addition_1(input *, CLIENT *);
extern  int * addition_1_svc(input *, struct svc_req *);
#define subtraction 2
extern  int * subtraction_1(input *, CLIENT *);
extern  int * subtraction_1_svc(input *, struct svc_req *);
#define multiplication 3
extern  int * multiplication_1(input *, CLIENT *);
extern  int * multiplication_1_svc(input *, struct svc_req *);
#define division 4
extern  int * division_1(input *, CLIENT *);
extern  int * division_1_svc(input *, struct svc_req *);
extern int oper_prog_1_freeresult (SVCXPRT *, xdrproc_t, caddr_t);


#else /* K&R C */
#define addition 1
extern  int * addition_1();
extern  int * addition_1_svc();
#define subtraction 2
extern  int * subtraction_1();
extern  int * subtraction_1_svc();
#define multiplication 3
extern  int * multiplication_1();
extern  int * multiplication_1_svc();
#define division 4
extern  int * division_1();
extern  int * division_1_svc();
extern int oper_prog_1_freeresult ();
#endif /* K&R C */

/* the xdr functions */

#if defined(__STDC__) || defined(__cplusplus)
extern  bool_t xdr_input (XDR *, input*);

#else /* K&R C */
extern bool_t xdr_input ();

#endif /* K&R C */

#ifdef __cplusplus
}
#endif

#endif /* !_OPER_H_RPCGEN */
