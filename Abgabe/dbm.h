#ifndef DBM_H
#define DBM_H

enum { STD_FILEPATH="serversettings.cfg" };

int match_filter(DBRecord *rec, const void *data);
int key_filter(DBRecord *rec, const void *data);
int cat_filter(DBRecord *rec, const void *data);
int execute_cmd(const char *filepath, int argc, char *line);


#endif

