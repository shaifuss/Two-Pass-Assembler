/*
 * second_pass.c
 *
 *  Created on: Feb 14, 2017
 *      Author: Avishai Fuss 332658608
 *
 *      Contains functions involved in second pass over source code
 */
#include "asm_global.h"
#include "passes.h"


/*************************************************************
 * outputs data to files
 *************************************************************/
void files_out(char * arg)
{
	char filename[MAX_LBL];
	FILE *fp;
	int i;
	sym_ptr temp;
	ext_table *trav;

	/*open new object file */
	strcpy(filename, arg);
	strcat(filename, ".ob");
	fp = fopen(filename, "w");

	fprintf(fp, "%X %X\n", i_count, d_count); /* print lengths */

	for (i = 0; i < i_count; i++)	/* print instructions */
		fprintf(fp, "%X %04hX\n", i + CODE_OFFSET, instr[i].fullReg);
	for (i = 0 ; i < d_count; i++)			/* print data */
		fprintf(fp, "%X %04hX\n", i + i_count + CODE_OFFSET, data[i]);
	fclose(fp);

	if (has_entry)	/* file contains .entry */
	{
		strcpy(filename, arg);
		strcat(filename, ".ent");
		fp = fopen(filename, "w");	/* write to .ent file */

		temp = table;
		while (temp != NULL)		/* find entries in label table */
		{
			if (temp->place == ENTRY)
				fprintf(fp, "%-30s %X\n", temp->label, temp->address);
			temp = temp->next;
		}
		fclose(fp);
	}
	if (ext_head != NULL)			/* file contains extern */
	{
		strcpy(filename, arg);
		strcat(filename, ".ext");
		fp = fopen(filename, "w");	/* open .ext file */

		trav = ext_head;
		while (trav != NULL)		/* print entire extern table */
		{
			fprintf(fp, "%-30s %X\n", trav->label, trav->address);
			trav = trav->next;
		}
		fclose(fp);
	}
}
/*************************************************************
 * flags label as entry (can be made generic???)
 *************************************************************/
void flag_entry(char * label)
{
	sym_ptr temp;

	while (!isspace(*label)) /* skip over .entry tag */
		label++;
	SKIP_WHITE(label);

	/* trim trailing whitespace */
	TRIM_WHITE(label);

	if (*label == '\0')
	{
		error_flag = TRUE;	/* no match in lookup table */
		fprintf(stderr, "Line %d: No label listed after .entry\n", s_count);
		return;
	}
	temp = table;
	while (temp != NULL)
	{
		if (strcmp(label, temp->label) == 0) /* found the matching label */
		{
			if (temp->type == EXTRN) {
				error_flag = TRUE;
				fprintf(stderr, "Line %d: Label can't be extern and entry\n", s_count);
				return;
			}
			temp->place = ENTRY;
			return;
		}
		temp = temp->next;
	}
	error_flag = TRUE;	/* no match in lookup table */
	fprintf(stderr, "Line %d: entry label not in file\n", s_count);
	return;
}
/****************************************************************************
 * add values to extern table
 **************************************************************************/
void add_ext(char * label, int address)
{
	ext_table *node, *temp, *prev;

	node = (ext_table *)malloc(sizeof(ext_table));	/* allocate memory for extern */
	if (node == NULL) {
			fprintf(stderr, "Out of Memory");
			exit(1);
	}
	strcpy(node->label, label);
	node->address = address;
	node->next = NULL;
	/* add to extern table */
	if(ext_head == NULL)	/* list empty, node becomes head */
	{
		ext_head = node;
		return;
	}
	else
	{
		temp = ext_head;
		while (temp != NULL)	/* move to end */
		{
			prev = temp;
			temp = temp->next;
		}
		prev->next = node;		/* add extern to end of list */
		return;
	}
}
/*****************************************************************************
 * stores extra instruction words in array
 ****************************************************************************/
