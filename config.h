#ifndef __MEMO_CONFIG_H__
#define __MEMO_CONFIG_H__

gchar *config_read (const gchar *element, gchar *name);
gchar **config_read_names (gchar *element, gint *count);
void config_write (const gchar *element, gchar *data);
void config_init (void);
void config_cleanup (void);

#endif
