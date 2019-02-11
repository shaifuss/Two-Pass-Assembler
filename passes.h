/*
 * passes.h
 *
 *  Created on: Mar 28, 2017
 *      Author: shai
 */

#ifndef PASSES_H_
#define PASSES_H_

#define OPS 16				/* number of instructions in ISA */
#define MAX_INSTR 10000 	/* max instructions per machine code file */
#define MAX_DATA 1500 	/* max instructions per machine code file */

typedef enum { SRC, DEST } operand;		/* source, destination */
typedef enum { NO, DATA, STRING, EXTERN, ENTRY, ERROR } store;

/*immediate, label, sum of two registers, direct register, syntax error */
typedef enum { IMM, LABEL, OFFSET, REG, WRONG } format;
typedef enum { ABS, EXTRN, RELOC } link; /*absolute, external, relocatable */

/******************** symbol table entry for address lookup *************************/
/** stored as linked list **/
typedef struct symbol {
	char label[MAX_LBL];
	unsigned short address;		/* address where label points to */
	link type;
	store place;
	struct symbol *next;
}sym;

/************************** extern table *******************************************/
typedef struct ext_table {
	char label[MAX_LBL];
	unsigned short address;		/* address where extern is used in current program */
	struct ext_table *next;
}ext_table;

typedef struct symbol *sym_ptr;

/*********************** data tag lookup table *************************************/
typedef struct tag_menu {
	char *tag;			/* tag string (such as .data or .entry) */
	store field;
} tag_menu;

/*********************** addressing format lookup table *****************************/
typedef struct opCode {
	char* op;   	/* operation name */
	bool addr_op1[4];	/* first operand address format lookup table */
	bool addr_op2[4];	/* second operand address format lookup table */
} opCode;


/****************************** first instruction word format *******************/
typedef union opWord
{
	struct
	{
		unsigned int ARE : 2;
		unsigned int opDest : 2;
		unsigned int opSource : 2;
		unsigned int opCode : 4;
		unsigned int group : 2;
		unsigned int unused : 3;
	} word;
	unsigned short fullReg;
} opWord;

/****************************** extra instruction word(s) format *******************/
typedef union extra_op_word
{
	struct
	{
		unsigned int ARE : 2;
		unsigned int value : 13;
		unsigned int unused : 1;
	} imm_lab;	/* immediate or label */
	struct
	{
		unsigned int ARE : 2;
		unsigned int dest : 6;
		unsigned int src : 6;
		unsigned int unused : 1;
	} regs;
	unsigned short fullReg;
} extra_op_word;

/************************* global variable external declarations ******************/
extern sym_ptr table;
extern ext_table *ext_head;
extern bool has_label;
extern bool has_entry;
extern tag_menu menu[];
extern unsigned short data[MAX_DATA];
extern opWord instr[MAX_INSTR];
extern opCode opCode_table[];
extern char *registers[];

/*********************************function prototypes ****************************/
bool blank(char * line);
void add_sym(char * label, store data_tag);
store is_store(char * tag);
bool chk_label(char * temp, char * c);
void add_mem(char *temp, store data_tag);
bool chk_data(char * token);
char * tokenize (char * string);
bool chk_string(char * values);
bool all_false(int opCode);
void parse_op(char * instr, bool first_pass);
void add_code(int opCode, char * first, char * second);
format find_format(char * op);
void flag_entry(char * label);
void fill_code(int opCode, char * source, char * dest);

#endif /* PASSES_H_ */
