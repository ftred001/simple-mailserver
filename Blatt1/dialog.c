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

int validator(DialogRec *d) {
	if (d->validator!=NULL) {
		return d->validator(d);	
	}
	return 1;
}

DialogRec dialog[] = {
	/* Command,			Param, 	State,	Next-State,	Validator */
	{ "BEGIN", 			"",		0,		1,			validate_noparam },
	{ "kuehlschrank",	"",		1,		1,			validate_onoff },
	{ "fernseher", 		"", 	1,		1,			validate_onoff },
	{ "toaster", 		"", 	1,		1},
	{ "END", 			"", 	1,		2,			validate_noparam },
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

int main(void) {
	ProlResult res;
	char line[LINEMAX] = "BEGIN";
	char line2[LINEMAX] = "Toaster forever";
	char line3[LINEMAX] = "fernseher on";
	char line4[LINEMAX] = "END";
	char line5[LINEMAX] = "BEGIN";

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
	
	return 0;
}
