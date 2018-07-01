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
 	DialogRec *dialogrec = (DialogRec*)calloc(1, sizeof(DialogRec));
    int i;
    
    printf("DialogRec *findDialogRec(char *cmd, DialogRec dialogspec[])\n");
    printf("command: %s\n----\n", command);
    
	for (i=0; i<CMDMAX; i++) {
		if (strlen(dialogspec[i].command)) {
			if (!(strcasecmp(command, dialogspec[i].command))) {
                printf("Gefundener DialogSpec Command: %s\n----\n", dialogspec[i].command);
                dialogrec = &dialogspec[i];
                return dialogrec;
			}
		}
	}
    printf("Return DialogRec = NULL!\n");
    dialogrec = NULL;
    return dialogrec;
}


ProlResult processLine(char line[LINEMAX], int state, DialogRec dialogspec[]) {
	ProlResult result;
	DialogRec *drecord = (DialogRec*)calloc(1, sizeof(DialogRec));
	char *cmd = (char*)calloc(CMDMAX, sizeof(char));
	char *param = line;
	char infomsg[GRMSGMAX];
	
	result.dialogrec = NULL;
	result.failed = 1;
    
    printf("processLine()\nLine: %s, state: %d\n-----\n", line,state);
	
	cmd = __strtok_r(param, " ", &param);
    
    printf("cmd strtok: %s\n",cmd);
    
    drecord = findDialogRec(line, dialogspec);
    
    if (drecord == NULL) {
        printf("drecord was NULL!\n");
		strcpy(infomsg, "The following command was not found: ");
		strcat(infomsg, cmd);
		strcpy(result.info, infomsg);
        return result;
	}
	
	if (drecord !=NULL) {
		/* Check if global state and CMD-State drecord */
        
        printf("DRecord-State: %d ÜbergebenerState: %d\n----\n", drecord->state, state);
		if (drecord->state == state) {
            printf("drecord->state == übergebener State\n");
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
	}
    
    memset(line, 0, strlen(line));
    free(cmd);
	return result;
}

void printRes(ProlResult res) {
	printf("printRes(ProlResult ):\n%d %s | Global-State: %d \n", res.failed, res.info, globalstate);	
    printf("ProlRes.DialogRec CMD: %s  PARAM: %s\n", res.dialogrec->command, res.dialogrec->param);
}
