# Multithreaded server: simple html server

1. Source code
    - server.c: master workder model with thread pool 기법을이용한 simple html server
    - server_epoll_: per model with thread pool 기법을 이용한 simple html server
    - client.c: 서버에 특정 page를 요청하는 프로그램. 
        thread를 이용하여 다수의 페이지를 동시에 요청하는 구조로 작성되어있다.

2. Complile
    - using Makefile
        * $ make

3. Usage
    - Server
        * ./http_server <port> <number of worker threads>
        * ./http_server_epoll <port> <number of worker threads>
    - Client
        * ./http_client <ip> <port> <number of threads> <number of reqs per thread>

4. 실험의 진행
    - 실행 파일 위치의 var/tmp/gr120190211 index.html 파일을 넣어 진행했다. 
	- 파일 위치가 안 맞을 경우 파일 오픈 에러가 난다.
	- request.txt를 읽어서 파일리스트를 만들도록 하였다.
    - port번호는 1111을 이용했다.
