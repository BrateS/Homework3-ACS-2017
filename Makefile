build : bmp_edit.c
	gcc bmp_edit.c -o bmp_edit -lm -Wall
run :	build
	./bmp_edit
clean :
	rm -f bmp_edit
