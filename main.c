#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include "memo-db.h"

#define MEMO_LIST_COUNT        24 * 2

enum
  {
    MEMO_TREE_VIEW_TIME = 0,
    MEMO_TREE_VIEW_MEMO,
    MEMO_TREE_VIEW_MAX,
  };

enum
  {
    MEMO_TREE_MODEL_TIME = 0,
    MEMO_TREE_MODEL_TIME_INFO_DATE,
    MEMO_TREE_MODEL_TIME_INFO_TIME,
    MEMO_TREE_MODEL_MEMO,
    MEMO_TREE_MODEL_FLAG,
    MEMO_TREE_MODEL_MAX,
  };

gchar *memo_column_titles[MEMO_TREE_VIEW_MAX] = {
    "Time",
    "Memo",
  };

enum
  {
    MEMO_SEARCH_TREE_VIEW_DATE = 0,
    MEMO_SEARCH_TREE_VIEW_TIME,
    MEMO_SEARCH_TREE_VIEW_MEMO,
    MEMO_SEARCH_TREE_VIEW_MAX,
  };

enum
  {
    MEMO_SEARCH_TREE_MODEL_DATE = 0,
    MEMO_SEARCH_TREE_MODEL_TIME,
    MEMO_SEARCH_TREE_MODEL_MEMO,
    MEMO_SEARCH_TREE_MODEL_MAX,
  };

static GtkWidget *memo_calendar;
static GtkTextBuffer *memo_edit_buffer;
static GtkTextBuffer *memo_main_edit_buffer;
static GtkWidget *memo_tree_view;
static GtkWidget *memo_button;
static GtkWidget *memo_swindow;
static GtkWidget *memo_search_entry;

static gchar memo_day[9], memo_time[8];
static GTimeVal memo_timeval;

static void
memo_get_date_info (gchar *_date, gint *year, gint *month, gint *day,
                    gint *hour, gint *min, gint *sec)
{
  gint yy, mm, dd, h, m, s;

  sscanf (_date, "%04d%02d%02d-%02d%02d%02d", &yy, &mm, &dd, &h, &m, &s);
  if (year) *year = yy;
  if (month) *month = mm;
  if (day) *day = dd;
  if (hour) *hour = h;
  if (min) *min = m;
  if (sec) *sec = s;
}

static void
memo_get_date_str_aproximation (gchar *_date, gchar *_time)
{
  gchar *str;
  gint hour, min;
  
  str = g_strdup_printf ("%s-%s", memo_day, memo_time);
  memo_get_date_info (str, NULL, NULL, NULL, &hour, &min, NULL);
  g_free (str);
  
  if (min > 30)
    min = 30;
  if (min < 30)
    min = 0;
  
  snprintf (_date, 9,"%s", memo_day);
  snprintf (_time, 7, "%02d%02d00", hour, min);
}

static void
memo_set (gchar *date, gchar *_time, gchar *data, gint flag)
{
  GtkTreeModel *model;
  GtkTreeIter iter;
  gchar *list_date, *list_time;

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (memo_tree_view));
  gtk_tree_model_get_iter_first (model, &iter);
  do
    {
      gtk_tree_model_get (model, &iter,
                          MEMO_TREE_MODEL_TIME_INFO_DATE, &list_date,
                          MEMO_TREE_MODEL_TIME_INFO_TIME, &list_time,
                          -1);
      
      if (!g_strcmp0 (list_date, date) && !g_strcmp0 (list_time, _time))
        gtk_tree_store_set (GTK_TREE_STORE (model), &iter,
                            MEMO_TREE_MODEL_MEMO, data,
                            MEMO_TREE_MODEL_FLAG, flag,
                            -1);
      g_free (list_date);
      g_free (list_time);

    }
  while (gtk_tree_model_iter_next (model, &iter));

}

static void
memo_set_main (gchar *data)
{
  gchar *buf;
  
  buf = g_locale_to_utf8 (data, -1, NULL, NULL, NULL);

  gtk_text_buffer_set_text (memo_main_edit_buffer, buf, -1);
  g_free (buf);
}

