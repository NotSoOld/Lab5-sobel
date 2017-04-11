#define _REENTRANT

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_HDR
#define STBI_NO_LINEAR
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#define byte unsigned char
#define BUFSIZE 512
// Uncomment the following to see multithreading debug information:
//#define DEBUG_THREADING

int kX[3][3] = {{-1, 0, 1},
                {-2, 0, 2},
                {-1, 0, 1}};
int kY[3][3] = {{ 1, 2, 1},
                { 0, 0, 0},
                {-1,-2,-1}};

byte *img;			// Input image data pointer
byte *sobelImg;		// Output image data pointer
int threadsNum, x, y;

// Converts RGB pixel to grayscale pixel.
byte ToGrayscale(byte r, byte g, byte b)
{
	return (byte)((r + g + b) / 3);
}

// Calculates one component for Sobel operator
// from given matrix and image data.
int Convolute(int kernel[3][3], byte *imgData, int x)
{
	return 
	kernel[0][0] * ToGrayscale(imgData[-x-3], imgData[-x-2], imgData[-x-1]) +
	kernel[0][1] * ToGrayscale(imgData[-3],   imgData[-2],   imgData[-1])   +
	kernel[0][2] * ToGrayscale(imgData[x-3],  imgData[x-2],  imgData[x-1])  +
	kernel[1][0] * ToGrayscale(imgData[-x],   imgData[-x+1], imgData[-x+2]) +
	kernel[1][1] * ToGrayscale(imgData[0],    imgData[1],    imgData[2])    +
	kernel[1][2] * ToGrayscale(imgData[x],    imgData[x+1],  imgData[x+2])  +
	kernel[2][0] * ToGrayscale(imgData[-x+3], imgData[-x+4], imgData[-x+5]) +
	kernel[2][1] * ToGrayscale(imgData[3],    imgData[4],    imgData[5])    +
	kernel[2][2] * ToGrayscale(imgData[x+3],  imgData[x+4],  imgData[x+5]);
}

void ShowManual(void)
{
	printf("Usage: sobel file_name number_of_threads\n");
	printf("ERROR: not enough arguments. Stopping.\n");
	exit(1);
}

// Start method for each thread.
void *SobelThread(void *id)
{
	int i, j, Gx, Gy;
	int strip = (int)ceil(y / threadsNum);
	int indent = strip * (intptr_t)id;
	int stop = (strip+indent == y) ? y-1 : strip+indent;
	
	indent += ((intptr_t)id == 0) ? 1 : 0;
#ifdef DEBUG_THREADING
	printf("id = %li, indent = %i, stop = %i\n", (intptr_t)id, indent, stop);
#endif
	// We're working on image only inside given strip.
	// Size of strip is calculated according to the number of threads.
	for (i = indent; i < stop; i++) {
		for (j = 3; j < x*3-3; j += 3) {		     
			Gx = Convolute(kX, img + i*x*3+j, x*3);
			Gy = Convolute(kY, img + i*x*3+j, x*3);
			sobelImg[(i-1)*(x-2)+((int)(j/3)-1)] = (byte)(sqrt(Gx*Gx + Gy*Gy));
		}
	}
	pthread_exit(NULL);
}

// Creates the name of the output image.
void InsertSuffix(char *inputName, char *outputName)
{
	int i, j;
	
	for (i = strlen(inputName)-1; i >= 0; i--) {
		if (inputName[i] == '.')
			break;
	}
	for (j = 0; j < i; j++)
		outputName[j] = inputName[j];
	strcat(outputName, "_sobel.png");
}
					 
void main(int argc, char *argv[])
{
	int n, k;
	char outName[BUFSIZE];
	double tbefore, tafter;
	struct timespec before;
	struct timespec after;
	
	if (argc != 3)
		ShowManual();
	// Load image.
	img = stbi_load(argv[1], &x, &y, &n, 0);
	if (img == NULL) {
		printf("Failed to load the image %s! Stopping.\n", argv[1]);
		exit(2);
	}
	printf("Input image: %ix%i, %i pixel contents\n", x, y, n);
	
	memset(outName, '\0', BUFSIZE);
	InsertSuffix(argv[1], outName);

	printf("Calculating Sobel operations...\n");
	sobelImg = (byte *)calloc((x-2)*(y-2), 1);
	// Here we start multithreading.
	threadsNum = atoi(argv[2]);
	if (threadsNum == 0) {
		printf("Cannot parse number of threads '%s'! Stopping.\n", argv[2]);
		exit(3);
	}
	pthread_t threads[threadsNum];
	printf("Timer started.\n");
	// Get time before operations.
	if (clock_gettime(CLOCK_REALTIME, &before) == -1) {
		perror("Failed to get time before threading started");
		exit(7);
	}
	for (k = 0; k < threadsNum; k++) {
		// Create another thread and start computations in it.
		n = pthread_create(&(threads[k]), NULL, SobelThread, (void *)(intptr_t)k);
		if (n != 0) {
			printf("Failed to create thread #%i. Stopping.\n", k);
			perror("Purpose");
			exit(4);
		}
	}
	k = threadsNum - 1;
	for (; k >= 0; k--) {
		// After thread had finished its job, it'll be joined.
		n = pthread_join(threads[k], NULL);
		if (n != 0) {
			printf("Failed to join thread #%i. Stopping.\n", k);
			perror("Purpose");
			exit(5);
		}
#ifdef DEBUG_THREADING
		else {
			printf("Joined thread #%i\n", k);
		}
#endif
	}
	// Get time after operations.
	if (clock_gettime(CLOCK_REALTIME, &after) == -1) {
		perror("Failed to get time after threading finished");
		exit(8);
	}
	printf("Timer stopped.\n");
	// Calculate and print elapsed time.
	tbefore = before.tv_sec + before.tv_nsec * 0.000000001;
	tafter = after.tv_sec + after.tv_nsec * 0.000000001;
	tafter -= tbefore;
	printf("Elapsed time: %.3lf s\n", tafter);
	// Write filtered image to disk.
	printf("Saving image after Sobel operator as %s...\n", outName);
	n = stbi_write_png(outName, x-2, y-2, 1, sobelImg, 0);
	if (n == 0) {
		printf("Failed to save image to disk!\n");
		exit(6);
	}
	stbi_image_free(img);
	free(sobelImg);
	printf("Done.\n");
	exit(0);
}
