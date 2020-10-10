#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>

// for DP programming
#include <mpi.h>
#define ARRAY_SIZE 1000000


const int BROADCAST_TAG = 0;
const int PREFIX_TAG = 1;

int main(int argc, char **argv)
{
    int n;
    int rank, size;
	int newArr;
    MPI_Init( &argc, &argv );
    
    MPI_Comm_size( MPI_COMM_WORLD, &size );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );

    int i=0;
    int sum = 0;
    int temp = 0;
    int key = 1;
	srand(time(NULL));
	newArr=ARRAY_SIZE;
	int * chunk = (int *) malloc(sizeof(int) * newArr/size);
	int * array = (int *) malloc(sizeof(int) * newArr);
	if (rank==0)
	{
		if (ARRAY_SIZE%size!=0)
			newArr=ARRAY_SIZE-ARRAY_SIZE%size;
			
		
		
		for(i = 0; i < newArr; i++)
		{
		array[i] = rand() % 1024;
		}
		for(i = 0; i < ARRAY_SIZE%size; i++)
		{
			sum += rand() % 1024;
		//	printf("%d\n",sum);
		}
	}
	double local_start, local_finish, local_elapsed, elapsed;
    

    MPI_Barrier( MPI_COMM_WORLD );
    MPI_Scatter(array,ARRAY_SIZE/size,MPI_INT,chunk,ARRAY_SIZE/size,MPI_INT,0,MPI_COMM_WORLD);
		
    local_start = MPI_Wtime();

	
	
    for(i = 0; i < ARRAY_SIZE/size; i++)
    {
		sum += rand()%50;
		//sum+=10;
		//printf("%d\n",sum);
    }
	
    
    char hostname[256];
    gethostname(hostname, 255);
	int mynum=0;
    if(rank == 0) {
        int to;
		if (!(size == 1)){
            for(to = 1; to < size; to++)				
			{
                // blocking send
 
				MPI_Send(&sum, 1, MPI_INT, rank+1, PREFIX_TAG, MPI_COMM_WORLD); // pass it to rank 1
            }
        }
    } 
    else {
        int prev_prefix;

        // blocking recv
      
        //printf("rank[%d]: my_num = %d\n", rank, my_num);

        MPI_Recv(&prev_prefix, 1, MPI_INT, rank - 1, PREFIX_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // calculation for prefix sum
        sum += prev_prefix;

        if(rank + 1 < size)
            MPI_Send(&sum, 1, MPI_INT, rank + 1, PREFIX_TAG, MPI_COMM_WORLD);
    }

//    printf("rank[%d](on %s): prefix sum = %d\n", rank, hostname, sum);

    local_finish = MPI_Wtime();
    local_elapsed = local_finish - local_start;
    MPI_Reduce(&local_elapsed, &elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    MPI_Finalize();

    if (rank == 0){
        printf("Total time: %e\n", elapsed);
		free(array);
	}
	free(chunk);
    return 0;
}