void add_words(char * operand, format addr, bool src)
{
	extra_op_word extra;
	sym_ptr trav;
	char * c, buf[MAX_LBL];

	extra.fullReg = 0;	/* zero out garbage values */
	SKIP_WHITE(operand);
	/* look at type of addr for opDest */
	switch (addr)
	{
		case LABEL :  	/* label address format */
			trav = table;
			while (trav != NULL)
			{
				if (strcmp(operand, trav->label) == 0) /* find in label table */
				{
					if (trav->type == EXTRN)	/* add to extern table */
						add_ext(trav->label, i_count + CODE_OFFSET);
					/* adjust values according to info */
					extra.imm_lab.ARE = trav->type;
					extra.imm_lab.value = trav->address;
					instr[i_count++].fullReg = extra.fullReg;	/* store in instruction array */
					return;
				}
				trav = trav->next;
			}
			if (trav == NULL) /* unrecognized label used */
			{
				error_flag = TRUE;
				fprintf(stderr, "Line %d: Label not found\n", s_count);
				i_count++;
				return;
			}
			break;
		case IMM : /* immediate address format - store info in word */
			extra.imm_lab.ARE = ABS;
			extra.imm_lab.value = atoi(operand + 1); 	/* skip # tag */
			extra.imm_lab.unused = 0;
			instr[i_count++].fullReg = extra.fullReg;	/*store in array */
			break;
		case REG : /* direct register format */
			extra.regs.ARE = ABS;
			if (src == FALSE)	/* store in proper slot */
				extra.regs.dest = atoi(operand + 1);			/* skip r */
			else /* src == TRUE */
				extra.regs.src = atoi(operand + 1);
			instr[i_count++].fullReg = extra.fullReg;
			break;
		case OFFSET : /* register combo format eg rx[rx] */
			extra.regs.ARE = ABS;
			/* load registers into word */
			c = strchr(operand, '[');
			strncpy(buf, operand, (c - operand));
			extra.regs.dest = atoi(buf + 1);
			c++; SKIP_WHITE(c); /*move c past open bracket */
			extra.regs.src = atoi(c + 1);
			instr[i_count++].fullReg = extra.fullReg;
			return;
		case WRONG :
			break;
	}
}
/*************************************************************************
 * fills in second and third (if necessary) word(s) of code
 *************************************************************************/
void fill_code(int opCode, char * source, char * dest)
{
	extra_op_word two;
	opWord one; 				/* first word of machine code */

	two.fullReg = 0;	/* discard garbage values */

	one = instr[i_count++];		/* first word of instruction */
	if (dest) {
		SKIP_WHITE(dest); TRIM_WHITE(dest);
	}
	if (one.word.group == 0)	/*op without operands */
		return;
	else if (one.word.group == 1)	/* contains one operand */
	{
		add_words(dest, one.word.opDest, FALSE);
		return;
	}
	else if (one.word.group == 2)	/* contains two operands */
	{
		if (one.word.opDest == REG && one.word.opSource == REG)
		{
			two.regs.src = atoi(source + 1);
			two.regs.dest = atoi(dest + 1);
			instr[i_count++].fullReg = two.fullReg;
			return;
		}
		else
		{
			add_words(source, one.word.opSource, TRUE);
			add_words(dest, one.word.opDest, FALSE);
			return;
		}
	}
}
/*********************************************************************
 * second pass through file according to algorithm given in specification
 *********************************************************************/
void second_pass(FILE *fp)
{
	char* c /* char index */;
	char *temp, buf[MAX_CODE];
	store data_tag;

	while (fgets(buf, MAX_CODE, fp) != NULL)
	{
		/*************** set flags, counters ********/
		s_count++;  /* increment assembly code counter */
		has_label = FALSE;
		temp = buf;

		if (*temp == '\n' || *temp == ';' || blank(temp))	/*skip comments or blank lines */
			continue;

		if ((c = strchr(temp, ':')) != NULL)	/* skip over labels */
			temp = ++c;
		SKIP_WHITE(temp);

		switch (data_tag = is_store(temp)) {
			case ENTRY :
				flag_entry(temp); /* flag in label table */
				break;
			case NO :	/* regular instruction */
				parse_op(temp, FALSE);
				/* no break */
			case EXTERN : case DATA : case STRING : case ERROR :	/* skip data lines */
				break;
		}
	}
}

