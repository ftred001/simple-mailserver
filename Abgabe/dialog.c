#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include "dialog.h"

int globalstate = 0;



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
    
 
    /*printf("DialogRec *findDialogRec(char *cmd, DialogRec dialogspec[])\n");*/
    /* printf("findDialogRec: %s\n", command);*/
    
	for (i=0; i<CMDMAX; i++) {
		if (strlen(dialogspec[i].command)) {
			if (!(strncasecmp(command, dialogspec[i].command, strlen(dialogspec[i].command)))) {

                /* printf("Gefundener DialogSpec Command: %s\n----\n", dialogspec[i].command);*/
                command += strlen(dialogspec[i].command);
                
                /* printf("Rest Param: %s\n", command);*/

                dialogrec = &dialogspec[i];
                strcpy(dialogrec->param, command);
                printf("drecord.command: %s param: X%sX\n", dialogrec->command, dialogrec->param);
                
                return dialogrec;
			}
		}
	}
    /* printf("Return DialogRec = NULL!\n"); */
    dialogrec = NULL;
    return dialogrec;
}


ProlResult processLine(char line[LINEMAX], int state, DialogRec dialogspec[]) {
	ProlResult result;
	DialogRec *drecord = (DialogRec*)calloc(1, sizeof(DialogRec));
	char infomsg[GRMSGMAX];
	
	result.dialogrec = NULL;
	result.failed = 1;
    /* printf("processLine()\nLine: %s, state: %d\n-----\n", line,state); */
    drecord = findDialogRec(line, dialogspec);
    
    
    if (drecord == NULL) {
        /* printf("drecord was NULL!\n"); */
		strcpy(infomsg, "The following command was not found: ");
		strcat(infomsg, line);
		strcpy(result.info, infomsg);
		result.failed = 1;
        return result;
	}
	
	if (drecord !=NULL) {
		/* Check if global state and CMD-State drecord */
        
        /* printf("DRecord-State: %d ÜbergebenerState: %d\n----\n", drecord->state, state); */
		if (drecord->state == state) {
            /* printf("drecord->state == übergebener State\n"); */
			
			/* Validate params */
            /* Lineprocessing successful */
            drecord->is_valid = validator(drecord);
            result.dialogrec = drecord;
            globalstate = drecord->nextstate;
            
		} else {
			strcpy(result.info, "The global state did not match the command-state.");
		}
	}
    
    if (!drecord->is_valid) {
        strcpy(result.info, "Validation failed\n");
        result.failed = 1;
    } else {
        strcpy(result.info, "OK");
        result.failed = 0;
    }
    
    
    memset(line, 0, strlen(line));
	return result;
}

void printRes(ProlResult res) {
	printf("printRes(ProlResult):\n%d %s | Global-State: %d \n", res.failed, res.info, globalstate);	
    printf("ProlRes.DialogRec CMD: %s  PARAM: %s\n----\n", res.dialogrec->command, res.dialogrec->param);
}