static void
memo_save_main (void)
{
  GtkTextIter start, end;
  gchar *str;
  
  gtk_text_buffer_get_start_iter (GTK_TEXT_BUFFER (memo_main_edit_buffer),
                                  &start);
  gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (memo_main_edit_buffer),
                                  &end);
  
  str = gtk_text_buffer_get_text (memo_main_edit_buffer, &start, &end, TRUE);
  if (!str)
    return;

  if (!g_strcmp0 (str, ""))
    return;
  
  memo_db_update ("00000000", "000000", str);
  g_free (str);
}

static gboolean
memo_save_button_clicked (GtkWidget *button, gpointer data)
{
  gchar *str, _date[9], _time[8];
  GtkTextIter start, end;
  gint flag;
  GtkTreeModel *model;
  GtkTreeSelection *selection;
  GtkTreeIter iter;
      
  gtk_text_buffer_get_start_iter (GTK_TEXT_BUFFER (memo_edit_buffer),
                                  &start);
  gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (memo_edit_buffer),
                                  &end);
  
  str = gtk_text_buffer_get_text (memo_edit_buffer, &start, &end, TRUE);
  if (!str)
    return FALSE;
  
  if (!g_strcmp0 (str, ""))
    return FALSE;

  flag = 0;
  memo_get_date_str_aproximation (_date, _time);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (memo_tree_view));
  gtk_tree_selection_get_selected (selection, &model, &iter);
  do
    {
      gchar *list_date, *list_time, *memo;
  
      gtk_tree_model_get (model, &iter,
                          MEMO_TREE_MODEL_TIME_INFO_DATE, &list_date,
                          MEMO_TREE_MODEL_TIME_INFO_TIME, &list_time,
                          MEMO_TREE_MODEL_MEMO, &memo,
                          -1);
      if (!g_strcmp0 (list_date, _date) && !g_strcmp0 (list_time, _time))
        {
          if (!memo)
            memo_db_insert (_date, _time, str, flag);
          else
            memo_db_update (_date, _time, str);

          memo_set (_date, _time, str, flag);
        }

      g_free (memo);
      g_free (list_date);
      g_free (list_time);

    }
  while (gtk_tree_model_iter_next (model, &iter));
  
  g_free (str);
  gtk_text_buffer_set_text (memo_edit_buffer, "", 0);
  
  return FALSE;
}

