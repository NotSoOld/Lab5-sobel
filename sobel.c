#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_HDR
#define STBI_NO_LINEAR
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#define byte unsigned char

int kX[3][3] = {{-1, 0, 1},
                {-2, 0, 2},
                {-1, 0, 1}};
int kY[3][3] = {{ 1, 2, 1},
                { 0, 0, 0},
                {-1,-2,-1}};


byte ToGrayscale(byte r, byte g, byte b)
{
	return (byte)((r + g + b) / 3);
}

					 
void main(void)
{
	int x, y, n, i, j, Gx, Gy;
	byte *img;
	byte *grayImg;
	byte *sobelImg;
	
	img = stbi_load("images/3.jpg", &x, &y, &n, 0);
	if (img == NULL) {
		printf("Failed to load the image! Stopping.\n");
		exit(1);
	}
	printf("Input image: %ix%i, n = %i\n", x, y, n);
	printf("Converting to grayscale...\n");
	grayImg = (byte *)calloc(x*y, 1);
	for (i = 0; i < y; i++) {
		for (j = 0; j < x*3; j += 3) {
			grayImg[i*x+(int)(j/3)] = ToGrayscale(img[i*x*3+j], 
                                                  img[i*x*3+j+1], 
                                                  img[i*x*3+j+2]);
		}
	}
	printf("Converted. Saving as image...\n");
	stbi_write_png("images/3gray.png", x, y, 1, grayImg, 0);
	stbi_image_free(img);
	printf("Saved.\nCalculating Sobel operations...\n");
	sobelImg = (byte *)calloc((x-2)*(y-2), 1);
	for (i = 1; i < y-1; i++) {
		for (j = 1; j < x-1; j++) {
		
			Gx = kX[0][0]*grayImg[(i-1)*x+(j-1)] + 
			     kX[0][1]*grayImg[i*x+(j-1)] +
			     kX[0][2]*grayImg[(i+1)*x+(j-1)] +
			     kX[1][0]*grayImg[(i-1)*x+j] + 
			     kX[1][1]*grayImg[i*x+j] +
			     kX[1][2]*grayImg[(i+1)*x+j] +
			     kX[2][0]*grayImg[(i-1)*x+(j+1)] + 
			     kX[2][1]*grayImg[i*x+(j+1)] +
			     kX[2][2]*grayImg[(i+1)*x+(j+1)];
			     
			Gy = kY[0][0]*grayImg[(i-1)*x+(j-1)] + 
			     kY[0][1]*grayImg[i*x+(j-1)] +
			     kY[0][2]*grayImg[(i+1)*x+(j-1)] +
			     kY[1][0]*grayImg[(i-1)*x+j] + 
			     kY[1][1]*grayImg[i*x+j] +
			     kY[1][2]*grayImg[(i+1)*x+j] +
			     kY[2][0]*grayImg[(i-1)*x+(j+1)] + 
			     kY[2][1]*grayImg[i*x+(j+1)] +
			     kY[2][2]*grayImg[(i+1)*x+(j+1)];    
			sobelImg[(i-1)*(x-2)+(j-1)] = (byte)(sqrt(Gx*Gx + Gy*Gy));
		}
	}
	free(grayImg);
	printf("Saving image after Sobel operator...\n");
	stbi_write_png("images/3sobel.png", x-2, y-2, 1, sobelImg, 0);
	free(sobelImg);
	printf("Done.\n");
	exit(0);
}
