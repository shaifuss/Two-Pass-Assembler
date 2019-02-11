/*
 * utils.c
 *
 *  Created on: Feb 14, 2017
 *      Author: Avishai Fuss 332658608
 *
 *      contains utility functions used in multiple places throughout the program
 */

#include "asm_global.h"
#include "passes.h"

#define REGS 8				/* number of registers */
/***********************************************************************************************
 * returns true if label has valid syntax
 **********************************************************************************/
bool chk_label(char * temp, char * c)
{
	int i;

	if (!isalpha(*temp) || (c - temp) > MAX_LBL) /* starts on blank or too long */
	{
		error_flag = TRUE;
		fprintf(stderr, "Line %d: Improper label syntax\n", s_count);
		return FALSE;
	}
	/* all alphanumeric? */
	for (i = 0; temp[i] != ':' && temp[i] != '\n' && temp[i] != '\0'; i++)
	{
		if (!isalpha(temp[i]) && !isdigit(temp[i]))
		{
			error_flag = TRUE;
			fprintf(stderr, "Line %d: Improper label syntax - non-alphanumeric char\n", s_count);
			return FALSE;
		}
	}
	/* matches register or op name? */
	for (i = 0; i < OPS; i++)
	{
		if (c - temp >= 3 && c - temp <= 4 && strncmp(opCode_table[i].op, temp, c - temp) == 0)
		{	/* 3 or 4 chars */
			error_flag = TRUE;
			fprintf(stderr, "Line %d: Label matches operation name\n", s_count);
			return FALSE;
		}
		if (i < REGS && strncmp(registers[i], temp, c - temp) == 0)
		{
			error_flag = TRUE;
			fprintf(stderr, "Line %d: Label matches register name\n", s_count);
			return FALSE;
		}
	}
	return TRUE;	/* label is valid */
}

/******************************************************************
 * checks for .data or .string tags
 *********************************************************/
store is_store(char * tag)
{
	int i;
	char token[10];

	if (*tag != '.') /* no period, not a storage instruction */
		return NO;
	/* load next word into array */
	else
	{	/** how many chars until space? ***/
		for (i = 0; !isspace(tag[i]); i++)
			;
		/******* match storage types **************/
		if (i > 7) {	/* number of chars in longest tag */
			return ERROR;
		}
		else
		{
			strncpy(token, tag, i);	/* copy tag to token */
			token[i] = '\0';
			/** find match for token with in menu, return value **/
			for (i = 1; menu[i].field != ERROR; i++)
			{
				if (strcmp(menu[i].tag, token) == 0)
					return menu[i].field;
			}
			/** token did not match valid tag field */
			return ERROR;
		}
		return menu[i].field;
	}
}
/****************************************************************
 * checks syntax of data values
 ************************************************************************/
bool chk_data(char * token)
{
	SKIP_WHITE(token);
	/* check that input is only digits */
	if (*token == '\0') /* leading or consecutive commas */
	{
		error_flag = TRUE;
		fprintf(stderr, "Line %d: improper data syntax: misplaced comma\n", s_count);
		return FALSE;
	}

	if (*token == '-' || *token == '+')		/* signs are valid */
	{
		token++;
		SKIP_WHITE(token);
	}

	while (!isspace(*token) && *token != '\0')
	{
		if (!isdigit(*token))
		{
			error_flag = TRUE;
			fprintf(stderr, "Line %d: improper data syntax: value is not an integer\n", s_count);
			return FALSE;
		}
		token++;
	}
	SKIP_WHITE(token);		/* whats after the spaces? */
	if (*token != '\0')
	{
		error_flag = TRUE;
		fprintf(stderr, "Line %d: improper data syntax: missing comma\n", s_count);
		return FALSE;
	}
	return TRUE;
}
/* **********************************************************************
 * breaks string of data values in tokens without replacing the delimiter
 ****************************************************************************/
