#include <glib.h>
#include <expat.h>
#include <unistd.h>
#include "config.h"

typedef struct _MemoConfig
{
  gchar element[16];
  gchar name[16];
  gchar key[16];
} MemoConfig;

static gchar *config_file;
static GList *config_list;

static const char *
config_get_xml_value (const char **attr, const char *name)
{
  int i;

  for (i = 0; attr[i]; i += 2)
    if (!g_ascii_strcasecmp (attr[i], name))
      return (const char *)attr[i + 1];

  return NULL;
}

static void
config_xml_start (void *data, const gchar *element, const char **attr)
{
  MemoConfig *conf;
  
  while (!g_ascii_strcasecmp (element, "menu"))
    {
      const char *name = config_get_xml_value (attr, "name");
      const char *key = config_get_xml_value (attr, "key");

      conf = g_new (MemoConfig, 1);
      g_strlcpy (conf->element, "menu", 16);
      g_strlcpy (conf->name, name, 16);
      g_strlcpy (conf->key, key, 16);

      config_list = g_list_append (config_list, conf);
      break;
    }
}

static void
config_xml_end (void *data, const gchar *element)
{


}

static void
config_xml_data (void *data, const gchar *element, int len)
{


}

gchar *
config_read (const gchar *element, gchar *name)
{
  GList *list;

  for (list = config_list; list; list = g_list_next (list))
    {
      MemoConfig *conf = list->data;
      
      if (!g_strcmp0 (conf->element, element)
          && !g_strcmp0 (conf->name, name))
        return conf->key;
    }

  return NULL;
}

gchar **
config_read_names (gchar *element, gint *count)
{
  gint len, i;
  GList *list;
  gchar **names;

  len = 0;
  for (list = config_list; list; list = g_list_next (list))
    {
      MemoConfig *conf = list->data;
      if (!g_strcmp0 (conf->element, element))
        len++;
    }

  names = g_new (gchar *, len);
  i = 0;
  for (list = config_list; list; list = g_list_next (list))
    {
      MemoConfig *conf = list->data;
      if (!g_strcmp0 (conf->element, element))
        names[i++] = g_strdup (conf->name);
    }
  
  *count = len;
  return names;
}

void
config_write (const gchar *element, gchar *data)
{



}

static void
config_save (void)
{



}

void
config_init (void)
{
  gchar *buf;
  guint buf_size;
  XML_Parser p;
  GError *err;
  
  config_file = "./memo.xml";
  config_list = NULL;
  
  err = NULL;
  if (!g_file_get_contents (config_file, &buf, &buf_size, &err))
    {
      g_warning ("failed loading config file");
      return;
    }
  
  p = XML_ParserCreate (NULL);
  if (!p)
    {
      g_warning ("can't create XML parser");
      g_free (buf);
      return;
    }

  XML_SetElementHandler (p, config_xml_start, config_xml_end);
  XML_SetCharacterDataHandler (p, config_xml_data);
  if (!XML_Parse (p, buf, buf_size, 0))
    {
      g_warning ("XML parse error at line %d: %s",
                 (gint)XML_GetCurrentLineNumber (p),
                 XML_ErrorString (XML_GetErrorCode (p)));
      XML_ParserFree (p);
      g_free (buf);
      return;
    }
  
  XML_ParserFree (p);
  g_free (buf);
}

void
config_cleanup (void)
{

  config_save ();

}

/*
  Local Variables:
    mode:c
    c-file-style:"gnu"
    indent-tabs-mode:nil
  End:
  vim:autoindent:filetype=c:expandtab:shiftwidth=2:softtabstop=2:tabstop=8
*/
