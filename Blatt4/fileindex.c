#include <stdio.h>

/* Ziel:
 * Betrachtete Textdatei in indexierbare Abschnitte einteilen.
 * 
 * Jeder Abschnitt enthält eine SEPARATORZEILE, die NICHT zum Inhalt gehört.
 * ACHTUNG: Leerzeilen gehören zum Inhalt.
 * 
 * 
 * 
 *  */
 
 
/* Indiziert angegebene Datei filepath. Trennzeilen sind wie SEPARATOR und gehören NICHT zum Inhalt
 * return Zeiger auf FileIndex wenn okay
 * return NULL bei Fehler
 * */
FileIndex *fi_new(const char *filepath, const char *separator);

/* Gibt Speicher für FileIndex-Struct frei
 * ACHTUNG: INKLUSIVE Liste der FileIndex Knoten 
 */
void fi_dispose(FileIndex *fi);

/* return Zeiger auf FileIndexEntry zum Listenelement n
 * ACHTUNG: Zählung beginnt bei "1" <- EINS
 * return NULL bei Fehler
 * */
FileIndexEntry *fi_find(FileIndex *fi, int n);


/* Löscht alle Abschnitte, wo del_flag == 1 ist. -> Umkopieren + Umbenennen
 * return 0 bei Erfolg.
 * return != 0 bei FEHLER
 */
int fi_compactify(FileIndex *fi);


int main(void) {
	return 0;
}
