assembler: assembler.o
	gcc -g -ansi -Wall -pedantic assembler.o first_pass.o second_pass.o utils.o globals.o -o assembler
	
assembler.o: assembler.c 
	gcc -c -ansi -Wall -pedantic assembler.c -o assembler.o   

first_pass.o: first_pass.c
	gcc -c -ansi -Wall -pedantic first_pass.c -o first_pass.o	

second_pass.o: second_pass.c
	gcc -c -ansi -Wall -pedantic second_pass.c -o second_pass.o

utils.o: utils.c
	gcc -c -ansi -Wall -pedantic utils.c -o utils.o

globals.o: globals.c 
	gcc -c -ansi -Wall -pedantic globals.c -o globals.o 