static gboolean
memo_edit_button_clicked (GtkWidget *button, gpointer data)
{
  GtkTreeModel *model;
  GtkTreeSelection *selection;
  GtkTreeIter iter;
  gchar *memo, *time_info, *date_info;
  gint flag;
  
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (memo_tree_view));
  gtk_tree_selection_get_selected (selection, &model, &iter);

  gtk_tree_model_get (model, &iter,
                      MEMO_TREE_MODEL_TIME_INFO_DATE, &date_info,
                      MEMO_TREE_MODEL_TIME_INFO_TIME, &time_info,
                      MEMO_TREE_MODEL_MEMO, &memo,
                      MEMO_TREE_MODEL_FLAG, &flag,
                      -1);
  if (memo && g_strcmp0 (memo, ""))
    {
      GtkWidget *parent, *window;
      GtkWidget *vbox, *textview;
      GtkTextBuffer *text_buffer;
      gchar *buf;
      gint response;
      
      parent = gtk_widget_get_toplevel (GTK_WIDGET (button));
      window = gtk_dialog_new_with_buttons ("Edit Memo",
                                            GTK_WINDOW (parent),
                                            GTK_DIALOG_MODAL,
                                            GTK_STOCK_OK,
                                            GTK_RESPONSE_OK,
                                            GTK_STOCK_CANCEL,
                                            GTK_RESPONSE_CANCEL,
                                            NULL);
      gtk_window_set_modal (GTK_WINDOW (window), TRUE);
      gtk_window_set_transient_for (GTK_WINDOW (window), GTK_WINDOW (parent));
      gtk_window_set_default_size (GTK_WINDOW (window), 300, 550);
      
      vbox = GTK_DIALOG (window)->vbox;
      textview = gtk_text_view_new ();
      gtk_widget_show (textview);
      gtk_box_pack_start (GTK_BOX (vbox), textview, TRUE, TRUE, 0);

      text_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));
      buf = g_locale_to_utf8 (memo, -1, NULL, NULL, NULL);

      gtk_text_buffer_set_text (text_buffer, buf, -1);
      g_free (buf);

      response = gtk_dialog_run (GTK_DIALOG (window));
      if (response == GTK_RESPONSE_OK)
        {
          gchar *str;
          GtkTextIter start, end;
  
          gtk_text_buffer_get_start_iter (GTK_TEXT_BUFFER (text_buffer),
                                          &start);
          gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (text_buffer),
                                        &end);
  
          str = gtk_text_buffer_get_text (text_buffer, &start, &end, TRUE);
          if (!str)
            {
              g_free (memo);
              g_free (date_info);
              g_free (time_info);
              return FALSE;
            }
          
          memo_db_update (date_info, time_info, str);
          memo_set (date_info, time_info, str, flag);  
  
          g_free (str);
        }
      
      gtk_widget_destroy (window);
      g_free (memo);
    }

  g_free (date_info);
  g_free (time_info);

  return FALSE;
}
static GtkWidget *
memo_create_tree_view (GtkWidget *parent)
{
  GtkTreeStore *store;
  GtkWidget *view;
  GtkTreeViewColumn *column;
  GtkCellRenderer *renderer;
  GtkTreeIter iter;
  gint i;

  store = gtk_tree_store_new (MEMO_TREE_MODEL_MAX,
                              G_TYPE_STRING,
                              G_TYPE_STRING,
                              G_TYPE_STRING,
                              G_TYPE_STRING,
                              G_TYPE_INT);

  view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));
  gtk_widget_show (view);
  g_object_unref (G_OBJECT (store));
  
  gtk_tree_view_columns_autosize (GTK_TREE_VIEW (view));
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (view), TRUE);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (view), TRUE);

  column = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_title (column,
                                  memo_column_titles[MEMO_TREE_VIEW_TIME]);
  gtk_tree_view_column_set_resizable (column, FALSE);
  gtk_tree_view_column_set_clickable (column, FALSE);
  gtk_tree_view_append_column (GTK_TREE_VIEW (view), column);

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (column, renderer, FALSE);
  gtk_tree_view_column_add_attribute (column, renderer, "markup",
                                      MEMO_TREE_MODEL_TIME);
  
  column = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_title (column,
                                  memo_column_titles[MEMO_TREE_VIEW_MEMO]);
  gtk_tree_view_column_set_resizable (column, FALSE);
  gtk_tree_view_column_set_clickable (column, FALSE);
  gtk_tree_view_append_column (GTK_TREE_VIEW (view), column);

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (column, renderer, FALSE);
  gtk_tree_view_column_add_attribute (column, renderer, "markup",
                                      MEMO_TREE_MODEL_MEMO);
  
  gtk_container_add (GTK_CONTAINER (parent), view);
  
  for (i = 0; i < MEMO_LIST_COUNT; i++)
    {
      gchar date_str[18], time_info[8], date_info[9];

      snprintf (date_info, sizeof (date_info), "%s", memo_day); 
      if (i % 2)
        { 
          g_snprintf (time_info, sizeof (time_info), "%02d3000", i / 2);
          g_snprintf (date_str, sizeof (date_str), "             %02d", 30);
        }
      else
        {
          g_snprintf (time_info, sizeof (time_info),"%02d0000", i / 2);
          if (i == 0)
            g_snprintf (date_str, sizeof (date_str), "AM %02d : 00", i / 2);
          else if (i == 24)
            g_snprintf (date_str, sizeof (date_str), "FM %02d : 00", 24 / 2);
          else if (i > 24)
            g_snprintf (date_str, sizeof (date_str), "      %02d : 00", (i - 24) / 2);
          else
            g_snprintf (date_str, sizeof (date_str), "      %02d : 00", i / 2);
          
        }
      
      gtk_tree_store_append (store, &iter, NULL);
      gtk_tree_store_set (store, &iter,
                          MEMO_TREE_MODEL_TIME, date_str,
                          MEMO_TREE_MODEL_TIME_INFO_DATE, date_info,
                          MEMO_TREE_MODEL_TIME_INFO_TIME, time_info,
                          -1);
    }
  
  return view;
}

