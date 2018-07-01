#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include "dialog.h"

int globalstate = 0;

int validate_noparam(DialogRec *d) {
    if(strlen(d->param) == 0) {
		return 1;
	} else {
		return 0;
	}
}

int validate_hasparam(DialogRec *d) {
	return strlen(d->param);
}

int validator(DialogRec *d) {
	if (d->validator!=NULL) {
		return d->validator(d);	
	}
	return 1;
}


/* Returns DialogRec if found. Else NULL */
DialogRec *findDialogRec(char *command, DialogRec dialogspec[]) {
 	DialogRec *dialogrec = NULL;
    int i;
    
	for (i=0; i<CMDMAX; i++) {
		if (strlen(dialogspec[i].command)) {
			if (!(strcasecmp(command, dialogspec[i].command))) {
                dialogrec = &dialogspec[i];
				break;
			}
		}
	}
	return dialogrec;
}


ProlResult processLine(char line[LINEMAX], int state, DialogRec dialogspec[]) {
	ProlResult result;
	DialogRec *drecord;
	char *divider = " ";
	char *cmd;
	char *param = line;
	char infomsg[GRMSGMAX];
	
	result.dialogrec = NULL;
	result.failed = 1;
	
	cmd = __strtok_r(param, divider, &param);
    drecord = findDialogRec(line, dialogspec);
	
	if (drecord !=NULL) {
		/* Check if global state and CMD-State drecord */
		if (drecord->state == state) {
			strcpy(drecord->param, param);
			
			/* Validate params */
			if (drecord->validator != NULL) {
				/* Lineprocessing successful */
				drecord->is_valid = validator(drecord);
				result.failed = 0;
				strcpy(result.info, "OK");
				result.dialogrec = drecord;
				globalstate = drecord->nextstate;
			} else {
				drecord->is_valid = 1;
				strcpy(result.info, "Validation failed\n");
			}
		} else {
			strcpy(result.info, "The global state did not match the command-state.");
		}
	} else {
		strcpy(infomsg, "The following command was not found: ");
		strcat(infomsg, cmd);
		strcpy(result.info, infomsg);
	}
    memset(line, 0, strlen(line));
	return result;
}

void printRes(ProlResult res) {
	printf("%d %s | Global-State: %d \n", res.failed, res.info, globalstate);	
    printf("ProlRes.DialogRec CMD: %s  PARAM: %s\n", res.dialogrec->command, res.dialogrec->param);
}
