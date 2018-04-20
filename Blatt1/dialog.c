#include <stdio.h>
#include <string.h>
#include <strings.h>
#include "dialog.h"

int globalstate = 0;

int validate_noparam(DialogRec *d) {
	return !d->param;
}

int validate_onoff(DialogRec *d) {
	return !strcmp(d->param, "on") || !strcmp(d->param, "off"); 
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
	
	token = __strtok_r(param, divider, &param);
	printf("Command: %s Param: %s\n", token, param);
	
	drecord = findDialogRec(token, dialogspec);
	if (drecord !=NULL) {
		/* Check if global state and CMD-State drecord */
		if (drecord->state == state) {
			strcpy(drecord->param, param);
			
		} else {
			result.failed = '1';
			result.dialogrec = NULL;
			strcpy(result.info, "The global state did not drecord the command-state");
		}
		
		
		
		
		/* Ändere State */
		globalstate = drecord->nextstate;
		result.failed = '0';
		strcpy(result.info, "Command was found");
		printf("Command: [ %s ] Param: [ %s ]\n",drecord->command, drecord->param);
	} else {
		result.failed = '1';
		result.dialogrec = NULL;
		strcpy(infomsg, "The following command was not found: ");
		strcat(infomsg, token);
		strcpy(result.info, infomsg);
	}
	

	return result;
}


int main(void) {
	ProlResult res;
	char line[LINEMAX] = "BEGIN";
	char line2[LINEMAX] = "Toaster forever";

	res = processLine(line, globalstate, dialog);
	printf("Status: %c Info: %s State: %d \n", res.failed, res.info, globalstate);
	res = processLine(line2, globalstate, dialog);
	printf("Status: %c Info: %s State: %d \n", res.failed, res.info, globalstate);
	
	return 0;
}