static void
memo_remove_tree_view (GtkWidget *view)
{
  gtk_widget_destroy (view);
}

static void
memo_load_treeview (void)
{
  Tuples *tuples;
  gchar **datas;
  gint count, i;
  
  tuples = memo_db_search (NULL);
  if (!tuples)
    return;

  count = tuples_get_rows (tuples);
  for (i = 0; i < count; i++)
    {
      datas = tuples_get (tuples, i);

      if (!g_strcmp0 (datas[0], "00000000"))
        memo_set_main (datas[2]);
      
      if (!g_strcmp0 (datas[0], memo_day))
        memo_set (datas[0], datas[1], datas[2], atoi (datas[3]));
    }
  tuples_free (tuples);
}

static void
memo_load_calendar_marker (void)
{
  Tuples *tuples;
  gchar **datas;
  gint count, i;
  gint year, month, day;
  guint cyear, cmonth, cday;
  
  gtk_calendar_get_date (GTK_CALENDAR (memo_calendar), &cyear, &cmonth, &cday);
  cmonth += 1;
  
  gtk_calendar_clear_marks (GTK_CALENDAR (memo_calendar));

  tuples = memo_db_search_date ();
  if (!tuples)
    return;

  count = tuples_get_rows (tuples);
  for (i = 0; i < count; i++)
    {
      datas = tuples_get (tuples, i);

      sscanf (datas[0], "%04d%02d%02d", &year, &month, &day);
      
      if (year == cyear && month == cmonth)
        gtk_calendar_mark_day (GTK_CALENDAR (memo_calendar), day);
    }
  tuples_free (tuples);
}

static void
memo_set_cursor (void)
{
  GtkTreeModel *model;
  GtkTreeSelection *selection;
  GtkTreePath *path;
  GtkTreeIter iter;
  gchar _date[9], _time[8];


  model = gtk_tree_view_get_model (GTK_TREE_VIEW (memo_tree_view));
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (memo_tree_view));
  
  memo_get_date_str_aproximation (_date, _time);
  
  gtk_tree_model_get_iter_first (model, &iter);
  do
    {
      gchar *list_date, *list_time;
      
      gtk_tree_model_get (model, &iter,
                          MEMO_TREE_MODEL_TIME_INFO_DATE, &list_date,
                          MEMO_TREE_MODEL_TIME_INFO_TIME, &list_time,
                          -1);
      if (!g_strcmp0 (list_date, _date) && !g_strcmp0 (list_time, _time))
        {
          gtk_tree_selection_select_iter (selection, &iter);
          path = gtk_tree_model_get_path (model, &iter);
          gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (memo_tree_view),
                                        path,
                                        NULL,
                                        FALSE,
                                        0,
                                        0);
          gtk_tree_path_free (path);
        }
      
      g_free (list_date);
      g_free (list_time);
    }
  while (gtk_tree_model_iter_next (model, &iter));
}

static gboolean
memo_check_time (gpointer data)
{
  GTimeVal current_time;
  struct tm tm;
  time_t _time;
  gint year, month, day, hour, min, sec, old_hour;
  gint init;

  init = (gint )data;
  
  g_get_current_time (&current_time);
  _time = (time_t ) current_time.tv_sec;
  localtime_r (&_time, &tm);

  year = tm.tm_year + 1900;
  month = tm.tm_mon + 1;
  day = tm.tm_mday;

  hour = tm.tm_hour;
  min = tm.tm_min;
  sec = tm.tm_sec;

  memo_save_main ();
  
  if ((min == 30 || min == 0) && !init)
    g_signal_emit_by_name (memo_button, "clicked", NULL);
  
  g_snprintf (memo_day, sizeof (memo_day), "%04d%02d%02d", year, month, day);
  g_snprintf (memo_time, sizeof (memo_time), "%02d%02d%02d", hour, min, sec);
  
  _time = (time_t ) memo_timeval.tv_sec;
  localtime_r (&_time, &tm);
  old_hour = tm.tm_hour;
  
  memo_timeval = current_time;
  
  if (0)
    g_warning ("%04d%02d%02d-%02d%02d%02d", year, month, day, hour, min, sec);

  if (min == 30 || min == 0 || init)
    memo_set_cursor ();

  return TRUE;
}

