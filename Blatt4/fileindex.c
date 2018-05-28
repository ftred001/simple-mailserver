#include <stdio.h>



FileIndex *fi_new(const char *filepath, const char *separator);

void fi_dispose(FileIndex *fi);

FileIndexEntry *fi_find(FileIndex *fi, int n);

int fi_compactify(FileIndex *fi);


int main(void) {
	return 0;
}
