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

/* Aufgabe 2 
ProlResult processLine(char line[LINEMAX], int state, DialogRec dialogspec[]) {
	
} */


int main(void) {
	DialogRec *match;
	
	match = findDialogRec("toast forever", dialog);
	if (match !=NULL) {
			printf("Command: [ %s ] Param: [ %s ]\n",match->command, match->param);
	}

	
	return 0;
}