static void
memo_init_treeview (void)
{
  GtkTreeModel *model;
  GtkTreeSelection *selection;
  GtkTreeIter iter;
  gchar _date[9], _time[8];
  
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (memo_tree_view));
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (memo_tree_view));

  memo_get_date_str_aproximation (_date, _time);
  
  gtk_tree_model_get_iter_first (model, &iter);
  do
    {
      gchar *list_date, *list_time;
      
      gtk_tree_model_get (model, &iter,
                          MEMO_TREE_MODEL_TIME_INFO_DATE, &list_date,
                          MEMO_TREE_MODEL_TIME_INFO_TIME, &list_time,
                          -1);
      
      if (!g_strcmp0 (list_date, _date) && !g_strcmp0 (list_time, _time))
        gtk_tree_selection_select_iter (selection, &iter);
      
      g_free (list_date);
      g_free (list_time);
  }
  while (gtk_tree_model_iter_next (model, &iter));
}

static void
memo_calendar_selected (GtkCalendar *calendar, gpointer data)
{
  guint year, month, day;
  
  gtk_calendar_get_date (calendar, &year, &month, &day);
  g_snprintf (memo_day, sizeof (memo_day), "%04d%02d%02d", year, month + 1, day);
  
  memo_remove_tree_view (memo_tree_view);
  memo_tree_view = memo_create_tree_view (memo_swindow);
  memo_load_treeview ();
  memo_init_treeview ();
}

static void
memo_calendar_changed (GtkCalendar *calendar, gpointer data)
{
  memo_load_calendar_marker ();
}

static void
memo_search_button_clicked (GtkButton *button, gpointer data)
{
  GtkWidget *parent, *window;
  GtkWidget *vbox;
  gint response;
  const gchar *str;
  
  str = gtk_entry_get_text (GTK_ENTRY (memo_search_entry));
  if (!g_strcmp0 (str, ""))
    return;
  
  parent = gtk_widget_get_toplevel (GTK_WIDGET (button));
  window = gtk_dialog_new_with_buttons ("Memo Search Result",
                                        GTK_WINDOW (parent),
                                        GTK_DIALOG_MODAL,
                                        GTK_STOCK_OK,
                                        GTK_RESPONSE_OK,
                                        NULL);
  gtk_window_set_modal (GTK_WINDOW (window), TRUE);
  gtk_window_set_transient_for (GTK_WINDOW (window), GTK_WINDOW (parent));
  gtk_window_set_default_size (GTK_WINDOW (window), 500, 550);

  vbox = GTK_DIALOG (window)->vbox;

  {
    GtkTreeStore *store;
    GtkWidget *view, *swindow;
    GtkTreeViewColumn *column;
    GtkCellRenderer *renderer;
    Tuples *tuples;
    gint count, i;

    swindow = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_show (swindow);
    gtk_box_pack_start (GTK_BOX (vbox), swindow, TRUE, TRUE, 0);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swindow),
                                    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    
    
    store = gtk_tree_store_new (MEMO_SEARCH_TREE_MODEL_MAX,
                                G_TYPE_STRING,
                                G_TYPE_STRING,
                                G_TYPE_STRING);
    
    view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));
    gtk_widget_show (view);
    g_object_unref (G_OBJECT (store));

    gtk_tree_view_columns_autosize (GTK_TREE_VIEW (view));
    gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (view), TRUE);
    gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (view), TRUE);

    column = gtk_tree_view_column_new ();
    gtk_tree_view_column_set_title (column, "Date");
    gtk_tree_view_column_set_resizable (column, FALSE);
    gtk_tree_view_column_set_clickable (column, FALSE);
    gtk_tree_view_append_column (GTK_TREE_VIEW (view), column);

    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start (column, renderer, FALSE);
    gtk_tree_view_column_add_attribute (column, renderer, "markup",
                                        MEMO_SEARCH_TREE_MODEL_DATE);
  
    column = gtk_tree_view_column_new ();
    gtk_tree_view_column_set_title (column, "Time");
    gtk_tree_view_column_set_resizable (column, FALSE);
    gtk_tree_view_column_set_clickable (column, FALSE);
    gtk_tree_view_append_column (GTK_TREE_VIEW (view), column);

    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start (column, renderer, FALSE);
    gtk_tree_view_column_add_attribute (column, renderer, "markup",
                                      MEMO_SEARCH_TREE_MODEL_TIME);

    column = gtk_tree_view_column_new ();
    gtk_tree_view_column_set_title (column, "Memo");
    gtk_tree_view_column_set_resizable (column, FALSE);
    gtk_tree_view_column_set_clickable (column, FALSE);
    gtk_tree_view_append_column (GTK_TREE_VIEW (view), column);

    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start (column, renderer, FALSE);
    gtk_tree_view_column_add_attribute (column, renderer, "markup",
                                      MEMO_SEARCH_TREE_MODEL_MEMO);
    
    gtk_container_add (GTK_CONTAINER (swindow), view);

    tuples = memo_db_search_find (str);
    if (tuples)
      {
        count = tuples_get_rows (tuples);
        for (i = 0; i < count; i++)
          {
            gchar **datas;
            GtkTreeIter iter;
        
            datas = tuples_get (tuples, i);
            gtk_tree_store_append (store, &iter, NULL);
            gtk_tree_store_set (store, &iter,
                                MEMO_SEARCH_TREE_MODEL_DATE, datas[0],
                                MEMO_SEARCH_TREE_MODEL_TIME, datas[1],
                                MEMO_SEARCH_TREE_MODEL_MEMO, datas[2],
                                -1);
          }
        tuples_free (tuples);
      }
  }

  response = gtk_dialog_run (GTK_DIALOG (window));
  gtk_widget_destroy (window);
  gtk_entry_set_text (GTK_ENTRY (memo_search_entry), "");
  
}


