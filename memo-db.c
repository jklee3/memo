#include "memo-db.h"
#include "tuples.h"
#include <sqlite3.h>
#include <glib.h>

static sqlite3 *memo_db;

static gint
memo_db_exec_cb (void *data,
                 int cols,
                 char **values,
                 char **names)
{
  Tuples **tuples = data;

  if (*tuples == NULL)
    {
      *tuples = tuples_new (cols);
      if (!*tuples)
        return 1;               /* abort */
    }
  
  if (tuples_insert (*tuples, values))
    return 1;                   /* abort */
  
  return 0;
}

static void
memo_db_create_table (void)
{
  char *err_msg;
  gint ret;

  err_msg = NULL;
  ret = sqlite3_exec (memo_db,
                      "CREATE TABLE Memo (stampDate TEXT (8),"
                      "stampTime TEXT(8),"
		      "msg VARCHAR(10000), flag INTEGER(1))",
		      memo_db_exec_cb,
		      NULL,
		      &err_msg);
  if (ret != SQLITE_OK)
    {
      g_warning ("sql error ; %s", err_msg);
      return;
    }
}

static gboolean
memo_db_has_table (void)
{
  char *err;
  gint ret;
  Tuples *tuples;

  tuples = NULL;
  err = NULL;
  ret = sqlite3_exec (memo_db,
                      "PRAGMA table_info(Memo)",
                      memo_db_exec_cb,
                      &tuples,
                      &err);

  if (ret != SQLITE_OK)
    return FALSE;

  if (tuples)
    return TRUE;

  return FALSE;
}

void
memo_db_update (gchar *date, gchar *_time, gchar *msg)
{
  gchar sql[1024];
  gchar *err_msg;
  gint ret;
  
  g_snprintf (sql,
              sizeof (sql),
              "UPDATE Memo SET msg = '%s' WHERE stampDate = '%s' AND stampTime = '%s'",
              msg,
              date,
              _time);
  
  ret = sqlite3_exec (memo_db, sql, memo_db_exec_cb, NULL, &err_msg);
  if (ret != SQLITE_OK)
    {
      g_warning ("sql error ; %s", err_msg);
      return;
    }
}

void
memo_db_insert (gchar *date, gchar *_time, gchar *msg, gint flag)
{
  gchar sql[1024];
  gchar *err_msg;
  gint ret;
  
  g_snprintf (sql,
              sizeof (sql),
              "INSERT INTO Memo (stampDate, stampTime,msg, flag) VALUES ('%s', '%s', '%s', '%d')",
              date,
              _time,
              msg,
              flag);
  
  ret = sqlite3_exec (memo_db, sql, memo_db_exec_cb, NULL, &err_msg);
  if (ret != SQLITE_OK)
    {
      g_warning ("sql error ; %s", err_msg);
      return;
    }
}

Tuples *
memo_db_search (gchar *where)
{
  char *err;
  gint ret;
  Tuples *tuples;
  gchar sql[1024];
  
  tuples = NULL;
  err = NULL;
  
  if (where)
    g_snprintf (sql,
                sizeof (sql),
                "SELECT * FROM Memo WHERE %s",
                where);
  else
    g_strlcpy(sql, "SELECT * FROM Memo", sizeof (sql));
  
  ret = sqlite3_exec (memo_db,
                      sql,
                      memo_db_exec_cb,
                      &tuples,
                      &err);

  if (ret != SQLITE_OK)
    {
      g_warning ("memo_db_search error : %s", err);
      g_free (err);
      return NULL;
    }
  
  return tuples;
}

Tuples *
memo_db_search_date (void)
{
  char *err;
  gint ret;
  Tuples *tuples;
  gchar sql[1024];
  
  tuples = NULL;
  err = NULL;

  g_strlcpy(sql, "SELECT DISTINCT stampDate FROM Memo",
            sizeof (sql));
  
  ret = sqlite3_exec (memo_db,
                      sql,
                      memo_db_exec_cb,
                      &tuples,
                      &err);

  if (ret != SQLITE_OK)
    {
      g_warning ("memo_db_search error : %s", err);
      g_free (err);
      return NULL;
    }
  
  return tuples;
}

Tuples *
memo_db_search_find (const gchar *keyword)
{
  char *err;
  gint ret;
  Tuples *tuples;
  gchar sql[1024];
  
  tuples = NULL;
  err = NULL;

  g_snprintf (sql,
              sizeof (sql),
              "SELECT * FROM Memo WHERE msg LIKE '%%%s%%'",
              keyword);
  
  ret = sqlite3_exec (memo_db,
                      sql,
                      memo_db_exec_cb,
                      &tuples,
                      &err);

  if (ret != SQLITE_OK)
    {
      g_warning ("memo_db_search error : %s", err);
      g_free (err);
      return NULL;
    }
  
  return tuples;
}

void
memo_db_init (void)
{
  if (sqlite3_open ("memo.db", &memo_db))
    {
      g_warning ("memo database not opened");
      return;
    }

#if 0  
  sqlite3_exec (memo_db, "PRAGMA synchronous = OFF", NULL, NULL, NULL);
  sqlite3_exec (memo_db, "PRAGMA auto_vacuum = 1", NULL, NULL, NULL);
#endif
  
  if (!memo_db_has_table ())
    memo_db_create_table ();
}

void
memo_db_cleanup (void)
{
	sqlite3_close (memo_db);
}

/*
  Local Variables:
    mode:c
    c-file-style:"gnu"
    indent-tabs-mode:nil
  End:
  vim:autoindent:filetype=c:expandtab:shiftwidth=2:softtabstop=2:tabstop=8
*/
