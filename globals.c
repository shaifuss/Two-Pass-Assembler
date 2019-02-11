/*
 * globals.c
 *
 *  Created on: Feb 14, 2017
 *      Author: Avishai Fuss 332658608
 *
 *      contains global variables and data structures
 */

#include "asm_global.h"
#include "passes.h"

sym_ptr table;			/*label look up table */
ext_table *ext_head;
int i_count = 0/* instruction count */, d_count = 0/* data count */;
int s_count = 0;   /*line counter for assembly source code */
bool has_label;
bool has_entry;		/* toggles output of ent file */
bool error_flag;	/* toggles output of files if no errors */

/**************************** table of data tags *****************************/
tag_menu menu[] = {
		{"blank", NO},
		{".data", DATA},
		{".string", STRING},
		{".extern", EXTERN},
		{".entry", ENTRY},
		{"no match", ERROR}
};
/*************************** addressing format lookup table ******************/
opCode opCode_table[OPS] = {
	{"mov", {TRUE, TRUE, TRUE, TRUE}, {FALSE, TRUE, TRUE, TRUE}},
	{"cmp", {TRUE, TRUE, TRUE, TRUE}, {TRUE, TRUE, TRUE, TRUE}},
	{"add", {TRUE, TRUE, TRUE, TRUE}, {FALSE, TRUE, TRUE, TRUE}},
	{"sub", {TRUE, TRUE, TRUE, TRUE}, {FALSE, TRUE, TRUE, TRUE}},
	{"not", {FALSE, FALSE, FALSE, FALSE}, {FALSE, TRUE, TRUE, TRUE}},
	{"clr", {FALSE, FALSE, FALSE, FALSE}, {FALSE, TRUE, TRUE, TRUE}},
	{"lea", {FALSE, TRUE, TRUE, FALSE}, {FALSE, TRUE, TRUE, TRUE}},
	{"inc", {FALSE, FALSE, FALSE, FALSE}, {FALSE, TRUE, TRUE, TRUE}},
	{"dec", {FALSE, FALSE, FALSE, FALSE}, {FALSE, TRUE, TRUE, TRUE}},
	{"jmp", {FALSE, FALSE, FALSE, FALSE}, {FALSE, TRUE, TRUE, TRUE}},
	{"bne", {FALSE, FALSE, FALSE, FALSE}, {FALSE, TRUE, TRUE, TRUE}},
	{"red", {FALSE, FALSE, FALSE, FALSE}, {FALSE, TRUE, TRUE, TRUE}},
	{"prn", {FALSE, FALSE, FALSE, FALSE}, {TRUE, TRUE, TRUE, TRUE}},
	{"jsr", {FALSE, FALSE, FALSE, FALSE}, {FALSE, TRUE, TRUE, TRUE}},
	{"rts", {FALSE, FALSE, FALSE, FALSE}, {FALSE, FALSE, FALSE, FALSE}},
	{"stop", {FALSE, FALSE, FALSE, FALSE}, {FALSE, FALSE, FALSE, FALSE}},
};

char *registers[] = {"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"}; /* register names */
opWord instr[MAX_INSTR];	/* stores instruction words */
unsigned short data[MAX_DATA];	/* stores data words */
