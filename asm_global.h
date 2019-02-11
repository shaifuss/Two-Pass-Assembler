/*
 * asm_global.h
 *
 *  Created on: Feb 14, 2017
 *      Author: Avishai Fuss 332658608
 *
 *      global header file
 */


#ifndef ASM_GLOBAL_H_
#define ASM_GLOBAL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>

#define MAX_CODE 50			/* max chars per line in .as file */
#define MAX_LBL 30			/* max label length */
#define CODE_OFFSET 100		/* offset for data addresses */

/* skip whitespace */
#define SKIP_WHITE(x) while (isspace(*x)) x++;
/* remove trailing whitespace */
#define TRIM_WHITE(x) {\
	char * end;\
	end = x + strlen(x) - 1;\
	while(end > x && isspace((unsigned char)*end)) end--;\
	*(end+1) = 0;\
}
/* add boolean functionality */
typedef enum { FALSE, TRUE } bool;

/*global variable declarations*/
extern int i_count /* instruction count */, d_count /* data count */;
extern int s_count;	/* source line counter */
extern bool error_flag;	/* does source file have errors? */

#endif /* ASM_GLOBAL_H_ */