static void
memo_create_window (void)
{
  GtkWidget *window, *frame;
  GtkWidget *main_hbox, *left_vbox, *right_vbox, *button_box;
  GtkWidget *list_vbox, *search_hbox;
  GtkWidget *calendar, *text_view, *main_text_view;
  GtkTextBuffer *text_buffer, *main_text_buffer;
  GtkWidget *swindow, *label;
  GtkWidget *button, *entry;
  
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_name (window, "Memo");
  gtk_window_set_title (GTK_WINDOW (window), "Memo");
  gtk_widget_show (window);

  main_hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (main_hbox);
  gtk_container_add (GTK_CONTAINER (window), main_hbox);

  left_vbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (left_vbox);
  gtk_box_pack_start (GTK_BOX (main_hbox), left_vbox, FALSE, FALSE, 0);
  
  calendar = gtk_calendar_new ();
  gtk_widget_show (calendar);
  memo_calendar = calendar;
  
  gtk_box_pack_start (GTK_BOX (left_vbox), calendar, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (calendar), "day-selected",
                    G_CALLBACK (memo_calendar_selected), NULL);

  g_signal_connect (G_OBJECT (calendar), "next-month",
                    G_CALLBACK (memo_calendar_changed), NULL);
  g_signal_connect (G_OBJECT (calendar), "next-year",
                    G_CALLBACK (memo_calendar_changed), NULL);
  g_signal_connect (G_OBJECT (calendar), "prev-month",
                    G_CALLBACK (memo_calendar_changed), NULL);
  g_signal_connect (G_OBJECT (calendar), "prev-year",
                    G_CALLBACK (memo_calendar_changed), NULL);
  
  {
    GTimeVal current_time;
    struct tm tm;
    time_t _time;
    guint year, month, day;
    gint hour, min, sec;
    
    gtk_calendar_get_date (GTK_CALENDAR (calendar), &year, &month, &day);
    
    g_get_current_time (&current_time);

    _time = (time_t ) current_time.tv_sec;
    localtime_r (&_time, &tm);

    hour = tm.tm_hour;
    min = tm.tm_min;
    sec = tm.tm_sec;

    month += 1;
    
    g_snprintf (memo_day, sizeof (memo_day), "%04d%02d%02d", year, month, day);
    g_snprintf (memo_time, sizeof (memo_time), "%02d%02d%02d", hour, min, sec);

    if (0)
      g_warning ("%04d%02d%02d-%02d%02d%02d", year, month, day, hour, min, sec);

  }

  label = gtk_label_new ("Sticky Memo");
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (left_vbox), label, FALSE, FALSE, 0);
  
  {
    GtkTextTagTable *table;

    table = gtk_text_tag_table_new ();
    main_text_buffer = gtk_text_buffer_new (table);
    main_text_view = gtk_text_view_new_with_buffer (main_text_buffer);
  }
  
  memo_main_edit_buffer = main_text_buffer;
  gtk_widget_show (main_text_view);
  gtk_box_pack_start (GTK_BOX (left_vbox), main_text_view, TRUE, TRUE, 0);

  label = gtk_label_new ("Daily Memo");
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (left_vbox), label, FALSE, FALSE, 0);
  
  text_view = gtk_text_view_new ();
  text_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));
  memo_edit_buffer = text_buffer;
  
  gtk_widget_show (text_view);
  gtk_widget_set_size_request (text_view, 300, 300);
  gtk_box_pack_start (GTK_BOX (left_vbox), text_view, FALSE, FALSE, 0);

  button_box = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (button_box);
  gtk_box_pack_start (GTK_BOX (left_vbox), button_box, FALSE, FALSE, 0);
  
  button = gtk_button_new_with_label ("Save");
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (button_box), button, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (memo_save_button_clicked), NULL);
  memo_button = button;

  button = gtk_button_new_with_label ("Edit");
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (button_box), button, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (memo_edit_button_clicked), NULL);
  
  right_vbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (right_vbox);
  gtk_box_pack_start (GTK_BOX (main_hbox), right_vbox, TRUE, TRUE, 0);
  
  frame = gtk_frame_new ("Search");
  gtk_widget_show (frame);
  gtk_box_pack_start (GTK_BOX (right_vbox), frame, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);
  
  search_hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (search_hbox);
  gtk_container_add (GTK_CONTAINER (frame), search_hbox);
  
  label = gtk_label_new (" Keyword : ");
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (search_hbox), label, FALSE, FALSE, 0);

  entry = gtk_entry_new ();
  gtk_widget_show (entry);
  memo_search_entry = entry;
  
  gtk_box_pack_start (GTK_BOX (search_hbox), entry, TRUE, TRUE, 0);
  
  button = gtk_button_new_with_label ("   Show   ");
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (search_hbox), button, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (memo_search_button_clicked), NULL);
 
  frame = gtk_frame_new ("Memos");
  gtk_widget_show (frame);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
  
  gtk_box_pack_start (GTK_BOX (right_vbox), frame, TRUE, TRUE, 0);
  
  list_vbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (list_vbox);
  gtk_container_add (GTK_CONTAINER (frame), list_vbox);
  gtk_container_set_border_width (GTK_CONTAINER (list_vbox), 3);
  
  swindow = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (swindow);
  memo_swindow = swindow;

  gtk_widget_set_size_request (swindow, 420, -1);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swindow),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX (list_vbox), swindow, TRUE, TRUE, 0);

  gtk_widget_set_size_request (window, -1, 900);
  
  memo_tree_view = memo_create_tree_view (swindow);
  memo_load_calendar_marker ();
  memo_load_treeview ();
  memo_init_treeview ();

  if (!g_file_test (".started", G_FILE_TEST_EXISTS))
    {
      GError *err;
      
      memo_db_insert ("00000000", "000000", "", 0);
      err = NULL;
      g_file_set_contents (".started", "memo", 4, &err);
    }
  memo_check_time ((gpointer)1);
  g_timeout_add (60000, memo_check_time, NULL);
}


int 
main (void)
{
  memo_db_init ();

  gtk_init(NULL, NULL);
  memo_create_window ();
  gtk_main ();
  memo_db_cleanup ();

  return 0;
}

/*
  Local Variables:
   mode:c
   c-file-style:"gnu"
   indent-tabs-mode:nil
  End:
  vim:autoindent:filetype=c:expandtab:shiftwidth=2:softtabstop=2:tabstop=8
*/
