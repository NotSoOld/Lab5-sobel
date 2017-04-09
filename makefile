regular:
	gcc -D_REENTERANT -o sobel sobel.c -lm -lpthread

memsan:
	gcc -D_REENTERANT -o sobel_m sobel.c -fsanitize=address -lm -lpthread
