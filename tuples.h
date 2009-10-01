#ifndef __TUPLES_H__
#define __TUPLES_H__

typedef struct _Tuples Tuples;

Tuples *tuples_new (int columns);
void tuples_free (Tuples * tuples);
int tuples_insertv (Tuples * tuples, ...);
int tuples_insert (Tuples * tuples, char **values);

int tuples_get_rows (Tuples * tuples);
int tuples_get_columns (Tuples * tuples);
char **tuples_get (Tuples * tuples, int row);

#endif /* __TUPLES_H__ */

/*
  Local Variables:
   mode:c
   c-file-style:"gnu"
   indent-tabs-mode:nil
  End:
  vim:autoindent:filetype=c:expandtab:shiftwidth=2:softtabstop=2:tabstop=8
*/
