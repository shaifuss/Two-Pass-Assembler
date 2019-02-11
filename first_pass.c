/*
 * first_pass.c
 *
 *  Created on: Feb 14, 2017
 *      Author: Avishai Fuss 332658608
 *
 *      Contains functions involved in first pass over source code
 */
#include "asm_global.h"
#include "passes.h"

#define UNUSED 7		/* code for unused bits - 111*/
#define MASK 32767			/* 15-LSB mask */

/******************************************************************
 * adds label to symbol table, stored as linked list in order of IC
 *********************************************************/
void add_sym(char * label, store data_tag) {
	sym_ptr node, trav, prev;

	node = (sym_ptr)malloc(sizeof(sym));	/* allocate space for label */
	if (node == NULL) {
		fprintf(stderr, "Out of Memory");
		exit(1);
	}
	if (data_tag == STRING || data_tag == DATA)	/* handle address according to tag */
	{
		strcpy(node->label, label);
		node->address = i_count + d_count + CODE_OFFSET;
		node->type = RELOC;
	}
	else if (data_tag == EXTERN)
	{
		strcpy(node->label, label);
		node->address = 0;			/* non-local address */
		node->type = EXTRN;
	}
	else /* if (data_tag == NO) */
	{
		strcpy(node->label, label);
		node->address = i_count + CODE_OFFSET;
		node->type = ABS;
	}
	node->next = NULL;

	if (table == NULL) {		/* symbol table is empty, node becomes head */
		table = node;
	}
	else {
		trav = table;
		while (trav != NULL) /* check if label already exists */
		{
			if (strcmp(trav->label, node->label) == 0)
			{
				error_flag = TRUE;
				fprintf(stderr, "Line %d: Label already exists\n", s_count);
			}
			prev = trav;
			trav = trav->next;
		}
		prev->next = node; 	/* add node to back of list */
	}
}

/*********************************************************
 * configures instruction machine code
 *******************************************************/
void add_code(int opCode, char * source, char * dest)
				/* TEST WITH ALL COMBOS */
{
	format addr;	/* address format */
	opWord one; /* first word of machine code */

	/* operand-format-independent fields */
	one.word.ARE = ABS;		/* absolute */
	one.word.opCode = opCode;
	one.word.unused = UNUSED;
	if (source == NULL)		/* less than 2 operands */
	{
		if (dest == NULL)	/* no operands */
		{
			one.word.opDest = 0;
			one.word.opSource = 0;
			one.word.group = 0;

			instr[i_count++].fullReg = one.fullReg;
			return;
		}
		else /* if dest != NULL ie 1 operand*/
		{
			if ((addr = find_format(dest)) != WRONG)  /* valid addressing format */
			{
				if (opCode_table[opCode].addr_op2[addr] == FALSE)
				{
					error_flag = TRUE;
					fprintf(stderr, "Line %d: Operation doesn't support address format\n", s_count);
				}
				one.word.opDest = addr;
			}
			one.word.opSource = 0;
			one.word.group = 1;		/* single operand */

			instr[i_count++].fullReg = one.fullReg;
			i_count++;		/* space for extra op word in all cases*/
			return;
		}
	}
	else /* if source != NULL ie 2 operands*/
	{
		if ((addr = find_format(source)) != WRONG) /* valid addressing format */
		{
			if (opCode_table[opCode].addr_op1[addr] == FALSE)
			{
				error_flag = TRUE;
				fprintf(stderr, "Line %d: Operation doesn't support address format\n", s_count);
			}
			one.word.opSource = addr;
		}
		if ((addr = find_format(dest)) != WRONG) /* format is valid */
		{
			if (opCode_table[opCode].addr_op2[addr] == FALSE) /* no good for this op */
			{
				error_flag = TRUE;
				fprintf(stderr, "Line %d: Operation doesn't support address format\n", s_count);
			}
			one.word.opDest = addr;
		}
		one.word.group = 2;		/* two operands */

		instr[i_count++].fullReg = one.fullReg;
		/* save space for extra words */
		if (one.word.opDest == REG && one.word.opSource == REG)
			i_count++;		/*two reg operands == only 1 extra word */
		else
			i_count += 2;	/* all other cases */
	}
	return;
}


