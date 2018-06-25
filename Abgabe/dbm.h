#ifndef DBM_H
#define DBM_H

int match_filter(DBRecord *rec, const void *data);
int key_filter(DBRecord *rec, const void *data);
int cat_filter(DBRecord *rec, const void *data);


#endif

