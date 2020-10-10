# Distibuted Image Processing

1. Source code
    - sequential version: serial.c
    - parallel version: parallel.c

2. Complile
	- using makefile	
		* all: serial, parallel 
		* serial: serial
		* parellel: parallel

3. Usage
	- e.g.)
	  1) serial program: ./serial data/mcfaddin_1.ppm
	  2) parallel program: mpirun -np 4 (-hostfile hosts) parellel data/mcfaddin_1.ppm

4. PPM(or PGM) viewer: because of unauthorized ID , I use honeyview in window. 
    

5. hostfile example
    - hosts

6. Image source file folder
    - data

7. Result
    - result
