# Distibuted Image Processing

1. Source code
    - seqential locking: blocking.c
    - parallel version: mpiscan.c

2. Complile
	- using makefile	
		* all: blocking, mpiscan

3. Usage
	- e.g.)
	  1) sequential blocking program: mpirun -np 4 (-hostfile hosts) ./blocking
      2) MPI_Scan: mpirun -np 4 (-hostfile hosts) ./mpiscan

4. hostfile example
    - hosts
