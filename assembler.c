/*
 ============================================================================
 Name        : assembler.c
 Author      : Avishai Fuss - 332658608
 Version     : MMN14 Feb 2017
 Description : Translates assembly into machine code
 ============================================================================
 file contains main function, which governs progression through the program

The program implements the algorith suggested in the MMN14 specification
 */
#include "asm_global.h"

#define MEMORY 1000

void first_pass(FILE *fp);
void second_pass(FILE *fp);
void files_out(char * arg);
void free_table();
void free_ext();

int main(int argc, char *argv[])
{
	char filename[MAX_LBL];
	FILE *fp;

	if (argc < 2) {		/* no file names listed */
		fprintf(stderr, "No code files listed\n");
		exit(1);
	}
	while (--argc)	/*until all files are processed */
	{
		/* add .as to file name */
		strcpy(filename, *++argv);
		strcat(filename, ".as");

		/* open file */
		fp = fopen(filename, "r");
		if (fp == NULL) {
			perror("Error: ");
			exit(1);
		}

		first_pass(fp);		/* initial parsing of file, line by line */
		i_count = s_count = 0;		/* zero out counters */
		rewind(fp);

		second_pass(fp);	/* second parsing of file, line by line */
		fclose(fp);

		/* warning if code is over 1000 */
		if (d_count >= MEMORY)
			fprintf(stderr, "Warning: data exceeds memory");

		if (error_flag == FALSE)
			files_out(*argv);		/* output to files if no errors */

		/* free label tables */
		free_table();
		free_ext();
	}
	return 0;
}
