regular:
	gcc -o sobel sobel.c -lm

memsan:
	gcc -o sobel_m sobel.c -fsanitize=address -lm
