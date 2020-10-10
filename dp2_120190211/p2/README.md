# Multithreaded Programming

1. Source code
    - pth_ll_rwl.c: pthread의 read-write lock을 이용한 first writer reader problem을 해결한 코드
    - pth_ll_rwl_cm.c: pthread의mutex와 conditional variable을 이용한 코드
   
2. Complile
    - using Makefile
        * $ make

3. Usage
	- $./pth_ll_rwl [thread_count]
    - $./pth_ll_rwl_cm [thread_count]


    * 차례대로 thread에입력할 key의 숫자, ops 의숫자, insert의 비율(0~1), search의 비율(0~1)을입력한다
