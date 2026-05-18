bin_PROGRAMS = mz77
mz77_SOURCES = \
	       main.c antidebugger.c hashtable.c messages.c \
	       lz77.c bitio.c queue.c
mz77_CFLAGS = $(GTK4_CFLAGS) $(PROFILE_CFLAGS)
mz77_LDFLAGS = $(PROFILE_LDFLAGS)
mz77_LDADD = $(GTK4_LIBS) -lpthread

