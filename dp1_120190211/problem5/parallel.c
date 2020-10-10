#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <mpi.h>

#define IMAGE_PART_SEND_RECV   0

float smoothMatrix[3][3] = {{1.f / 9.f, 1.f / 9.f, 1.f / 9.f}, 
                            {1.f / 9.f, 1.f / 9.f, 1.f / 9.f}, 
                            {1.f / 9.f, 1.f / 9.f, 1.f / 9.f}};

typedef struct {
    unsigned char RGB[3];
} RGBPixel;

typedef struct {
    int width;
    int height;
    RGBPixel **pixels;

    int max;
} RGB_Image;

typedef struct {
    int width;
    int height;
    unsigned char *grayvalue;

    int max;
} GRAY_Image;


int imread_RGB(char *filename, RGB_Image *img){
    char magic[2];
    int i, j;
    FILE *fp;

    if((fp = fopen(filename, "rb")) == NULL){
        fprintf(stderr, "No File open\n");
        return -1;
    }

    fscanf(fp, "%c %c\n", &magic[0], &magic[1]);
    if((magic[0] != 'P' || magic[1] != '6') && (magic[0] != 'P' || magic[1] != '3')) {
        fprintf(stderr, "Invalid PPM_RGB format\n");
        return -1;
    }

    fscanf(fp, "%d %d\n", &img->width, &img->height);
    fscanf(fp, "%d\n", &img->max);

    if(img->max > 255) {
        fprintf(stderr, "Invalid 8bit-image format\n");
        return -1;
    }

    img->pixels = (RGBPixel **) calloc(img->height, sizeof(RGBPixel *));
    if (img->pixels == NULL){
        fprintf(stderr, "ERROR: Memory Failure.\n");
        return -1;
    }

    for(i = 0; i < img->height; ++i) {
        img->pixels[i] = (RGBPixel *) calloc(img->width, sizeof(RGBPixel));

        if (img->pixels[i] == NULL){
            fprintf(stderr, "ERROR: Memory Failure\n");

            for (j = 0; j < i; ++j)
                 free(img->pixels[j]);
            free(img->pixels);
        }
    }

    for(j = 0; j < img->height; ++j) {
        for(i = 0; i < img->width; i++) {
            fread(&img->pixels[j][i], sizeof(RGBPixel), 1, fp);
        }
    }

    fclose(fp);

    return 0;
}

int imwrite_GRAY(char *filename, GRAY_Image *img){
    int i;
    FILE *fp;

    assert (filename != NULL);
    assert (img != NULL);

    if((fp = fopen(filename, "w+")) == NULL){
        fprintf(stderr, "ERROR: File Error\n"); 
        return -1;
    }

    fprintf(fp, "P2\n");
    fprintf(fp, "%d %d\n", img->width, img->height);
    fprintf(fp, "%d\n", img->max);

    for(i = 0; i < img->height * img->width; ++i) {
        fprintf(fp, "%d ", img->grayvalue[i]);
    }
    fprintf(fp, "\n");
    fclose(fp);

    return 0;
}

int horizontal_flip_RGB(RGB_Image *img, int from, int to){
    int i, j;

    for(j = from; j < to; ++j) {
        for(i = 0; i < img->width/2; ++i) {
            RGBPixel tmp = img->pixels[j][i];
            img->pixels[j][i] = img->pixels[j][img->width - i - 1];
            img->pixels[j][img->width - i - 1] = tmp;
        }
    }
}

int graysmooth( GRAY_Image *src ,GRAY_Image *outsrc, int from, int to)
{
	int j=0;
	int i=0;
	int temp, max = -1;
	outsrc->height   = src->height;
    outsrc->width    = src->width;
	outsrc->grayvalue = (unsigned char *) calloc(outsrc->height * outsrc->width, sizeof(unsigned char));
	
	from=from+1;
	to=to-1;
	
	for(j = from; j < to; ++j) 
	{
        for(i = 0; i < src->width; ++i) 
		{
		
			if (j == 0 || j == src->height - 1 || i == 0 || i == src->width - 1) 
			{    
				outsrc->grayvalue[j * src->width + i] = src->grayvalue[j * src->width + i];
				//printf("%d,%d\n",from,i);
			}
            else
			{
				unsigned char gray=0;
				for (int x = i - 1; x <= i + 1; ++x) {
					for (int y = j - 1; y <= j + 1; ++y) {
						
								gray += src->grayvalue[y*(src->width)+x]*smoothMatrix[x - (i - 1)][y - (j - 1)];
						}
					}
				if (max < gray)
					max = gray;
            
				//printf("%f",gray);
				//if(gray==0)
					//printf("fffdfddffddf");
				outsrc->grayvalue[j*(src->width) + i] =  (unsigned char)gray;

				
			}
		}
	}
	outsrc->max = max;
	 // SRC Image Memory Free

   // free(src->grayvalue);
}


