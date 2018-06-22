#ifndef LINEBUFFER_H
#define LINEBUFFER_H

#define LINEBUFFERSIZE 256

typedef struct lbuf {
    int descriptor;             /* Eingabe-Descriptor */
    const char *linesep;        /* Zeilentrenner, C-String */
    unsigned lineseplen;        /* Länge des Zeilentrenners in Bytes*/
    char buffer[LINEBUFFERSIZE];/* Lesebuffer */
    unsigned bytesread;         /* Anzahl bereits von descriptor gelesener Bytes */
    unsigned here;              /* aktuelle Verarbeitungsposition im Lesebuffer */
    unsigned end;               /* Anzahl belegter Bytes im Lesebuffer */
} LineBuffer;


LineBuffer *buf_new(int descriptor, const char *linesep);

void buf_dispose(LineBuffer *b);

int buf_readline(LineBuffer *b, char *line, int linemax);

int buf_where(LineBuffer *b);

int buf_seek(LineBuffer *b, int seekpos);

void print_buffer(LineBuffer *lb); /* Zeigt den Buffer */

void print_pos(LineBuffer *b); /* Printet Position */

#endif