/*************************************************************
 * add data to memory array
 *************************************************************/
void add_mem(char *values, store data_tag)
{
	char *token;
	while (!isspace(*values)) /* skip over tag */
		values++;
	SKIP_WHITE(values);

	switch (data_tag) {
		case DATA :
			/*break each value into tokens*/
			token = tokenize(values);

			while (*values != '\n' && *values != '\0' && chk_data(token))
			{
				data[d_count++] = (atoi(token) & MASK);	/* store 15 LSBs */
				while (*values != ',' && *values != '\n' && *values != '\0') /*move past previous value */
					values++;
				if (*values == ',') values++;
				free(token);
				token = tokenize(values);
			}
			if (token)
				free(token);
			break;

		case STRING :
			chk_string(values); /*checks syntax and stores chars in memory array */
			data[d_count++] = '\0';
			/* no break */
		default :
			break;
	}
}
/************************************************************************
 * first pass through file, line by line, following algorithm from specification
 ************************************************************************/
void first_pass(FILE *fp)
{
	char* c /* char index */;
	char *temp, buf[MAX_CODE + 1], label[MAX_LBL];
	store data_tag;

	/* zero out counters and flags */
	error_flag = has_entry = FALSE;
	i_count = d_count = s_count = 0;
	table = NULL;
	ext_head = NULL;
	/* load lines into buffer */
	while (fgets(buf, MAX_CODE, fp) != NULL)
	{
		/*************** set flags, counters ********/
		s_count++;  /* increment assembly code counter */
		has_label = FALSE;
		buf[MAX_CODE] = '\0'; /* null terminate string */
		temp = buf;

		/*skip comments or blank lines */
		if (*temp == ';' || blank(temp))
			continue;

		/***********  check for valid label, syntax ***********/
		if ((c = strchr(temp, ':')) != NULL && (has_label = chk_label(temp, c)) == TRUE)
		{
			/* prepare label to be copied into label array */
			memset(label, 0, MAX_LBL);
			strncpy(label, temp, c++ - temp);
			temp += c - temp;	/* move pointer past colon */
		}
		else if (c != NULL && has_label == FALSE)
				temp = c + 1; /* move past bad label, continue with rest of command */

		SKIP_WHITE(temp);
		/**** check for data tag (.data, .string, .entry, .extern) ****/
		switch ((data_tag = is_store(temp))) {

			case STRING : case DATA :
				if (has_label == TRUE)
					add_sym(label, data_tag);
				/* add data to memory */
				add_mem(temp, data_tag);
				continue; /* goto next line */

			case EXTERN :
				while (!isspace(*temp)) /* skip over .extern tag */
					temp++;
				SKIP_WHITE(temp);
				TRIM_WHITE(temp);	/* remove trailing whitespace */

				if (*temp == '\0')
				{
					error_flag = TRUE;
					fprintf(stderr, "Line %d: No label listed after .extern\n", s_count);
					continue;
				}
				/* check label after tag */
				if ((c = strchr(temp, '\0')) != NULL && (has_label = chk_label(temp, c)) == TRUE)
					add_sym(temp, data_tag);
				continue; /* goto next line */

			case ENTRY :
				has_entry = TRUE;
				if (has_label)
					fprintf(stderr, "Line %d: Warning - label before .entry has no meaning.\n", s_count);
				continue; /* goto next line */

			case NO :
				if (has_label == TRUE)	/* add label to table */
					add_sym(label, data_tag);

				/*check instruction */
				parse_op(temp, TRUE);
				continue;

			case ERROR :
				error_flag = TRUE;
				fprintf(stderr, "Line %d: invalid tag\n", s_count);
				break;
		}
	}
	return;
}

