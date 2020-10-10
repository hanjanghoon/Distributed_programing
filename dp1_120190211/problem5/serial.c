#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#include <assert.h>
/**********************************************
 * Basic Image sturcture                      *
 **********************************************/
 
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
    unsigned char **grayvalue;

    int max;
} GRAY_Image;

/**********************************************
 * Image I/O		                      *
 **********************************************/
int
imread_RGB(char *filename, RGB_Image *img){
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

int 
imwrite_GRAY(char *filename, GRAY_Image *img){
    int i, j;
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

    for(j = 0; j < img->height; ++j) {
        for(i = 0; i < img->width; ++i) {
            fprintf(fp, "%d ", img->grayvalue[j][i]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);

    return 0;
}

int 
horizontal_flip_RGB(RGB_Image *img){
    int i, j;

    for(j = 0; j < img->height; ++j) {
        for(i = 0; i < img->width/2; ++i) {
            
            RGBPixel tmp = img->pixels[j][i];
            img->pixels[j][i] = img->pixels[j][img->width - i - 1];
            img->pixels[j][img->width - i - 1] = tmp;
        }
    }
}
int graysmooth( GRAY_Image *src ,GRAY_Image *outsrc)
{
	int j=0;
	int i=0;
	int temp, max = -1;
	outsrc->height   = src->height;
    outsrc->width    = src->width;
	outsrc->grayvalue = (unsigned char **) calloc(outsrc->height, sizeof(unsigned char *));
	

	for(j = 0; j < src->height; ++j) 
	{
		outsrc->grayvalue[j] = (unsigned char *) calloc(outsrc->width, sizeof(unsigned char));
        for(i = 0; i < src->width; ++i) 
		{
		
			if (j == 0 || j == src->height - 1 || i == 0 || i == src->width - 1) 
                    outsrc->grayvalue[j][i] = src->grayvalue[j][i];
            else
			{
				float gray=0;
				for (int x = i - 1; x <= i + 1; ++x) {
					for (int y = j - 1; y <= j + 1; ++y) {
						
								gray += src->grayvalue[y][x]*smoothMatrix[x - (i - 1)][y - (j - 1)];
						}
					}
				if (max < gray)
					max = gray;
            
				//printf("%f",gray);
				outsrc->grayvalue[j][i] = (unsigned char)gray;

				
			}
		}
	}
	outsrc->max = max;
	 // SRC Image Memory Free

    //free(src->grayvalue);
}


int
rgb2gray(RGB_Image *src, GRAY_Image *dst){
    int i, j, k;
    int max = -1, temp;

    dst->height   = src->height;
    dst->width    = src->width;

    dst->grayvalue = (unsigned char **) calloc(dst->height, sizeof(unsigned char *));

    if (dst->grayvalue == NULL){
        fprintf(stderr, "ERROR: Memory Failure.\n");
        return -1;
    }

    for(i = 0; i < dst->height; ++i) {
        dst->grayvalue[i] = (unsigned char *) calloc(dst->width, sizeof(unsigned char));

        if (dst->grayvalue[i] == NULL){
            fprintf(stderr, "ERROR: Memory Failure\n");

            for (j = 0; j < i; ++j)
                 free(dst->grayvalue[j]);
            free(dst->grayvalue);

            return -1;
        }
    }

    for(j = 0; j < src->height; ++j) {
        for(i = 0; i < src->width; ++i) {
            temp = 0;
            for (k = 0; k < 3; ++k){
                temp += src->pixels[j][i].RGB[k];
            }

            temp /= 3;
            dst->grayvalue[j][i] = (unsigned char)temp;

            if (max < temp){
                max = temp;
            }
        }
    }

    dst->max = max;

    return 0;
}

int
main (int argc, char **argv){
    RGB_Image img;
    GRAY_Image gray;
	GRAY_Image smooth;
    if(argc != 2) {
        fprintf(stderr, "%s <filename>\n", argv[0]);
        return -1;
    }

    time_t startTime = clock();

    imread_RGB(argv[1], &img);
    horizontal_flip_RGB(&img);
    rgb2gray(&img, &gray);
	graysmooth(&gray,&smooth);
    imwrite_GRAY("./result/serial.ppm", &smooth);

    time_t endTime = clock();
    float perf = (float)(endTime - startTime)/(CLOCKS_PER_SEC);
    printf("Total time: %f\n", perf);

    return 0;
}

