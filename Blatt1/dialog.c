#include <stdio.h>
#include <string.h>
#include <strings.h>
#include "dialog.h"

int validate_noparam() {
	return 0;
}

int validate_onoff() {
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
	DialogRec *match;
	char *divider = " ";
	char *token;
	char *param = line;
	
	token = __strtok_r(param, divider, &param);
	printf("Command: %s Param: %s\n", token, param);
	match = findDialogRec(token, dialogspec);
	if (match !=NULL) {
		result.failed = '0';
		strcpy(result.info, "Command was found");
		printf("Command: [ %s ] Param: [ %s ]\n",match->command, match->param);
	} else {
		result.failed = '1';
		result.dialogrec = NULL;
		strcpy(result.info, "Command was not found");
	}
	printf("Result ist: %c %s\n", result.failed, result.info);
	return result;
}


int main(void) {
	char line[LINEMAX] = "toaster forever";

	processLine(line, 0, dialog);

	
	return 0;
}
