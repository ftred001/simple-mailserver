#include <stdio.h>
#include <string.h>
#include <strings.h>
#include "dialog.h"

int globalstate = 0;

int validate_noparam(DialogRec *d) {
	return !!d->param;
}

int validate_onoff(DialogRec *d) {
	return !strcmp(d->param, "on") || !strcmp(d->param, "off"); 
}

int validate_username(DialogRec *d) {
	/* TODO: IMPLEMENT! */
	return !!d->param;
}

int validate_password(DialogRec *d) {
	/* TODO: IMPLEMENT! */
	return !!d->param;
}

int validate_index(DialogRec *d) {
	/* TODO: IMPLEMENT! */
	return !!d->param;
}

int validator(DialogRec *d) {
	if (d->validator!=NULL) {
		return d->validator(d);	
	}
	return 1;
}

DialogRec dialog[] = {
	/* Command,		Param, 	State,	Next-State,	Validator */
	{ "user", 		"",		0,		0,			validate_username },
	{ "pass",		"",		0,		1,			validate_password },
	{ "stat", 		"", 	1,		1,			validate_noparam },
	{ "list", 		"", 	1,		1,			},
	{ "list", 		"", 	1,		2,			validate_noparam },
	{ "retr", 		"",		1,		1,			validate_index},
	{ "QUIT",		"",		1,		2,			validate_noparam },
	{ "" }
};

/* Returns DialogRec if found. Else NULL */
DialogRec *findDialogRec(char *command, DialogRec dialogspec[]) {
 	int i;
	for (i=0; i<CMDMAX; i++) {
		if (strlen(dialogspec[i].command)) {
			if (!(strcasecmp(command, dialogspec[i].command))) {
				return &dialogspec[i];
			}
		}
	}
	return NULL;
}


ProlResult processLine(char line[LINEMAX], int state, DialogRec dialogspec[]) {
	ProlResult result;
	DialogRec *drecord;
	char *divider = " ";
	char *token;
	char *param = line;
	char infomsg[GRMSGMAX];
	
	result.dialogrec = NULL;
	result.failed = 1;
	
	token = __strtok_r(param, divider, &param);
	
	drecord = findDialogRec(token, dialogspec);
	if (drecord !=NULL) {
		/* Check if global state and CMD-State drecord */
		if (drecord->state == state) {
			strcpy(drecord->param, param);
			
			/* Validate params */
			if (validator(drecord)) {
				/* Lineprocessing successful */
				drecord->is_valid = 1;
				result.failed = 0;
				strcpy(result.info, "OK");
				result.dialogrec = dialogspec;
				globalstate = drecord->nextstate;
			} else {
				drecord->is_valid = 0;
				strcpy(result.info, "Validation failed\n");
			}
		} else {
			strcpy(result.info, "The global state did not match the command-state.");
		}
	} else {
		strcpy(infomsg, "The following command was not found: ");
		strcat(infomsg, token);
		strcpy(result.info, infomsg);
	}
	return result;
}

void printRes(ProlResult res) {
	printf("%d %s | Global-State: %d \n", res.failed, res.info, globalstate);	
}

/*
int main(void) {
	ProlResult res;
	char line[LINEMAX] = "user joendhard";
	char line2[LINEMAX] = "password joendhard";
	char line3[LINEMAX] = "stat";
	char line4[LINEMAX] = "list 1";
	char line5[LINEMAX] = "list";
	char line6[LINEMAX] = "retr 1";
	char line7[LINEMAX] = "QUIT";

	res = processLine(line, globalstate, dialog);
	printRes(res);
	res = processLine(line2, globalstate, dialog);
	printRes(res);
	
	res = processLine(line3, globalstate, dialog);
	printRes(res);
	res = processLine(line4, globalstate, dialog);
	printRes(res);
	res = processLine(line5, globalstate, dialog);
	printRes(res);
	res = processLine(line6, globalstate, dialog);
	printRes(res);
	res = processLine(line7, globalstate, dialog);
	printRes(res);
	
	return 0;
}
*/
