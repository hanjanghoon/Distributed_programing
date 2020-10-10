#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// for DP programming
#include <mpi.h>
#define ARRAY_SIZE 1000000
// Program Validation check

int
main(int argc, char **argv){
    int n, i, rc;
    int rank, size;

    int random_int;
    double local_start, local_finish, local_elapsed, elapsed;

    MPI_Comm communicator = MPI_COMM_WORLD;

    //1. Initialized
    rc = MPI_Init( &argc, &argv );

    if (rc != MPI_SUCCESS){
	printf("ERROR: Startig MPI Program!\n");
	MPI_Abort(communicator, rc);
    } 

    MPI_Comm_size(communicator, &size);
    MPI_Comm_rank(communicator, &rank);

    char hostname[256];
    gethostname(hostname, 255);

    
    MPI_Barrier(communicator);
    int local_data=0;
    if(rank==0)
	for( i = 0 ; i<ARRAY_SIZE%size;i++)
	{
	    local_data+=rand()%1024;
	}
    

    srand(time(NULL) + rank);
    int prefix_sum=0;
    local_start = MPI_Wtime();
    //printf("rank[%d]: %d\n", rank, local_data); 
    for( i = 0 ; i<ARRAY_SIZE/size;i++)
    {
		local_data+=rand()%50;
		//local_data+=10;
    }
    
    MPI_Scan(&local_data, &prefix_sum, 1, MPI_INT, MPI_SUM, communicator);

    local_finish  = MPI_Wtime();
    local_elapsed = local_finish - local_start;
    MPI_Reduce(&local_elapsed, &elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    //    printf("rank[%d](host on %s): prefix %d \n", rank, hostname, prefix_sum);

    MPI_Barrier(communicator);

    if(rank == 0)
	printf("Elapsed time = %e seconds\n", elapsed);

    MPI_Finalize();

    return 0;
}