char * tokenize (char * string)
{
	int count = 0;
	char *token;
	token = (char *)malloc(MAX_LBL);	/* allocate memory */
	while (*string != ',' && *string != '\n' && *string != '\0')	/*stop at comma or end of line */
	{
		*token++ = *string++;
		count++;
	}
	*token = '\0';
	return token - count;
}
/*********************************************************************
 * checks string data syntax and stores value in memory array
 *********************************************************************/
bool chk_string(char * values)
{
	if (*values++ != '\"') /* no opening quotes */
	{
		error_flag = TRUE;
		fprintf(stderr, "Line %d: improper string syntax: missing opening quotation mark\n", s_count);
		return FALSE;
	}
	while (*values != '\"')
	{
		if (*values >= ' ')  /* only visible ascii chars */
		{
			data[d_count++] = *values++;	/*store data in data array */
		}
		else {	/*invalid string char */
			error_flag = TRUE;
			fprintf(stderr, "Line %d: Improper string syntax\n", s_count);
			return FALSE;
		}
	}
	return TRUE;
}
/*****************************************
 * frees memory from label table
 **********************************************/
void free_table()
{
	sym_ptr temp, prev;
	temp = table;
	while (temp != NULL)	/*cycle through table */
	{
		prev = temp->next;
		free(temp);
		temp = prev;
	}
}
/************************************************************************************************
 * checks against lookup table, returns true if operation doesn't take addressing format
********************************************************************************************************/
bool all_false(int opCode)
{
	int i;
	for (i = 0; i < 4; i++)
	{
		if (opCode_table[opCode].addr_op1[i])
			return FALSE;
		break;
	}
	return TRUE;
}
/*************************************************************************************************
 * returns addressing format of operand
 *********************************************************************************************************/
format find_format(char * op)
{
	int i, reg;
	char * temp, op_reg[OPS];
	SKIP_WHITE(op);
	TRIM_WHITE(op); /* remove trailing whitespace */

	if (*op == '#') /* leading immediate character */
	{
		strtol(op + 1, &temp, 10); /* can we convert chars to long (are they numeric)? */
		if (*temp == '\0')
			return IMM;
		else /* immediate value was not numeric */
		{
			error_flag = TRUE;
			fprintf(stderr, "Line %d: Non-numeric immediate value\n", s_count);
			return WRONG;
		}
	}
	else if (*op == 'r') /* check against reg syntax but not against label */
	{
		if ((temp = strchr(op, '[')) == NULL && atoi(op + 1) < REGS) /* not offset type */
		{
			for (i = 0; i < REGS; i++) /* match name of register */
			{
				if (strcmp(op, registers[i]) == 0)
					return REG;
			}
			error_flag = TRUE;		/* no match */
			fprintf(stderr, "Line %d: Invalid register name\n", s_count);
			return WRONG;
		}
		else if (temp != NULL)/* check for offset type */
		{
			temp++; /* move past first bracket, copy second operand register */
			for (i = 0; temp[i] != ']' && temp[i] != '\0'; i++)
				op_reg[i] = temp[i];
			op_reg[i] = '\0';

			if (temp[i] == '\0')
			{
				error_flag = TRUE;
				fprintf(stderr, "Line %d: Missing closing bracket\n", s_count);
				return WRONG;
			}

			temp += i + 1;		/* move past last operand */
			SKIP_WHITE(temp);

			if (*temp != '\0')
			{
				error_flag = TRUE;
				fprintf(stderr, "Line %d: Values after closing bracket\n", s_count);
				return WRONG;
			}

			if ((reg = atoi(op + 1)) > REGS - 1 || reg % 2 != 1) /* first reg must be 0 - 7 and odd */
			{
				error_flag = TRUE;
				fprintf(stderr, "Line %d: Invalid register (1st) for offset type\n", s_count);
				return WRONG;
			}

			temp = op_reg; /* point at second register */
			SKIP_WHITE(temp);
			if (*temp != 'r' || (reg = atoi(temp + 1)) > REGS - 1 || reg % 2 != 0) /* 2ns reg must be 0 - 7 and even */
			{
				error_flag = TRUE;
				fprintf(stderr, "Line %d: Invalid register (2nd) for offset type\n", s_count);
				return WRONG;
			}
			return OFFSET;
		}
	}
	if (chk_label(op, strchr(op, '\0'))) /* check for valid label */
		return LABEL;
	else
		return WRONG;
}
/*********************************************************************************
 * navigate operation and operand string, delegate to appropriate functions
 ***************************************************************************************/