int rgb2gray(RGB_Image *src, GRAY_Image *dst, int from, int to)
{
    int i, j, k;
    int temp, max = -1;   
 
    if(src == NULL) {
        fprintf(stderr, "ERROR: Image SRC: NULL\n");
        return -1;
    }
    dst->height   = src->height;
    dst->width    = src->width;

    
    dst->grayvalue = (unsigned char *) calloc(dst->height * dst->width, sizeof(unsigned char));

    if (dst->grayvalue == NULL){
        fprintf(stderr, "ERROR: Memory Failure\n");
        return -1;
    }

    for(j = from; j < to; ++j) {
        for(i = 0; i < src->width; ++i) {
            temp = 0; 
            
            for (k = 0; k < 3; ++k)
                temp += src->pixels[j][i].RGB[k];

            temp /=3;
            dst->grayvalue[j * src->width + i] = (unsigned char)temp;

            if (max < temp){
                max = temp;
            }
        }
    }

    dst->max = max;


    for(i = 0; i < src->height; i++)
        free(src->pixels[i]);
    free(src->pixels);

    return 0;
}

int 
main(int argc, char **argv){
    int rc;
    int size, rank;
    RGB_Image img;
    GRAY_Image gray;
	GRAY_Image smooth;
	int fgf=0;
	int fgt=0;
    if( argc != 2 ) {
        fprintf(stderr, "%s <filename>\n", argv[0]);
        return -1;
    }

    rc = MPI_Init( &argc, &argv );

    if (rc != MPI_SUCCESS){
        fprintf(stderr, "ERROR: MPI Initialization Failure\n");
        return -1;
    }

    MPI_Comm_size( MPI_COMM_WORLD, &size );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );

    MPI_Barrier(MPI_COMM_WORLD);

    double local_start, local_finish, local_elapsed;
    local_start = MPI_Wtime();

    imread_RGB(argv[1], &img);
	//printf("%d\n",img.height);
    int partition = img.height / size + 1;
    int from = partition * rank;
    int to   = (partition * (rank + 1) <  img.height) ? (partition * (rank + 1)) :  img.height ;
    int partition_size   = (to - from) * img.width;
	
//printf("rank[%d]: from %d to %d\n", rank, from, to);
	if (to+1<img.height)
	{	to++;
		fgt=1;
	}
	if(from-1>=0)
	{
		from--;
		fgf=1;
	}
    horizontal_flip_RGB(&img, from, to);
    rgb2gray(&img, &gray, from, to);
	graysmooth(&gray,&smooth,from,to);
	if(fgf==1)
		from++;
	if(fgt==1)	
		to--;

    if (rank == size - 1)
      imwrite_GRAY("./result/parallelpart.ppm", &smooth);
	MPI_Datatype graytype;
	MPI_Type_contiguous(partition_size,MPI_CHAR,&graytype);
	MPI_Type_commit(&graytype);
    if(rank == 0){
        int recvfrom, i, count, rank;
        MPI_Request *req_handles = NULL;
        MPI_Status *stat_handles = NULL;

        if(size >= 2) {
            req_handles  = calloc(size - 1, sizeof(MPI_Request));
            stat_handles = calloc(size - 1, sizeof(MPI_Status));

            
            for(recvfrom = 1; recvfrom < size; ++recvfrom) {
                int partition_st_idx = partition * recvfrom * img.width;
                
                MPI_Irecv(&smooth.grayvalue[partition_st_idx], 1, graytype, recvfrom, IMAGE_PART_SEND_RECV, MPI_COMM_WORLD, &req_handles[recvfrom - 1]);
            }

            MPI_Waitall(size - 1, req_handles, stat_handles);

            int total_received = 0;

            for(rank = 1; rank < size; rank++) {
                MPI_Get_count(&stat_handles[rank-1], MPI_CHAR, &count);
                printf("rank[%d]: Received %d bytes\n", rank, count);
                total_received += count;
            }

            printf("[Master]: total %d bytes\n", total_received);
        }

        
        imwrite_GRAY("./result/parallel.ppm", &smooth);

        printf("[Master] successfully saved image\n");
        if(req_handles) {
            free(req_handles);
        }
        if(stat_handles) {
            free(stat_handles);
        }
    } 
    else {
        MPI_Request req_handle;
        MPI_Status  stat_handle;

        MPI_Isend(&smooth.grayvalue[from * img.width], 1, graytype, 0, IMAGE_PART_SEND_RECV, MPI_COMM_WORLD, &req_handle);
        MPI_Wait(&req_handle, &stat_handle);
 
        printf("Partition sent from node-[%d]\n", rank);
    }

    local_finish = MPI_Wtime();
    local_elapsed = local_finish - local_start;
    double elapsed;
    MPI_Reduce(&local_elapsed, &elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0)
        printf("Total time: %e\n", elapsed);

    MPI_Finalize();


    return 0;
}

