#include <glib.h>
#include "tuples.h"

/**
 * Database Record(Tuple) Set
 */
struct _Tuples
{
  GPtrArray *rows;
  int cols;
  int row_size;
};

Tuples *
tuples_new (int columns)
{
  Tuples *tuples;

  g_return_val_if_fail (columns > 0, NULL);

  tuples = g_new (Tuples, 1);
  tuples->cols = columns;
  tuples->row_size = sizeof (gpointer) * tuples->cols;
  tuples->rows = g_ptr_array_new ();

  return tuples;
}

static void
tuples_free_each (gpointer data, gpointer user_data)
{
  char **tuple = data;
  Tuples *tuples = user_data;
  int i;

  for (i = 0; i < tuples->cols; i++)
    g_free (tuple[i]);
  g_slice_free1 (tuples->row_size, tuple);
}

void
tuples_free (Tuples * tuples)
{
  if (!tuples)
    return;

  if (tuples->rows)
    {
      g_ptr_array_foreach (tuples->rows, tuples_free_each, tuples);
      g_ptr_array_free (tuples->rows, TRUE);
    }

  g_free (tuples);
}

int
tuples_insert (Tuples * tuples, char **values)
{
  char **tuple;
  int i;

  g_return_val_if_fail (tuples != NULL, -1);

  tuple = g_slice_alloc (tuples->row_size);
  if (!tuple)
    {
      g_warning ("failed to allocate tuple (%d)", tuples->row_size);
      return -1;
    }

  for (i = 0; i < tuples->cols; i++)
    tuple[i] = g_strdup (values[i]);

  g_ptr_array_add (tuples->rows, tuple);

  return 0;
}

int
tuples_insertv (Tuples * tuples, ...)
{
  char **values;
  va_list args;
  int i, ret;

  g_return_val_if_fail (tuples != NULL, -1);

  values = g_new (char *, tuples->cols);

  va_start (args, tuples);
  for (i = 0; i < tuples->cols; i++)
    values[i] = va_arg (args, char *);
  va_end (args);

  ret = tuples_insert (tuples, values);

  g_free (values);

  return ret;
}

int
tuples_get_rows (Tuples * tuples)
{
  g_return_val_if_fail (tuples != NULL, -1);

  return tuples->rows->len;
}

int
tuples_get_columns (Tuples * tuples)
{
  g_return_val_if_fail (tuples != NULL, -1);

  return tuples->cols;
}

char **
tuples_get (Tuples * tuples, int row)
{
  char **tuple;

  g_return_val_if_fail (tuples != NULL, NULL);
  g_return_val_if_fail (0 <= row && (guint) row < tuples->rows->len, NULL);

  tuple = g_ptr_array_index (tuples->rows, row);

  return tuple;
}

/*
  Local Variables:
   mode:c
   c-file-style:"gnu"
   indent-tabs-mode:nil
  End:
  vim:autoindent:filetype=c:expandtab:shiftwidth=2:softtabstop=2:tabstop=8
*/