void parse_op(char * instr, bool first_pass)
{
	char *comma, *first_op, *second_op;
	char operation[MAX_LBL];
	int i;
	bool one_op;	/* true if operation only takes 1 operand */

	/* isolate operation name */
	SKIP_WHITE(instr);
	for (i = 0; !isspace(instr[i]) && instr[i] != '\0'; i++)
		;
	strncpy(operation, instr, i);
	operation[i] = '\0';

	/* increment string past op name */
	instr += i;
	SKIP_WHITE(instr);

	for (i = 0; i < OPS; i++)	/* search for op in lookup table */
	{
		if (strcmp(opCode_table[i].op, operation) == 0)
		{
			break;
		}
	}
	if (i == OPS)	/*not found */
	{
		error_flag = TRUE;
		if (first_pass == TRUE)
			fprintf(stderr, "Line %d: Invalid operation\n", s_count);
		return;
	}

	if (strcmp(opCode_table[i].op, "rts") == 0 ||	/*ops that dont have operands */
			(strcmp(opCode_table[i].op, "stop") == 0))
	{
		if (*instr != '\0') /* text after ops that don't take operands */
		{
			error_flag = TRUE;
			if (first_pass == TRUE)
				fprintf(stderr, "Line %d: Operation does't take operands\n", s_count);
			return;
		}
		/* call word creation function */
		if (first_pass == TRUE)
			add_code(i, NULL, NULL);
		else /*second pass */
			fill_code(i, NULL, NULL);
		return;
	}

	one_op = all_false(i);

	comma = strchr(instr, ','); /* find first delimiter? */
	TRIM_WHITE(instr);

	if (one_op == TRUE)		/*only takes one operand */
	{
		if (comma != NULL)
		{
			error_flag = TRUE;
			if (first_pass == TRUE)
				fprintf(stderr, "Line %d: Operation should only have one operand\n", s_count);
		}
		/* call word creator */
		if (first_pass == TRUE)
			add_code(i, NULL, instr);
		else
			fill_code(i, NULL, instr);
		return;
	}
	else /* if one_op == FALSE; */
	{
		if (comma == NULL) /* no delimiters */
		{
			error_flag = TRUE;
			if (first_pass == TRUE)
				fprintf(stderr, "Line %d: Operation should have two operands\n", s_count);
			return;
		}
		else if (strrchr(instr, ',') != comma)		/* more than one delimiter means more than two operands */
		{
			error_flag = TRUE;
			if (first_pass == TRUE)
				fprintf(stderr, "Line %d: Operation should only have two operands\n", s_count);
			return;
		}
		/* call word creator */
		first_op = strtok(instr, ",");
		second_op = strtok(NULL, ",");
		if (first_pass == TRUE)
			add_code(i, first_op, second_op);
		else
			fill_code(i, first_op, second_op);
	}
}
/*******************************************************
 * returns true if line is blank
 ******************************************************/
bool blank(char * line)
{
	SKIP_WHITE(line);
	return (*line == '\0' || *line == EOF) ? TRUE : FALSE;
}

/**************************************************************
 * frees extern table
 ************************************************************/
void free_ext()
{
	ext_table *temp, *prev;
	temp = ext_head;
	while (temp != NULL)
	{
		prev = temp->next;
		free(temp);
		temp = prev;
	}
}
