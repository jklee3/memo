
#ifndef __MEMO_DB_H_
#define __MEMO_DB_H__

#include <glib.h>
#include "tuples.h"

void memo_db_insert (gchar *date, gchar *_time, gchar *msg, gint flag);
void memo_db_update (gchar *date, gchar *_time, gchar *msg);
Tuples *memo_db_search (gchar *where);
Tuples *memo_db_search_date (void);
Tuples *memo_db_search_find (const gchar *keyword);
void memo_db_init (void);
void memo_db_cleanup (void);

#endif 	
