# RPC Programming: Arithmatic server

1. Source code
    - IDL Specification file: oper.x
    - Server program
        * stub: oper_svc.c
        * service: oper_server.c
    - Client program
        * stub: oper_clnt.c
        * skeleton: oper_client.c

2. Complile(Test를 위해서는 makefile만 이용한다)
	- IDL specification: rpcgen 을 통해 RPC에 필요한파일들을 생성한다.
		* rpcgen –C –a oper.x
    - using Makefile
        * make –f makefile.oper

3. Usage
	- e.g.) server: $./oper_server
            client: $./oper_client [server_ip or server_host_name]
                    $./oper_client cspro5.sogang.ac.kr
