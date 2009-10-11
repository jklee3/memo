
GTK_CFLAGS := $(shell pkg-config --cflags gtk+-2.0)
GTK_LDFLAGS := $(shell pkg-config --libs gtk+-2.0)

SQLITE3_CFLAGS += -I/usr/include/
SQLITE3_LDFLAGS += -L/usr/lib/ -l sqlite3

CFLAGS += $(GTK_CFLAGS) $(SQLITE3_CFLAGS)
LDFLAGS += $(GTK_LDFLAGS) $(SQLITE3_LDFLAGS)

CFLAGS += $(GTK_CFLAGS) -DGTK_DISABLE_DEPRECATED -DGDK_PIXBUF_DISABLE_DEPRECATED -DG_DISABLE_DEPRECATED
CFLAGS += -Wall -Wunused

%.o: %.c
	gcc -c -o $@ $(CFLAGS) $<

memo-manager : main.o memo-db.o tuples.o config.o
	gcc -o $@ $(CFLAGS) $(LDFLAGS) $^

clean:
	rm *.o memo-manager

dist-clean:
	rm *.o memo-manager memo.db .started

